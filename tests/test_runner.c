#include "easystorage.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RET_OK 0
#define RET_ERR 1

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(fn)                                                                                                   \
    do {                                                                                                               \
        tests_run++;                                                                                                   \
        printf("  %-30s", #fn);                                                                                        \
        fn();                                                                                                          \
        tests_passed++;                                                                                                \
        printf(" OK\n");                                                                                               \
    } while (0)

static node_config default_config(void) {
    node_config cfg = {0};
    cfg.api_port = 8080;
    cfg.disc_port = 8090;
    cfg.data_dir = "./test-data";
    cfg.log_level = "WARN";
    cfg.bootstrap_node = NULL;
    return cfg;
}

FILE *write_to_temp(const char *contents) {
    FILE *fp = tmpfile();
    assert(fp != NULL);
    assert(fwrite(contents, 1, strlen(contents), fp) == strlen(contents));
    assert(fseek(fp, 0, SEEK_SET) == 0);
    return fp;
}

// --- Tests ---

static void test_new(void) {
    node_config cfg = default_config();
    cfg.bootstrap_node = "spr:abc123";
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
}

static void test_new_defaults(void) {
    node_config cfg = {0};
    cfg.api_port = 9000;
    cfg.disc_port = 9010;
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
}

static void test_start(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    int ret = e_storage_start(node);
    assert(ret == RET_OK);
}

static void test_start_null(void) {
    int ret = e_storage_start(NULL);
    assert(ret == RET_ERR);
}

static void test_stop(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    e_storage_start(node);
    int ret = e_storage_stop(node);
    assert(ret == RET_OK);
}

static void test_destroy(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    int ret = e_storage_destroy(node);
    assert(ret == RET_OK);
}

static void test_destroy_null(void) {
    int ret = e_storage_destroy(NULL);
    assert(ret == RET_ERR);
}

static void test_upload(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    e_storage_start(node);

    char *cid = e_storage_upload(node, "/tmp/test.txt", NULL);
    assert(cid != NULL);
    assert(strlen(cid) > 0);
    free(cid);
}

static void test_upload_null(void) {
    char *cid = e_storage_upload(NULL, "/tmp/test.txt", NULL);
    assert(cid == NULL);
}

static void test_download(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    e_storage_start(node);

    int ret = e_storage_download(node, "zDvZRwzmSomeCid", "/tmp/out.dat", NULL);
    assert(ret == RET_OK);
}

static void test_download_null(void) {
    int ret = e_storage_download(NULL, "zDvZRwzmSomeCid", "/tmp/out.dat", NULL);
    assert(ret == RET_ERR);
}

static void test_should_delete_file(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    e_storage_start(node);

    char *cid = e_storage_upload(node, "/tmp/test.txt", NULL);
    assert(cid != NULL);
    assert(strlen(cid) > 0);

    assert(e_storage_delete(node, cid) == RET_OK);
    // Non-existing files can't be deleted.
    assert(e_storage_delete(node, cid) == RET_ERR);
    free(cid);
}

static void test_get_should_get_node_spr(void) {
    node_config cfg = default_config();
    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);
    assert(e_storage_start(node) == RET_OK);

    const char *sprprefix = "spr:CiUIAhIhA";
    const char *spr = e_storage_spr(node);
    assert(spr != NULL);
    assert(strlen(spr) > 0);
    assert(strncmp(sprprefix, spr, strlen(sprprefix)) == 0);
}

static void test_full_lifecycle(void) {
    node_config cfg = default_config();
    cfg.bootstrap_node = "spr:node1";

    STORAGE_NODE node = e_storage_new(cfg);
    assert(node != NULL);

    assert(e_storage_start(node) == RET_OK);

    char *cid = e_storage_upload(node, "/tmp/lifecycle.txt", NULL);
    assert(cid != NULL);
    assert(strlen(cid) > 0);

    assert(e_storage_download(node, cid, "/tmp/lifecycle_out.dat", NULL) == RET_OK);
    free(cid);

    assert(e_storage_stop(node) == RET_OK);
    assert(e_storage_destroy(node) == RET_OK);
}

static void test_should_read_configuration_file(void) {
    const char *conf = "[easystorage]                                    \n"
                       "bootstrap-node=spr:CiUIAhIhA-VlcoiRm02KyIzrcTP   \n"
                       "data-dir=/home/user/data-dir                     \n"
                       "log-level=WARN                                   \n"
                       "api-port=8081                                    \n"
                       "disc-port=8091                                   \n"
                       "nat=none                                         \n";

    node_config cfg = DEFAULT_STORAGE_NODE_CONFIG;
    FILE *cfg_file = write_to_temp(conf);
    assert(e_storage_read_config_file(cfg_file, &cfg) == RET_OK);
    fclose(cfg_file);

    assert(strcmp(cfg.bootstrap_node, "spr:CiUIAhIhA-VlcoiRm02KyIzrcTP") == 0);
    assert(strcmp(cfg.data_dir, "/home/user/data-dir") == 0);
    assert(strcmp(cfg.log_level, "WARN") == 0);
    assert(cfg.api_port == 8081);
    assert(cfg.disc_port == 8091);
    assert(strcmp(cfg.nat, "none") == 0);

    e_storage_free_config(&cfg);
}

int main(void) {
    printf("Running easylibstorage tests...\n");

    RUN_TEST(test_new);
    RUN_TEST(test_new_defaults);
    RUN_TEST(test_start);
    RUN_TEST(test_start_null);
    RUN_TEST(test_stop);
    RUN_TEST(test_destroy);
    RUN_TEST(test_destroy_null);
    RUN_TEST(test_upload);
    RUN_TEST(test_upload_null);
    RUN_TEST(test_download);
    RUN_TEST(test_download_null);
    RUN_TEST(test_should_delete_file);
    RUN_TEST(test_get_should_get_node_spr);
    RUN_TEST(test_full_lifecycle);
    RUN_TEST(test_should_read_configuration_file);

    printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
