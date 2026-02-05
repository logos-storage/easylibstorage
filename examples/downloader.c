/* downloader.c: Download files from a Logos Storage node into the local disk.
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
    if (argc < 4) {
        printf("Usage: %s BOOTSTRAP_SPR CID <output_file>\n", argv[0]);
        exit(1);
    }

    char *spr = argv[1];
    char *cid = argv[2];
    char *filepath = argv[3];

    node_config cfg = {
            .api_port = 8081,
            .disc_port = 9091,
            .data_dir = "./downloader-data",
            .log_level = "INFO",
            .bootstrap_node = spr,
            .nat = "none",
    };

    STORAGE_NODE node = e_storage_new(cfg);
    if (e_storage_start(node) != RET_OK) panic("Failed to start storage node");
    if (e_storage_download(node, cid, filepath, progress) != RET_OK) panic("Failed to download file");
    e_storage_stop(node);
    e_storage_close(node);
    e_storage_destroy(node);
}
