#include <stdio.h>
#include <stdlib.h>
#include "easystorage.h"

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
    };

    STORAGE_NODE node = e_storage_new(cfg);
    e_storage_start(node);

    char *cid = e_storage_upload(node, argv[1], progress);
    char *spr = e_storage_spr(node);

    printf("\nCID: %s\n", cid);
    printf("SPR: %s\n", spr);

    printf("\nPress Enter to exit\n");
    getchar();

    e_storage_stop(node);
    e_storage_destroy(node);

    return 0;
}
