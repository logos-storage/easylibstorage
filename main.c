#include "easylibstorage.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *ctx;
} console;

typedef void (*fn)(char *, console *);

struct command {
    const char *name;
    const char *desc;
    fn command;
};

int n_commands(void);
static const struct command commands[];

void cmd_help(char *args, console *c) {
    printf("Commands:\n");
    for (int i = 0; i < n_commands(); i++) {
        printf(" [%s]: %s\n", commands[i].name, commands[i].desc);
    }
}

void progress_print(int total, int complete, int status) {
    if (total > 0) {
        printf("\r  %d / %d bytes", complete, total);
    } else {
        printf("\r  %d bytes", complete);
    }
    fflush(stdout);
}

void cmd_start(char *args, console *c) {
    if (c->ctx) {
        printf("Node already running. Stop it first.\n");
        return;
    }

    int api_port = 0, disc_port = 0;
    char datadir[2048] = {0};
    char bootstrap[2048] = {0};

    if (!args || sscanf(args, "%d %d %2047s %2047s", &api_port, &disc_port, datadir, bootstrap) < 3) {
        printf("Usage: start [API_PORT] [DISC_PORT] [DATA_DIR] [BOOTSTRAP_NODE]\n");
        return;
    }

    node_config cfg = {0};
    cfg.api_port = api_port;
    cfg.disc_port = disc_port;
    cfg.data_dir = datadir;
    cfg.log_level = "INFO";
    cfg.bootstrap_node = bootstrap[0] ? bootstrap : NULL;

    printf("Creating node...\n");
    STORAGE_NODE node = e_storage_new(cfg);
    if (!node) {
        printf("Failed to create node.\n");
        return;
    }

    printf("Starting node...\n");
    if (e_storage_start(node) != 0) {
        printf("Failed to start node.\n");
        e_storage_destroy(node);
        return;
    }

    c->ctx = node;
    printf("Node started on API port %d, discovery port %d.\n", api_port, disc_port);
}

void cmd_stop(char *args, console *c) {
    if (!c->ctx) {
        printf("No node running.\n");
        return;
    }

    printf("Stopping node...\n");
    e_storage_stop(c->ctx);
    e_storage_destroy(c->ctx);
    c->ctx = NULL;
    printf("Node stopped.\n");
}

void cmd_upload(char *args, console *c) {
    if (!c->ctx) {
        printf("No node running. Start one first.\n");
        return;
    }

    if (!args || args[0] == '\0') {
        printf("Usage: upload [PATH]\n");
        return;
    }

    char resolved[PATH_MAX];
    if (!realpath(args, resolved)) {
        printf("File not found: %s\n", args);
        return;
    }

    printf("Uploading %s...\n", resolved);
    char *cid = e_storage_upload(c->ctx, resolved, progress_print);
    if (cid) {
        printf("CID: %s\n", cid);
        free(cid);
    } else {
        printf("Upload failed.\n");
    }
}

void cmd_download(char *args, console *c) {
    if (!c->ctx) {
        printf("No node running. Start one first.\n");
        return;
    }

    char cid[256] = {0};
    char path[2048] = {0};

    if (!args || sscanf(args, "%255s %2047s", cid, path) < 2) {
        printf("Usage: download [CID] [PATH]\n");
        return;
    }

    printf("Downloading %s to %s...\n", cid, path);
    if (e_storage_download(c->ctx, cid, path, progress_print) == 0) {
        printf("Download complete.\n");
    } else {
        printf("Download failed.\n");
    }
}

void cmd_quit(char *args, console *c) {
    if (c->ctx) {
        printf("Stopping node...\n");
        e_storage_stop(c->ctx);
        e_storage_destroy(c->ctx);
        c->ctx = NULL;
    }
    printf("Quitting...\n");
    exit(0);
}

static const struct command commands[] = {
    {"help", "prints this help message", cmd_help},
    {"quit", "quits this program", cmd_quit},
    {"start", "[API_PORT] [DISC_PORT] [DATA_DIR] [BOOTSTRAP_NODE] creates and starts a node", cmd_start},
    {"stop", "stops and destroys the node", cmd_stop},
    {"upload", "[PATH] uploads a file to the node", cmd_upload},
    {"download", "[CID] [PATH] downloads content to a file", cmd_download},
};

int n_commands(void) { return sizeof(commands) / sizeof(commands[0]); }

int main(void) {
    char buf[4096];
    console c;
    int i;

    c.ctx = NULL;

    printf("Welcome to storageconsole. Type 'help' for a list of commands.\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        buf[strcspn(buf, "\n")] = 0;

        if (buf[0] == '\0') {
            continue;
        }

        // Split at first space: cmd points to command name, rest points to arguments
        char *rest = strchr(buf, ' ');
        if (rest) {
            *rest = '\0';
            rest++;
            // Skip leading spaces in arguments
            while (*rest == ' ') rest++;
            if (*rest == '\0') rest = NULL;
        }

        for (i = 0; i < n_commands(); i++) {
            if (strcmp(buf, commands[i].name) == 0) {
                commands[i].command(rest, &c);
                break;
            }
        }

        if (i == n_commands()) {
            printf("Invalid command: %s\n", buf);
        }
    }
}
