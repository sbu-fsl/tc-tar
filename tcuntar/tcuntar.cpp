#include <stdio.h>
#include <vector>

#include "archive.h"
#include "archive_entry.h"

#include "tc_api.h"
#include "tc_helper.h"
#include "path_utils.h"

#define DEFAULT_LOG_FILE "/tmp/libarchive-tcminitar.log"

ssize_t tc_archive_read(struct archive *a, void *client_data, const void **buff)
{

    struct tc_iovec *input_file = (tc_iovec*) client_data;

    tc_res res = tc_readv(input_file, 1, false);
    if (!tc_okay(res)) {
        printf("readv: %s\n", strerror(res.err_no));
        return res.err_no;
    }
    *buff = input_file->data;
    input_file->offset += input_file->length;
    return input_file->length;
}

int main(int argc, char **argv) {
    char tc_config_path[PATH_MAX];
    void *context;
    struct archive *a;
    struct archive_entry *entry;
    int r;
    tc_res res;
    struct tc_iovec *input_file = (tc_iovec*) malloc(sizeof(struct tc_iovec));
    std::vector<struct tc_attrs> directories;
    std::vector<struct tc_iovec> writes;

    get_tc_config_file (tc_config_path, PATH_MAX);
    fprintf (stderr, "using config file: %s\n", tc_config_path);

    context = tc_init (NULL, DEFAULT_LOG_FILE, 77);
    if (!context)
    {
        printf("initializing tc failed\n");
        return 1;
    }

    const int read_size = 1024*1024;
    input_file->file = tc_file_from_path(argv[1]);
    input_file->offset = 0;
    input_file->length = read_size;
    input_file->data = (char*) malloc(read_size);

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    r = archive_read_open(a, input_file, NULL, tc_archive_read, NULL);
    if (r != ARCHIVE_OK) {
        printf("error, could not open archive");
        return 1;
    }
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        mode_t type = archive_entry_filetype(entry);
        if (S_ISDIR(type)) {
            struct tc_attrs dir;
            dir.file = tc_file_from_path(strdup(archive_entry_pathname(entry)));
            dir.masks = TC_ATTRS_MASK_NONE;
            dir.masks.has_mode = true;
            dir.mode = 0755;
            directories.push_back(dir);
        } else if (S_ISREG(type)) {
            size_t size = archive_entry_size(entry);
            void *buff = malloc(size);
            struct tc_iovec iovec;

            r = archive_read_data(a, buff, size);
            if (r != size) {
                printf("error: r != size\n");
            }

            iovec.file = tc_file_from_path(strdup(archive_entry_pathname(entry)));
            iovec.is_creation = true;
            iovec.offset = 0;
            iovec.length = size;
            iovec.data = (char*)buff;
            writes.push_back(iovec);

        }
    }

    res = tc_mkdirv(directories.data(), directories.size(), false);
    if (!tc_okay(res)) {
        printf("mkdirv: %s\n", strerror(res.err_no));
        return res.err_no;
    }

    res = tc_writev(writes.data(), writes.size(), false);
    if (!tc_okay(res)) {
        printf("writev: %s\n", strerror(res.err_no));
        return res.err_no;
    }

    for (auto& iovec : writes) {
        free((char*)iovec.file.path);
        free(iovec.data);
    }


    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        printf("error, could not free archive");
        return 1;
    }
}
