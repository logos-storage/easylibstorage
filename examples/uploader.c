/* uploader.c: makes a local file available to the Logos Storage network.
 */
#include <stdio.h>
#include <stdlib.h>
#include "easystorage.h"

void panic(const char *msg) {
    fprintf(stderr, "Panic: %s\n", msg);
    exit(1);
}

void progress(int total, int complete, int status) {
    printf("\r  %d / %d bytes", complete, total);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filepath>\n", argv[0]);
        exit(1);
    }

    node_config cfg = {
            .api_port = 8080,
            .disc_port = 9090,
            .data_dir = "./uploader-data",
            .log_level = "INFO",
            .bootstrap_node = NULL,
            .nat = "none",
    };

    char *filepath = argv[1];

    STORAGE_NODE node = e_storage_new(cfg);
    if (node == NULL) panic("Failed to create node");
    if (e_storage_start(node) != RET_OK) panic("Failed to start storage node");

    char *cid = e_storage_upload(node, filepath, progress);
    if (cid == NULL) panic("Failed to upload file to node");
    char *spr = e_storage_spr(node);
    if (spr == NULL) panic("Failed to obtain node's Signed Peer Record (SPR)");

    printf("Run: downloader %s %s ./output-file\n", spr, cid);
    printf("\nPress Enter to exit\n");
    getchar();

    printf("Deleting file (this could take a while)...");
    fflush(stdout);
    if (e_storage_delete(node, cid) != RET_OK) panic("Failed to delete file");
    printf("Done\n");

    free(cid);
    free(spr);
    e_storage_stop(node);
    e_storage_close(node);
    e_storage_destroy(node);

    return 0;
}
