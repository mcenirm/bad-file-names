#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Wrapper function to open a file with a UTF-8 filename across platforms
FILE *universal_fopen(const char *utf8_filename, const char *mode) {
#ifdef _WIN32
  // Windows: Convert UTF-8 to UTF-16 (wchar_t) for _wfopen
  int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_filename, -1, NULL, 0);
  wchar_t *wname = (wchar_t *)malloc(wlen * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, utf8_filename, -1, wname, wlen);

  int mlen = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
  wchar_t *wmode = (wchar_t *)malloc(mlen * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, mlen);

  FILE *f = _wfopen(wname, wmode);

  free(wname);
  free(wmode);
  return f;
#else
  // Unix/Linux/macOS: Most modern filesystems treat filenames as UTF-8 byte
  // strings
  return fopen(utf8_filename, mode);
#endif
}

/**
 * Encodes a Unicode code point into a UTF-8 byte sequence.
 * @param out: Pointer to a buffer of at least 5 bytes (4 for data + 1 for null
 * terminator).
 * @param cp: The Unicode code point (e.g., 0x20AC for the Euro sign).
 * @return: The number of bytes written (excluding null terminator), or 0 on
 * error.
 */
size_t utf8_from_codepoint(char *out, uint32_t cp) {
  if (cp <= 0x7F) {
    out[0] = (unsigned char)cp;
    out[1] = '\0';
    return 1;
  } else if (cp <= 0x7FF) {
    out[0] = (unsigned char)((cp >> 6) | 0xC0);
    out[1] = (unsigned char)((cp & 0x3F) | 0x80);
    out[2] = '\0';
    return 2;
  } else if (cp <= 0xFFFF) {
    out[0] = (unsigned char)((cp >> 12) | 0xE0);
    out[1] = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
    out[2] = (unsigned char)((cp & 0x3F) | 0x80);
    out[3] = '\0';
    return 3;
  } else if (cp <= 0x10FFFF) {
    out[0] = (unsigned char)((cp >> 18) | 0xF0);
    out[1] = (unsigned char)(((cp >> 12) & 0x3F) | 0x80);
    out[2] = (unsigned char)(((cp >> 6) & 0x3F) | 0x80);
    out[3] = (unsigned char)((cp & 0x3F) | 0x80);
    out[4] = '\0';
    return 4;
  }
  return 0; // Invalid code point
}

void fprintescapedstring(FILE *f, char *s) {
  for (size_t i = 0; s[i]; i++) {
    if (isprint(s[i])) {
      switch (s[i]) {
      case '\'':
        fprintf(f, "\\'");
        break;
      case '\"':
        fprintf(f, "\\\"");
        break;
      case '\?':
        fprintf(f, "\\?");
        break;
      case '\\':
        fprintf(f, "\\\\");
        break;
      case '\a':
        fprintf(f, "\\a");
        break;
      case '\b':
        fprintf(f, "\\b");
        break;
      case '\f':
        fprintf(f, "\\f");
        break;
      case '\n':
        fprintf(f, "\\n");
        break;
      case '\r':
        fprintf(f, "\\r");
        break;
      case '\t':
        fprintf(f, "\\t");
        break;
      case '\v':
        fprintf(f, "\\v");
        break;
      default:
        fprintf(f, "%c", s[i]);
        break;
      }
    } else {
      fprintf(f, "\\x%02" PRIx8, s[i]);
    }
  }
}

#define CHECK_COUNT 3
#define FILENAME_COUNT (CHECK_COUNT + 4 + CHECK_COUNT)

int main() {
  char check[CHECK_COUNT + 1];
  char filename[FILENAME_COUNT + 1];

  for (size_t i = 0; i < CHECK_COUNT; i++) {
    check[i] = 'a' + (char)i;
  }
  check[CHECK_COUNT] = '\0';

  for (uint32_t u = 0x01; u < 0x02FF; u++) {
    char u_as_utf8[5];

    size_t utf8_count = utf8_from_codepoint(u_as_utf8, u);
    if (utf8_count < 1) {
      fprintf(stderr, "Error creating UTF8 for codepoint: 0x%" PRIx32 "\n", u);
    }

    size_t i = 0;
    for (size_t j = 0; j < CHECK_COUNT && i <= FILENAME_COUNT; j++) {
      filename[i++] = check[j];
    }
    for (size_t j = 0; j < utf8_count && i <= FILENAME_COUNT; j++) {
      filename[i++] = u_as_utf8[j];
    }
    for (size_t j = 0; j < CHECK_COUNT && i <= FILENAME_COUNT; j++) {
      filename[i++] = check[j];
    }
    filename[i] = '\0';

    FILE *f = universal_fopen(filename, "w");

    if (f) {
      fprintf(f, "%s\n", filename);
      fclose(f);
      printf("Created");
    } else {
      printf("Error creating");
    }
    printf(" file [0x%04" PRIx32 "]: \"", u);
    fprintescapedstring(stdout, filename);
    printf("\"\n");
  }
  return 0;
}
