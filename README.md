# Storage Console

A command-line interface for interacting with libstorage, providing simple file upload and download operations via a distributed storage network.

## Prerequisites

- CMake 3.14+
- C11 compiler
- libstorage (build or get binary from [logos-storage-nim](https://github.com/status-im/logos-storage-nim))

## Building

```bash
cmake -B build -DLOGOS_STORAGE_NIM_ROOT=/path/to/logos-storage-nim
cmake --build build
```

## Running

```bash
./build/storageconsole
```

## Commands

| Command | Description |
|---------|-------------|
| `help` | Prints available commands |
| `start [API_PORT] [DISC_PORT] [DATA_DIR] [BOOTSTRAP_NODE]` | Creates and starts a storage node |
| `stop` | Stops and destroys the node |
| `upload [PATH]` | Uploads a local file; displays progress and prints CID |
| `download [CID] [PATH]` | Downloads content by CID to a local path |
| `quit` | Exits the program |

### Example Session

```
> start 8080 9090 ./data <bootstrap SPR>
Creating node...
Starting node...
Node started on API port 8080, discovery port 9090.

> upload /path/to/file.txt
Uploading /path/to/file.txt...
  1024 / 1024 bytes
CID: QmXyz...

> download QmXyz... /path/to/output.txt
Downloading QmXyz... to /path/to/output.txt...
  1024 / 1024 bytes
Download complete.

> quit
Quitting...
```

## Testing

```bash
cmake --build build
./build/test_runner
```

## Project Structure

```
storageconsole/
├── main.c              # Console application
├── easylibstorage.h    # Simplified libstorage API
├── easylibstorage.c    # API implementation
├── tests/
│   ├── test_runner.c   # Unit tests
│   └── mock_libstorage.c
└── CMakeLists.txt
```

## License

Dual-licensed under [Apache 2.0](LICENSE-APACHEv2) and [MIT](LICENSE-MIT).
