#ifndef EASYSTORAGE_H
#define EASYSTORAGE_H

#include <stdio.h>

#define STORAGE_NODE void *
#define RET_OK 0
#define RET_ERR 1

typedef struct {
    int api_port;
    int disc_port;
    char *data_dir;
    char *log_level;
    char *bootstrap_node;
    char *nat;
} node_config;

extern const node_config DEFAULT_STORAGE_NODE_CONFIG;

typedef void (*progress_callback)(int total, int complete, int status);

// Creates a new storage node. Returns opaque pointer, or NULL on failure.
STORAGE_NODE e_storage_new(node_config config);

int e_storage_start(STORAGE_NODE node);
int e_storage_stop(STORAGE_NODE node);
int e_storage_close(STORAGE_NODE node);
int e_storage_destroy(STORAGE_NODE node);

// Retrieves the node's SPR (caller must free), or NULL on failure.
char *e_storage_spr(STORAGE_NODE node);

// Uploads a file. Returns CID string on success (caller must free), or NULL on failure.
char *e_storage_upload(STORAGE_NODE node, const char *filepath, progress_callback cb);

// Downloads content identified by cid to filepath. Returns 0 on success.
int e_storage_download(STORAGE_NODE node, const char *cid, const char *filepath, progress_callback cb);

// Deletes a previously uploaded file from the node.
int e_storage_delete(STORAGE_NODE node, const char *cid);

// Config handling utilities. Note that for e_storage_read_config and e_storage_read_config, the
// caller is responsible for freeing the config object and its members.
int e_storage_read_config(char *filepath, node_config *config);
int e_storage_read_config_file(FILE *, node_config *config);
void e_storage_free_config(node_config *config);

#endif // EASYSTORAGE_H
