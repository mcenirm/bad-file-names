project(BadFileNames LANGUAGES C)
add_executable(badfilenames badfilenames.c)
enable_testing()
add_test(NAME TestIt COMMAND badfilenames)
