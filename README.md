# libeasystorage

A simplified, higher-level C wrapper around [libstorage](https://github.com/status-im/logos-storage-nim). Includes
examples showing how to implement simple filesharing apps.

## Prerequisites

- CMake 3.14+
- C11 compiler
- libstorage v0.3.2 (build or get binary from [logos-storage-nim](https://github.com/status-im/logos-storage-nim))

## Building

```bash
cmake -B build -DLOGOS_STORAGE_NIM_ROOT=/path/to/logos-storage-nim -DCOMPILE_TUTORIAL_EXAMPLES=1
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

Standalone programs that demonstrate file sharing between two nodes. 

Optionally generate a file to upload. For example, using `dd` on nix-based systems:

```bash
# generate a 500MB file with random contents
dd if=/dev/urandom of=myfile.txt bs=1048576 count=500
```

In one terminal, start the uploader, passing in the path to a file to upload:

```bash
./build/uploader ./myfile.txt
# prints: Run: downloader <SPR> <CID> ./output-file
```

In another terminal, run the downloader with the printed values:

```bash
./build/downloader <SPR> <CID> ./output-file
```

#### Connecting to remote peers

To run the uploader/downloader example, but connect to peers remotely, the config will need to be updated before it will work.

##### UPnP / PMP

The easiest way to connect to remote peers is to enable UPnP or PMP in your router. Once this is enabled, update the config to:

```c
// uploader.c
 node_config cfg = {
    .api_port = 8080,
    .disc_port = 9090,
    .data_dir = "./uploader-data",
    .log_level = "TRACE",
    .bootstrap_node = "spr:CiUIAhIhApIj9p6zJDRbw2NoCo-tj98Y760YbppRiEpGIE1yGaMzEgIDARpJCicAJQgCEiECkiP2nrMkNFvDY2gKj62P3xjvrRhumlGISkYgTXIZozMQvcz8wQYaCwoJBAWhF3WRAnVEGgsKCQQFoRd1kQJ1RCpGMEQCIFZB84O_nzPNuViqEGRL1vJTjHBJ-i5ZDgFL5XZxm4HAAiB8rbLHkUdFfWdiOmlencYVn0noSMRHzn4lJYoShuVzlw",
    .nat = "upnp" // or "pmp" or "any",
};

// downlaoder.c
node_config cfg = {
    .api_port = 8081,
    .disc_port = 9091,
    .data_dir = "./downloader-data",
    .log_level = "TRACE",
    .bootstrap_node = "spr:CiUIAhIhApIj9p6zJDRbw2NoCo-tj98Y760YbppRiEpGIE1yGaMzEgIDARpJCicAJQgCEiECkiP2nrMkNFvDY2gKj62P3xjvrRhumlGISkYgTXIZozMQvcz8wQYaCwoJBAWhF3WRAnVEGgsKCQQFoRd1kQJ1RCpGMEQCIFZB84O_nzPNuViqEGRL1vJTjHBJ-i5ZDgFL5XZxm4HAAiB8rbLHkUdFfWdiOmlencYVn0noSMRHzn4lJYoShuVzlw",
    .nat = "upnp" // or "pmp" or "any",
};
```

By specifying a common bootstrap node, the uploader and downloader will be able to share DHT entries. Specifically, the uploader will announce itself as a provider for the CID in the DHT to its closes neighbours (the bootstrap node), and the downloader will be able to traverse its closest peers when finding a provider for the CID.

#### Connecting to peers on the same network or machine

To connect to peers locally, the same config updates need to be made, except now, `nat` should be set to `none`:

```c
// uploader.c
 node_config cfg = {
    .api_port = 8080,
    .disc_port = 9090,
    .data_dir = "./uploader-data",
    .log_level = "TRACE",
    .bootstrap_node = "spr:CiUIAhIhApIj9p6zJDRbw2NoCo-tj98Y760YbppRiEpGIE1yGaMzEgIDARpJCicAJQgCEiECkiP2nrMkNFvDY2gKj62P3xjvrRhumlGISkYgTXIZozMQvcz8wQYaCwoJBAWhF3WRAnVEGgsKCQQFoRd1kQJ1RCpGMEQCIFZB84O_nzPNuViqEGRL1vJTjHBJ-i5ZDgFL5XZxm4HAAiB8rbLHkUdFfWdiOmlencYVn0noSMRHzn4lJYoShuVzlw",
    .nat = "none"
};

// downlaoder.c
node_config cfg = {
    .api_port = 8081,
    .disc_port = 9091,
    .data_dir = "./downloader-data",
    .log_level = "TRACE",
    .bootstrap_node = "spr:CiUIAhIhApIj9p6zJDRbw2NoCo-tj98Y760YbppRiEpGIE1yGaMzEgIDARpJCicAJQgCEiECkiP2nrMkNFvDY2gKj62P3xjvrRhumlGISkYgTXIZozMQvcz8wQYaCwoJBAWhF3WRAnVEGgsKCQQFoRd1kQJ1RCpGMEQCIFZB84O_nzPNuViqEGRL1vJTjHBJ-i5ZDgFL5XZxm4HAAiB8rbLHkUdFfWdiOmlencYVn0noSMRHzn4lJYoShuVzlw",
    .nat = "none"
};
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
