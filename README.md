# easystorage

A simplified, higher level C wrapper around [libstorage](https://github.com/status-im/logos-storage-nim) built to showcase it. Allows filesharing in the Logos Storage network. Comes with an example console application to interact with it.

## Prerequisites

- CMake 3.14+
- C11 compiler
- libstorage (build or get binary from [logos-storage-nim](https://github.com/status-im/logos-storage-nim))

## Building

```bash
cmake -B build -DLOGOS_STORAGE_NIM_ROOT=/path/to/logos-storage-nim
cmake --build build
```

This produces:
- `libeasystorage.so` — the shared library
- `storageconsole` — an example CLI application

## API

```c
#include "easystorage.h"

// Create and start a node
node_config cfg = {
    .api_port = 8080,
    .disc_port = 9090,
    .data_dir = "./data",
    .log_level = "INFO",
    .bootstrap_node = "<SPR>"
};
STORAGE_NODE node = e_storage_new(cfg);
e_storage_start(node);

// Upload a file
char *cid = e_storage_upload(node, "/path/to/file.txt", progress_cb);
free(cid);

// Download a file
e_storage_download(node, cid, "/path/to/output.txt", progress_cb);

// Cleanup
e_storage_stop(node);
e_storage_destroy(node);
```

## Example: storageconsole

An interactive CLI is included in `examples/storageconsole.c`:

```bash
./build/storageconsole
```

Commands: `help`, `start`, `stop`, `upload`, `download`, `quit`.

## Testing

```bash
cmake --build build
./build/test_runner
```

## Project Structure

```
easystorage/
├── easystorage.h         # Public API
├── easystorage.c         # Implementation
├── examples/
│   └── storageconsole.c  # CLI example
├── tests/
│   ├── test_runner.c
│   └── mock_libstorage.c
└── CMakeLists.txt
```

## License

Dual-licensed under [Apache 2.0](LICENSE-APACHEv2) and [MIT](LICENSE-MIT).
