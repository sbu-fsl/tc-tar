#include <stdio.h>
#include <vector>
#include <libgen.h>

#include "archive.h"
#include "archive_entry.h"

#include "tc_api.h"
#include "tc_helper.h"
#include "path_utils.h"

#define DEFAULT_LOG_FILE "/tmp/libarchive-tctar.log"

ssize_t tc_archive_write(struct archive *a, void *client_data, const void *buffer, size_t length)
{


    struct tc_iovec *output_file = (tc_iovec*) client_data;

    output_file->data = (char*) buffer;
    output_file->length = length;


    tc_res res = tc_writev(output_file, 1, false);
    if (!tc_okay(res)) {
        printf("writev: %s\n", strerror(res.err_no));
        return res.err_no;
    }
    output_file->offset += length;
    return length;
}

void deinit(int status, void *context) {
    tc_deinit(context);
}

struct cbarg {
    struct archive *a;
    std::vector<struct tc_iovec> reads;
    std::vector<const char *> symlinks;
};

bool listdir_callback(const struct tc_attrs *attr, const char *dir, void *arg) {
    struct cbarg *cbarg = (struct cbarg *) arg;
    struct archive_entry *entry = archive_entry_new();

    archive_entry_copy_pathname(entry, attr->file.path);
    archive_entry_set_mode(entry, attr->mode);

    if (S_ISDIR(attr->mode)) {
        archive_write_header(cbarg->a, entry);
    } else if (S_ISREG(attr->mode)) {
        struct tc_iovec;
    } else if (S_ISLNK(attr->mode)) {

    } else {
        printf("unsupported file type\n");
    }
    return true;
}


int main(int argc, char **argv) {
    char exe_path[PATH_MAX];
    char tc_config_path[PATH_MAX];
    void *context;
    struct archive *a;
    struct archive_entry *entry;
    int r;
    tc_res res;
    struct cbarg *cbarg = (struct cbarg *) malloc(sizeof(struct cbarg));
    struct tc_iovec *output_file = (tc_iovec*) malloc(sizeof(struct tc_iovec));

    readlink("/proc/self/exe", exe_path, PATH_MAX);
    snprintf(tc_config_path, PATH_MAX,
                 "%s/../../../../config/tc.ganesha.conf", dirname(exe_path));
    fprintf (stderr, "using config file: %s\n", tc_config_path);

    context = tc_init (tc_config_path, DEFAULT_LOG_FILE, 77);
    if (!context)
    {
        printf("initializing tc failed\n");
        return 1;
    }
    if (on_exit(deinit, (void*)context) != 0) {
        fprintf(stderr, "warning: failed to register tc_deinit() on exit");
    }

    output_file->file = tc_file_from_path(argv[1]);
    output_file->offset = 0;
    output_file->is_creation = true;

    a = archive_write_new();
    archive_write_add_filter_bzip2(a);
    archive_write_set_format_ustar(a);
    r = archive_write_open(a, output_file, NULL, tc_archive_write, NULL);
    if (r != ARCHIVE_OK) {
        printf("error, could not open archive");
        return 1;
    }

    struct tc_attrs_masks listdir_mask = TC_ATTRS_MASK_NONE;
    listdir_mask.has_mode = true;
    listdir_mask.has_size = true;
    cbarg->a = a;
    cbarg->reads = {};
    res = tc_listdirv((const char **) &argv[2], 1, listdir_mask, 0, true, listdir_callback, &a, false);
    if (!tc_okay(res)) {
        printf("listdir: %s\n", strerror(res.err_no));
        return res.err_no;

    }

    free(output_file);
    free(cbarg);
    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        printf("error, could not free archive");
        return 1;
    }
}
