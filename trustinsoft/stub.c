#include <time.h>

time_t time(time_t *tloc) {
  return 99;
}

#include <sys/stat.h>

int chmod(const char *pathname, mode_t mode) {
  return 0;
}

#include <unistd.h>
#include <string.h>

/* Upon successful completion, mkdir() shall return 0. */
int mkdir(const char *path, mode_t mode) {
  /* Creating directories is Not Implemented Yet, so we only check if the
     directory exists already. */
  if (strcmp(path, ".") == 0) return 0;
  return access(path, F_OK);
}

/* This will not be necessary after solving TRUS-2308:
   https://support.trust-in-soft.com/browse/TRUS-2308 */
void tis_make_unknown(char *__p, unsigned long __l) {
  return;
}
