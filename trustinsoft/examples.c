#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

#include "zip.h"

/* Create a new zip archive with default compression level. */

/* REQUIRES: foo-2.1.txt, foo-2.2.txt, foo-2.3.txt */
void create (void) {
    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        zip_entry_open(zip, "foo-1.txt");
        {
            const char *buf = "Some data here...\0";
            zip_entry_write(zip, buf, strlen(buf));
        }
        zip_entry_close(zip);

        zip_entry_open(zip, "foo-2.txt");
        {
            // merge 3 files into one entry and compress them on-the-fly.
            zip_entry_fwrite(zip, "foo-2.1.txt");
            zip_entry_fwrite(zip, "foo-2.2.txt");
            zip_entry_fwrite(zip, "foo-2.3.txt");
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
}


/* Append to the existing zip archive. */

/* REQUIRES: foo.zip */
void append (void) {
    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
    {
        zip_entry_open(zip, "foo-3.txt");
        {
            const char *buf = "Append some data here...\0";
            zip_entry_write(zip, buf, strlen(buf));
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
}


/* Extract a zip archive into a folder. */

int on_extract_entry(const char *filename, void *arg) {
    static int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}

/* REQUIRES: foo.zip */
void extract_into_a_folder (void) {
    int arg = 2;
    zip_extract("foo.zip", "/tmp", on_extract_entry, &arg);
}


/* Extract a zip entry into memory. */

/* REQUIRES: foo.zip */
void extract_into_memory (void) {
    void *buf = NULL;
    size_t bufsize;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        zip_entry_open(zip, "foo-1.txt");
        {
            zip_entry_read(zip, &buf, &bufsize);
        }
        zip_entry_close(zip);
    }
    zip_close(zip);

    free(buf);
}


/* Extract a zip entry into memory (no internal allocation). */

/* REQUIRES: foo.zip */
void extract_into_memory_no_allocation (void) {
    unsigned char *buf;
    size_t bufsize;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        zip_entry_open(zip, "foo-1.txt");
        {
            bufsize = zip_entry_size(zip);
            buf = calloc(sizeof(unsigned char), bufsize);

            zip_entry_noallocread(zip, (void *)buf, bufsize);
        }
        zip_entry_close(zip);
    }
    zip_close(zip);

    free(buf);
}


/* Extract a zip entry into a file. */

/* REQUIRES: foo.zip */
void extract_entry_into_a_file (void) {
    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        zip_entry_open(zip, "foo-2.txt");
        {
            zip_entry_fread(zip, "foo-2.txt");
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
}


/* List of all zip entries. */

/* REQUIRES: foo.zip */
void list_all_entries (void) {
    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    int i, n = zip_total_entries(zip);
    for (i = 0; i < n; ++i) {
        zip_entry_openbyindex(zip, i);
        {
            const char *name = zip_entry_name(zip);
            int isdir = zip_entry_isdir(zip);
            unsigned long long size = zip_entry_size(zip);
            unsigned int crc32 = zip_entry_crc32(zip);
        }
        zip_entry_close(zip);
    }
    zip_close(zip);
}


/* Compress folder (recursively). */

void zip_walk(struct zip_t *zip, const char *path) {
    DIR *dir;
    struct dirent *entry;
    char fullpath[MAX_PATH];
    struct stat s;

    memset(fullpath, 0, MAX_PATH);
    dir = opendir(path);
    assert(dir);

    while ((entry = readdir(dir))) {
      // skip "." and ".."
      if (!strcmp(entry->d_name, ".\0") || !strcmp(entry->d_name, "..\0"))
        continue;

      snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
      stat(fullpath, &s);
      if (S_ISDIR(s.st_mode))
        zip_walk(zip, fullpath);
      else {
        zip_entry_open(zip, fullpath);
        zip_entry_fwrite(zip, fullpath);
        zip_entry_close(zip);
      }
    }

    closedir(dir);
}

void compress_folder_recursively(void) {}

/* Prepare the expected foo.zip file. */
int main(void) {
    create();
    append();
    return 0;
}
