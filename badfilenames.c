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

int main() {
  // Filename with Unicode characters: "Greeting_你好_नमस्ते.txt"
  const char *filename = "Greeting_\xe4\xbd\xa0\xe5\xa5\xbd_"
                         "\xe0\xa4\xa8\xe0\xa4\xae\xe0\xa4\xb8\xe0\xa5\x8d\xe0"
                         "\xa4\xa4\xe0\xa5\x87.txt";

  FILE *f = universal_fopen(filename, "w");

  if (f) {
    fprintf(f, "File created successfully with a Unicode filename!\n");
    fclose(f);
    printf("Created file: %s\n", filename);
  } else {
    perror("Error creating file");
  }

  return 0;
}
