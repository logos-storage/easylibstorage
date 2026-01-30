#include <stdio.h>
#include <stdlib.h>
#include "easystorage.h"

void progress(int total, int complete, int status) {
    printf("\r  %d / %d bytes", complete, total);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <spr> <cid> <output_file>\n", argv[0]);
        exit(1);
    }

    node_config cfg = {
            .api_port = 8081,
            .disc_port = 9091,
            .data_dir = "./downloader-data",
            .log_level = "INFO",
            .bootstrap_node = argv[1],
    };

    STORAGE_NODE node = e_storage_new(cfg);
    e_storage_start(node);
    e_storage_download(node, argv[2], argv[3], progress);
    e_storage_stop(node);
    e_storage_destroy(node);
}
