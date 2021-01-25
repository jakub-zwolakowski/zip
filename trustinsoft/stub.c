#include <time.h>

time_t time(time_t *tloc) {
  return 99;
}

#include <sys/stat.h>

int chmod(const char *pathname, mode_t mode) {
  return 0;
}
int mkdir(const char *path, mode_t mode) {
  /* Upon successful completion, mkdir() shall return 0. */
  return 0;
}

// #include <stdio.h>
//
// FILE *freopen(const char *path, const char *mode, FILE *stream) {
//   if (stream != NULL)
//     fclose(stream);
//   stream = fopen(path, mode);
//   return stream;
// }

void tis_make_unknown(char *__p, unsigned long __l) {
  return;
}
