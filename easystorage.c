#include "easystorage.h"
#include "libstorage.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_RETRIES 250
#define POLL_INTERVAL_US (100 * 1000)
#define DEFAULT_CHUNK_SIZE (64 * 1024)

typedef struct {
    int ret;
    char *msg;
    size_t len;
    progress_callback pcb;
    int bytes_done;
    bool unreferenced;
} resp;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static resp *resp_alloc(void) {
    resp *r = calloc(1, sizeof(resp));
    r->ret = -1;
    return r;
}

static void resp_destroy(resp *r) {
    if (!r) return;
    if (r->msg) free(r->msg);
    free(r);
}

static void resp_wait(resp *r) {
    for (int i = 0; i < MAX_RETRIES && r->ret == -1; i++) {
        usleep(POLL_INTERVAL_US);
    }
}

// Callback for simple (non-progress) async operations.
static void on_complete(int ret, const char *msg, size_t len, void *userData) {
    resp *r = userData;
    if (!r) return;

    pthread_mutex_lock(&mutex);
    if (r->unreferenced) {
        resp_destroy(r);
        pthread_mutex_unlock(&mutex);
        return;
    }

    if (msg && len > 0) {
        r->msg = malloc(len + 1);
        if (r->msg) {
            memcpy(r->msg, msg, len);
            r->msg[len] = '\0';
            r->len = len;
        }
    }

    r->ret = ret;
    r->unreferenced = true;
    pthread_mutex_unlock(&mutex);
}

// Callback for operations that report progress before completing.
static void on_progress(int ret, const char *msg, size_t len, void *userData) {
    resp *r = userData;
    if (!r) return;

    pthread_mutex_lock(&mutex);

    if (r->unreferenced) {
        resp_destroy(r);
        pthread_mutex_unlock(&mutex);
        return;
    }

    if (ret == RET_PROGRESS) {
        r->bytes_done += (int) len;
        if (r->pcb) {
            r->pcb(0, r->bytes_done, ret);
        }
        pthread_mutex_unlock(&mutex);
        return; // don't set r->ret yet — still in progress
    }

    if (msg && len > 0) {
        r->msg = malloc(len + 1);
        if (r->msg) {
            memcpy(r->msg, msg, len);
            r->msg[len] = '\0';
            r->len = len;
        }
    }

    r->ret = ret;
    r->unreferenced = true;

    pthread_mutex_unlock(&mutex);
}

// Dispatches an async call, waits for completion, extracts the result.
// Returns RET_OK/RET_ERR. If dispatch_ret != RET_OK, returns RET_ERR immediately.
// If **out is non-NULL, allocates a buffer and copies the content of
// resp->msg onto it, which the caller must then free.
static int call_wait(int dispatch_ret, resp *r, char **out) {
    if (dispatch_ret != RET_OK) {
        resp_destroy(r);
        return RET_ERR;
    }

    resp_wait(r);

    pthread_mutex_lock(&mutex);
    int result = (r->ret == RET_OK) ? RET_OK : RET_ERR;

    if (out) {
        *out = r->msg ? strdup(r->msg) : NULL;
    }

    if (r->unreferenced) {
        resp_destroy(r);
    } else {
        r->unreferenced = true;
    }
    pthread_mutex_unlock(&mutex);
    return result;
}

static int nim_initialized = 0;

