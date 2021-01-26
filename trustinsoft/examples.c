#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

#include "zip.h"

/* -- Create a new zip archive with default compression level. -- */

/* REQUIRES: foo-2.1.txt, foo-2.2.txt, foo-2.3.txt */
int create (void) {
    int result = 0;

    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        result = zip_entry_open(zip, "foo-1.txt");
        if (result != 0) goto create_done;
        {
            const char *buf = "Some data here...\0";
            result = zip_entry_write(zip, buf, strlen(buf));
            if (result != 0) goto create_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto create_done;

        result = zip_entry_open(zip, "foo-2.txt");
        if (result != 0) goto create_done;
        {
            // merge 3 files into one entry and compress them on-the-fly.
            result = zip_entry_fwrite(zip, "foo-2.1.txt");
            if (result != 0) goto create_done;
            result = zip_entry_fwrite(zip, "foo-2.2.txt");
            if (result != 0) goto create_done;
            result = zip_entry_fwrite(zip, "foo-2.3.txt");
            if (result != 0) goto create_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto create_done;
    }
    zip_close(zip);

create_done:
    return result;
}


/* -- Append to the existing zip archive. -- */

/* REQUIRES: foo.zip */
int append (void) {
    int result = 0;

    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
    {
        result = zip_entry_open(zip, "foo-3.txt");
        if (result != 0) goto append_done;
        {
            const char *buf = "Append some data here...\0";
            result = zip_entry_write(zip, buf, strlen(buf));
            if (result != 0) goto append_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto append_done;
    }
    zip_close(zip);

append_done:
    return result;
}


/* -- Extract a zip archive into a folder. -- */

int on_extract_entry(const char *filename, void *arg) {
    static int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);
    return 0;
}

/* REQUIRES: foo.zip */
int extract_into_a_folder (void) {
    int arg = 2;
    return zip_extract("foo.zip", "/tmp", on_extract_entry, &arg);
}


/* -- Extract a zip entry into memory. -- */

/* REQUIRES: foo.zip */
int extract_into_memory (void) {
    int result = 0;

    void *buf = NULL;
    size_t bufsize;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        result = zip_entry_open(zip, "foo-1.txt");
        if (result != 0) goto extract_into_memory_done;
        {
            result = zip_entry_read(zip, &buf, &bufsize);
            if (result != 0) goto extract_into_memory_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto extract_into_memory_done;
    }
    zip_close(zip);

    free(buf);

extract_into_memory_done:
    return result;
}


/* -- Extract a zip entry into memory (no internal allocation). -- */

/* REQUIRES: foo.zip */
int extract_into_memory_no_allocation (void) {
    int result = 0;

    unsigned char *buf;
    size_t bufsize;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        result = zip_entry_open(zip, "foo-1.txt");
        if (result != 0) goto extract_into_memory_no_allocation_done;
        {
            bufsize = zip_entry_size(zip);
            buf = calloc(sizeof(unsigned char), bufsize);

            result = zip_entry_noallocread(zip, (void *)buf, bufsize);
            if (result != 0) goto extract_into_memory_no_allocation_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto extract_into_memory_no_allocation_done;
    }
    zip_close(zip);

    free(buf);

extract_into_memory_no_allocation_done:
    return result;
}


/* -- Extract a zip entry into a file. -- */

/* REQUIRES: foo.zip */
int extract_entry_into_a_file (void) {
    int result = 0;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    {
        result = zip_entry_open(zip, "foo-2.txt");
        if (result != 0) goto extract_entry_into_a_file_done;
        {
            result = zip_entry_fread(zip, "foo-2.txt");
            if (result != 0) goto extract_entry_into_a_file_done;
        }
        result = zip_entry_close(zip);
        if (result != 0) goto extract_entry_into_a_file_done;
    }
    zip_close(zip);

extract_entry_into_a_file_done:
    return result;
}


/* -- List of all zip entries. -- */

/* REQUIRES: foo.zip */
int list_all_entries (void) {
    int result = 0;

    struct zip_t *zip = zip_open("foo.zip", 0, 'r');
    int i, n = zip_total_entries(zip);
    for (i = 0; i < n; ++i) {
        result = zip_entry_openbyindex(zip, i);
        if (result != 0) goto list_all_entries_done;
        {
            const char *name = zip_entry_name(zip);
            int isdir = zip_entry_isdir(zip);
            unsigned long long size = zip_entry_size(zip);
            unsigned int crc32 = zip_entry_crc32(zip);
            printf("%s, is_dir=%d, size=%llu, crc32=%u\n", name, isdir, size, crc32);
        }
        result = zip_entry_close(zip);
        if (result != 0) goto list_all_entries_done;
    }
    zip_close(zip);

list_all_entries_done:
    return result;
}


/* -- Compress folder (recursively). -- */

int zip_walk(struct zip_t *zip, const char *path) {
    DIR *dir;
    struct dirent *entry;
    char fullpath[MAX_PATH];
    struct stat s;

    printf("zip_walk: BEGIN %s\n", path);
    memset(fullpath, 0, MAX_PATH);
    dir = opendir(path);
    assert(dir);

    while ((entry = readdir(dir))) {
      // skip "." and ".."
      if (!strcmp(entry->d_name, ".\0") || !strcmp(entry->d_name, "..\0"))
        continue;

      snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
      stat(fullpath, &s);
      if (S_ISDIR(s.st_mode)) {
        printf("zip_walk: WALK %s\n", fullpath);
        zip_walk(zip, fullpath);
      }
      else {
        printf("zip_walk: WRITE %s\n", fullpath);
        zip_entry_open(zip, fullpath);
        zip_entry_fwrite(zip, fullpath);
        zip_entry_close(zip);
      }
    }

    closedir(dir);
    printf("zip_walk: END %s\n", path);

    return 0;
}

int compress_folder_recursively(void) {
    int result = 0;
    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    char path[] = "input";
    result = zip_walk(zip, path);
    zip_close(zip);
    return result;
}


/* -- Prepare the expected "foo.zip" file. -- */
int main(void) {
    create();
    append();
    return 0;
}
