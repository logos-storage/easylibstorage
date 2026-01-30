#include "libstorage.h"

#include <stdlib.h>
#include <string.h>

// A fake context to return from storage_new.
static int fake_ctx_data = 42;

void libstorageNimMain(void) {
    // no-op
}

void *storage_new(const char *configJson, StorageCallback callback, void *userData) {
    if (callback) {
        callback(RET_OK, "ok", 2, userData);
    }
    return &fake_ctx_data;
}

int storage_start(void *ctx, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_OK, "started", 7, userData);
    }
    return RET_OK;
}

int storage_stop(void *ctx, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_OK, "stopped", 7, userData);
    }
    return RET_OK;
}

int storage_close(void *ctx, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_OK, "closed", 6, userData);
    }
    return RET_OK;
}

int storage_destroy(void *ctx, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_OK, "destroyed", 9, userData);
    }
    return RET_OK;
}

int storage_upload_init(void *ctx, const char *filepath, size_t chunkSize, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    // Return a fake session ID
    const char *session_id = "mock-session-123";
    if (callback) {
        callback(RET_OK, session_id, strlen(session_id), userData);
    }
    return RET_OK;
}

int storage_upload_file(void *ctx, const char *sessionId, StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    // Fire a progress callback first, then final OK with CID
    if (callback) {
        callback(RET_PROGRESS, "chunk", 5, userData);
        const char *cid = "zDvZRwzmAbCdEfGhIjKlMnOpQrStUvWxYz0123456789ABCD";
        callback(RET_OK, cid, strlen(cid), userData);
    }
    return RET_OK;
}

int storage_download_init(void *ctx, const char *cid, size_t chunkSize, bool local, StorageCallback callback,
                          void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_OK, "init", 4, userData);
    }
    return RET_OK;
}

int storage_download_stream(void *ctx, const char *cid, size_t chunkSize, bool local, const char *filepath,
                            StorageCallback callback, void *userData) {
    if (!ctx) return RET_ERR;
    if (callback) {
        callback(RET_PROGRESS, "data", 4, userData);
        callback(RET_OK, "done", 4, userData);
    }
    return RET_OK;
}
