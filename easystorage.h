#ifndef EASYSTORAGE_H
#define EASYSTORAGE_H

#define STORAGE_NODE void *

typedef struct {
    int api_port;
    int disc_port;
    char *data_dir;
    char *log_level;
    char *bootstrap_node;
    char *nat;
} node_config;

typedef void (*progress_callback)(int total, int complete, int status);

// Creates a new storage node. Returns opaque pointer, or NULL on failure.
STORAGE_NODE e_storage_new(node_config config);

int e_storage_start(STORAGE_NODE node);
int e_storage_stop(STORAGE_NODE node);
int e_storage_destroy(STORAGE_NODE node);

// Retrieves the node's SPR (caller must free), or NULL on failure.
char *e_storage_spr(STORAGE_NODE node);

// Uploads a file. Returns CID string on success (caller must free), or NULL on failure.
char *e_storage_upload(STORAGE_NODE node, const char *filepath, progress_callback cb);

// Downloads content identified by cid to filepath. Returns 0 on success.
int e_storage_download(STORAGE_NODE node, const char *cid, const char *filepath, progress_callback cb);

#endif // EASYSTORAGE_H
