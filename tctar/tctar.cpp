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
    struct tc_iovec *output_file = (struct tc_iovec*) client_data;

    output_file->data = (char*) buffer;
    output_file->length = length;


    tc_res res = tc_writev(output_file, 1, false);
    if (!tc_okay(res)) {
        printf("writev: %s\n", strerror(res.err_no));
        return res.err_no;
    }
    output_file->offset += output_file->length;
    return output_file->length;
}

void deinit(int status, void *context) {
    tc_deinit(context);
}

struct cbarg {
    struct archive *a;
    std::vector<struct archive_entry *> files;
    std::vector<struct archive_entry *> symlinks;
};

tc_res read_files_and_write_to_archive(struct archive *a, std::vector<struct archive_entry *>& files,
        std::vector<struct archive_entry *>& symlinks, int min_count) {

    tc_res res = {0, 0};
    const char **path_names;
    struct tc_iovec *reads;

    if (files.size() >= min_count) {
        reads = (struct tc_iovec *) malloc(sizeof(struct tc_iovec) * files.size());
        for (int i = 0; i < files.size(); i++) {
            reads[i].file = tc_file_from_path(archive_entry_pathname(files[i]));
            reads[i].offset = 0;
            reads[i].length = archive_entry_size(files[i]);
            reads[i].is_creation = false;
            char *buf = (char *) malloc(reads[i].length);
            reads[i].data = buf;
        }
        res = tc_readv(reads, files.size(), false);
        if (!tc_okay(res)) {
            printf("readv: %s\n", strerror(res.err_no));
            return res;
        }
        for (int i = 0; i < files.size(); i++) {
            archive_write_header(a, files[i]);
            ssize_t w = archive_write_data(a, reads[i].data, reads[i].length);
            if (w != reads[i].length) {
                printf("could not write all data to archive\n");
            }
            archive_entry_free(files[i]);
            free(reads[i].data);
        }
        free(reads);
        files.clear();
    }
    if (symlinks.size() >= min_count) {
        path_names = (const char **) malloc(sizeof(const char *) * symlinks.size());
        char **bufs = (char **) malloc(sizeof(char *) * symlinks.size());
        size_t *bufsizes = (size_t *) malloc(sizeof(size_t) * symlinks.size());
        for (int i = 0; i < symlinks.size(); i++) {
            path_names[i] = archive_entry_pathname(symlinks[i]);
            bufs[i] = (char *) malloc(sizeof(char) * PATH_MAX);
            bufsizes[i] = PATH_MAX;
        }
        res = tc_readlinkv(path_names, bufs, bufsizes, symlinks.size(), false);
        if (!tc_okay(res)) {
            printf("readlinkv: %s\n", strerror(res.err_no));
            return res;
        }
        for (int i = 0; i < symlinks.size(); i++) {
            archive_entry_set_symlink(symlinks[i], bufs[i]);
            archive_write_header(a, symlinks[i]);
            archive_entry_free(symlinks[i]);
            free(bufs[i]);
        }
        free(bufs);
        free(bufsizes);
        symlinks.clear();
    }
    return res;
}

bool listdir_callback(const struct tc_attrs *attr, const char *dir, void *arg) {
    struct cbarg *cbarg = (struct cbarg *) arg;
    struct archive_entry *entry = archive_entry_new();

    archive_entry_copy_pathname(entry, attr->file.path);
    archive_entry_set_size(entry, attr->size);
    archive_entry_set_mode(entry, attr->mode);

    if (S_ISDIR(attr->mode)) {
        archive_write_header(cbarg->a, entry);
        archive_entry_free(entry);
    } else if (S_ISREG(attr->mode)) {
        cbarg->files.push_back(entry);
    } else if (S_ISLNK(attr->mode)) {
        cbarg->symlinks.push_back(entry);
    } else {
        printf("unsupported file type\n");
    }

    if (!tc_okay(read_files_and_write_to_archive(cbarg->a, cbarg->files, cbarg->symlinks, 255))) {
        return false;
    }

    return true;
}


int main(int argc, char **argv) {
    char exe_path[PATH_MAX];
    char tc_config_path[PATH_MAX];
    void *context;
    struct archive *a;
    int r;
    tc_res res;
    struct cbarg *cbarg = (struct cbarg *) malloc(sizeof(struct cbarg));
    struct tc_iovec *output_file = (struct tc_iovec *) malloc(sizeof(struct tc_iovec));

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
    archive_write_add_filter_xz(a);
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
    cbarg->files = {};
    cbarg->symlinks = {};
    res = tc_listdirv((const char **) &argv[2], 1, listdir_mask, 0, true, listdir_callback, cbarg, false);
    if (!tc_okay(res)) {
        printf("listdir: %s\n", strerror(res.err_no));
        return res.err_no;
    }
    if (!tc_okay(read_files_and_write_to_archive(cbarg->a, cbarg->files, cbarg->symlinks, 1))) {
        return 1;
    }

    r = archive_write_free(a);
    free(output_file);
    free(cbarg);
    if (r != ARCHIVE_OK) {
        printf("error, could not free archive");
        return 1;
    }
}
