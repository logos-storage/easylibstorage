# libeasystorage

A simplified, higher-level C wrapper around [libstorage](https://github.com/status-im/logos-storage-nim). Includes
examples showing how to implement simple filesharing apps.

## Prerequisites

- CMake 3.14+
- C11 compiler
- libstorage (build or get binary from [logos-storage-nim](https://github.com/status-im/logos-storage-nim))

## Building

```bash
cmake -B build -DLOGOS_STORAGE_NIM_ROOT=/path/to/logos-storage-nim
cmake --build build
```

This produces three example executables:
- `storageconsole` — interactive CLI for managing a storage node
- `uploader` — uploads a local file and prints the CID and SPR
- `downloader` — downloads a file given a bootstrap SPR and CID

## API

```c
#include "easystorage.h"

// Create and start a node
node_config cfg = {
    .api_port = 8080,
    .disc_port = 9090,
    .data_dir = "./data",
    .log_level = "INFO",
    .bootstrap_node = "<SPR>",
    .nat = "auto"
};
STORAGE_NODE node = e_storage_new(cfg);
e_storage_start(node);

// Get the node's Signed Peer Record
char *spr = e_storage_spr(node);
free(spr);

// Upload a file
char *cid = e_storage_upload(node, "/path/to/file.txt", progress_cb);
// Download a file
e_storage_download(node, cid, "/path/to/output.txt", progress_cb);
free(cid);

// Cleanup
e_storage_stop(node);
e_storage_destroy(node);
```

Configuration can also be loaded from an INI file:

```ini
[easystorage]
api-port=8080
disc-port=9090
data-dir=./data
log-level=INFO
bootstrap-node=spr:...
nat=none
```

```c
node_config cfg = {0};
e_storage_read_config("config.ini", &cfg);
// ... use cfg ...
e_storage_free_config(&cfg);
```

## Examples

### storageconsole

An interactive CLI included in `examples/storageconsole.c`:

```bash
./build/storageconsole
```

Commands: `help`, `start`, `stop`, `upload`, `download`, `quit`.

### uploader / downloader

Standalone programs that demonstrate file sharing between two nodes. On one machine, start the uploader:

```bash
./build/uploader ./myfile.txt
# prints: Run: downloader <SPR> <CID> ./output-file
```

On another (or the same machine), run the downloader with the printed values:

```bash
./build/downloader <SPR> <CID> ./output-file
```

## Testing

```bash
cmake --build build
ctest --test-dir build
```

Tests use a mock libstorage implementation and do not require a running storage node.

## Project Structure

```
├── easystorage.h             # Public API
├── easystorage.c             # Implementation
├── CMakeLists.txt
├── examples/
│   ├── storageconsole.c      # Interactive CLI
│   ├── uploader.c            # File upload example
│   └── downloader.c          # File download example
├── tests/
│   ├── test_runner.c         # Unit tests
│   └── mock_libstorage.c     # Mock libstorage for testing
└── vendor/
    └── inih/                 # Vendored INI file parser
```

## License

Dual-licensed under [Apache 2.0](LICENSE-APACHEv2) and [MIT](LICENSE-MIT).
