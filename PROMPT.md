# Context
I want you to implement the API described in `easylibstorage.h` in a file named `easylibstorage.c`. This is a simplified wrapper on top of `libstorage`.

# Dependencies
The headers for `libstorage` are located at `/home/giuliano/Work/Status/logos-storage-nim/library/libstorage.h`.
**[IMPORTANT]**: If you cannot read this file, stop and ask me to provide the function signatures.

The code for libstorage is located at `home/giuliano/Work/Status/logos-storage-nim/library`.

There are examples for how libstorage can be used in `/home/giuliano/Work/Status/logos-storage-nim/examples/c/storage.c`.

# Development Process (TDD)
You MUST follow a Test Driven Development approach. Since no test environment exists yet, your FIRST task is to:
1. Create a simple test runner (e.g., `tests/test_runner.c`) using basic `assert()` statements.
2. Update `CMakeLists.txt` to build this test runner.

Then, for *each* function in `easylibstorage.h`:
1. **Red**: Write a failing test case in `tests/test_runner.c`.
2. **Green**: Implement the function in `easylibstorage.c` until the test passes.
3. **Refactor**: Stop, think of ways to refactor and simplify the code, and do it.

**CRITICAL**: This refactoring step is VERY IMPORTANT. You MUST look for ways to simplify, deduplicate, and coalesce code here, WITHOUT OVERCOMPLICATING. **SIMPLICITY IS KEY AND YOUR GUIDING PRINCIPLE.**

# API Implementation Details
- **Configuration JSON.** The keys described in the node_config struct should be passed as kebab-case into the config JSON string. See an example
in the `storage.c` file. `bootstrap-node` must be passed as a string array.
- **Memory Management**: Clearly document who owns returned pointers (e.g., CIDs). Ensure no memory leaks.
- **Log Levels**: Map the `char *log_level` in the config to the internal `enum log_level`.
- **Download Return**: Note that `e_storage_download` currently returns `STORAGE_NODE` in the header. If this is a mistake, change it to return `int` (status code) and update the header.

# Console Application (`main.c`)
Wire the API into `main.c`.
- The current command loop only parses the first argument. You MUST refactor the command dispatch logic or the specific command functions to handle multiple arguments (e.g., `start` needs 3 args).
- Ensure `progress_print` is correctly passed as a callback to upload/download functions.

# Commands to Support
- `help`: Prints the help message.
- `start [API PORT] [DISC PORT] [BOOTSTRAP NODE]`: Creates and starts the node (all parameters mandatory).
- `stop`: Stops and destroys the node.
- `upload [LOCAL PATH]`: Uploads a local file; shows progress; prints CID.
- `download [CID] [LOCAL PATH]`: Downloads remote CID; shows progress.