STORAGE_NODE e_storage_new(node_config config) {
    if (!nim_initialized) {
        extern void libstorageNimMain(void);
        libstorageNimMain();
        nim_initialized = 1;
    }

    // Build JSON config string.
    // Format: {"api-port":N,"disc-port":N,"data-dir":"...","log-level":"...","bootstrap-node":["..."], "nat": "..."}
    char json[3096];
    int pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "{\"api-port\":%d,\"disc-port\":%d", config.api_port,
                    config.disc_port);

    if (config.data_dir) {
        pos += snprintf(json + pos, sizeof(json) - pos, ",\"data-dir\":\"%s\"", config.data_dir);
    }

    if (config.log_level) {
        pos += snprintf(json + pos, sizeof(json) - pos, ",\"log-level\":\"%s\"", config.log_level);
    }

    if (config.bootstrap_node) {
        pos += snprintf(json + pos, sizeof(json) - pos, ",\"bootstrap-node\":[\"%s\"]", config.bootstrap_node);
    }

    if (config.nat) {
        pos += snprintf(json + pos, sizeof(json) - pos, ",\"nat\":\"%s\"", config.nat);
    }

    snprintf(json + pos, sizeof(json) - pos, "}");

    resp *r = resp_alloc();
    void *ctx = storage_new(json, (StorageCallback) on_complete, r);

    if (!ctx) {
        resp_destroy(r);
        return NULL;
    }

    resp_wait(r);

    if (r->ret != RET_OK) {
        resp_destroy(r);
        return NULL;
    }

    resp_destroy(r);
    return ctx;
}

int e_storage_start(STORAGE_NODE node) {
    if (!node)
        return RET_ERR;
    resp *r = resp_alloc();
    return call_wait(storage_start(node, (StorageCallback) on_complete, r), r, NULL);
}

int e_storage_stop(STORAGE_NODE node) {
    if (!node)
        return RET_ERR;
    resp *r = resp_alloc();
    return call_wait(storage_stop(node, (StorageCallback) on_complete, r), r, NULL);
}

int e_storage_destroy(STORAGE_NODE node) {
    if (!node)
        return RET_ERR;

    // Close first (tolerate failure)
    resp *r = resp_alloc();
    call_wait(storage_close(node, (StorageCallback) on_complete, r), r, NULL);

    // Destroy
    r = resp_alloc();
    return call_wait(storage_destroy(node, (StorageCallback) on_complete, r), r, NULL);
}

char *e_storage_spr(STORAGE_NODE node) {
    if (!node)
        return NULL;
    resp *r = resp_alloc();
    char *spr = NULL;
    int ret = call_wait(storage_spr(node, (StorageCallback) on_complete, r), r, &spr);
    if (ret != RET_OK) {
        return NULL;
    }
    return spr;
}

char *e_storage_upload(STORAGE_NODE node, const char *filepath, progress_callback cb) {
    if (!node || !filepath)
        return NULL;

    // Init upload session
    resp *r = resp_alloc();
    char *session_id = NULL;
    int ret = call_wait(storage_upload_init(node, filepath, DEFAULT_CHUNK_SIZE, (StorageCallback) on_complete, r), r,
                        &session_id);
    if (ret != RET_OK || !session_id) {
        free(session_id);
        return NULL;
    }

    // Upload file with progress
    r = resp_alloc();
    r->pcb = cb;
    char *cid = NULL;
    ret = call_wait(storage_upload_file(node, session_id, (StorageCallback) on_progress, r), r, &cid);
    free(session_id);

    if (ret != RET_OK) {
        free(cid);
        return NULL;
    }

    if (cb) {
        printf("\n"); // newline after progress output
    }

    return cid;
}

int e_storage_download(STORAGE_NODE node, const char *cid, const char *filepath, progress_callback cb) {
    if (!node || !cid || !filepath)
        return RET_ERR;

    // Init download
    resp *r = resp_alloc();
    int ret = call_wait(storage_download_init(node, cid, DEFAULT_CHUNK_SIZE, false, (StorageCallback) on_complete, r),
                        r, NULL);
    if (ret != RET_OK)
        return RET_ERR;

    // Stream to file with progress
    r = resp_alloc();
    r->pcb = cb;
    ret = call_wait(
            storage_download_stream(node, cid, DEFAULT_CHUNK_SIZE, false, filepath, (StorageCallback) on_progress, r),
            r, NULL);

    if (cb) {
        printf("\n");
    }

    return ret;
}
