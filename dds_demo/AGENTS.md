# FastDDS Wrapper - Agent Guide

This file contains project-specific context for AI coding agents.

## Project Overview

FastDDS Wrapper is a C++17 library that simplifies DDS (Data Distribution Service) communication by wrapping the eProsima FastDDS API. It reduces the conceptual surface from ~8 native DDS concepts (DomainParticipant, Topic, Publisher, Subscriber, DataWriter, DataReader, QoS, Listeners) to 3 (`DDSManager`, `Publisher`, `Subscriber`).

Key design goals:
- **Zero-intrusion extension**: New message types are added by dropping an IDL file into `idl/` and rebuilding. No wrapper code changes are required.
- **Type safety**: Template-based compile-time checking prevents publishing wrong types to a topic.
- **Production-ready**: RAII resource management, thread-safe operations, exception safety, and composition-based listeners to avoid circular references.

## Technology Stack

- **Language**: C++17
- **Build system**: CMake 3.16+
- **Core dependency**: eProsima FastDDS 2.10+ (`fastdds`, `fastcdr`)
- **Code generator**: FastDDS-Gen (`fastddsgen`) for IDL -> C++ generation
- **Optional tooling**: Python 3 (IDL generator script)

## Build Commands

```bash
# Standard build
mkdir build && cd build`
cmake ..
make -j$(nproc)

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Custom FastDDS path
cmake -DCMAKE_PREFIX_PATH=/path/to/fastdds ..

# Install system-wide
sudo make install
```

### Key CMake Targets

| Target | Description |
|--------|-------------|
| `dds_wrapper` | Static library (core wrapper) |
| `idl_types` | Static library (generated from `.idl` files) |
| `basic_pubsub` | Basic pub/sub example |
| `multiple_topics` | Multi-topic example |
| `reliable_communication` | Reliable QoS example |
| `custom_qos` | Custom QoS example |
| `add_new_type_tutorial` | New type tutorial |
| `service_demo` | ROS-style request/response demo |
| `list_types` | Tool to list registered IDL types |

## Project Structure

```
dds_demo/
├── CMakeLists.txt              # Root build config
├── include/dds_wrapper/        # Public headers (templates + declarations)
│   ├── DDSManager.h            # Central factory & lifecycle manager
│   ├── Publisher.h             # Template pub class + listener
│   ├── Subscriber.h            # Template sub class + listener
│   ├── ServiceClient.h         # ROS-style service client template
│   ├── ServiceServer.h         # ROS-style service server template
│   ├── Types.h                 # Type traits, enums, callback typedefs
│   ├── Config.h                # DDSConfig struct + ConfigManager
│   ├── Exception.h             # Exception hierarchy
│   └── Logger.h                # Singleton thread-safe logger
├── src/                        # Implementation files
│   ├── DDSManager.cpp          # Participant/pub/sub lifecycle, QoS mapping
│   ├── ConfigManager.cpp       # JSON config parsing (simplified)
│   └── Logger.cpp              # File + console logging
├── idl/                        # IDL definitions + CMake generation logic
│   ├── CMakeLists.txt          # fastddsgen invocation + IDLTypes.h generation
│   ├── CommonMessage.idl
│   ├── SensorData.idl
│   ├── CommandMessage.idl
│   └── AddTwoInts.idl          # Used by service_demo
├── examples/                   # Example programs
├── tools/                      # Helper tools
├── config/
│   └── default_config.json     # Default domain, QoS, logging settings
└── docs/                       # User documentation (English)
```

## Architecture Details

### Composition Over Inheritance

`Publisher` and `Subscriber` do **not** inherit from FastDDS listeners. Instead they contain separate `PublisherListener<T>` / `SubscriberListener<T>` objects. The listeners hold a `std::weak_ptr` to the parent to avoid circular references with FastDDS internals. This is a deliberate architectural choice documented in `ARCHITECTURE_IMPROVEMENT.md`.

### Factory Pattern for `shared_from_this`

Because listeners need `weak_from_this()`, constructors are private and instances are created through static `create()` factory methods:

```cpp
auto pub = Publisher<T>::create(participant, topic, publisher, qos);
auto sub = Subscriber<T>::create(participant, topic, subscriber, qos, callback);
```

### Type Registration

The `DDS_REGISTER_TYPE(DataType, PubSubTypeName)` macro in `Types.h` specializes `DDS_TypeTraits<T>` so that `DDSManager` can instantiate the correct FastDDS `TypeSupport` for any template type.

The `idl/CMakeLists.txt` automatically generates `IDLTypes.h` which includes all generated headers and invokes `DDS_REGISTER_TYPE` for every `.idl` file found in `idl/`.

### Service Layer

`ServiceServer` and `ServiceClient` implement ROS-style request/response on top of raw DDS pub/sub:
- Request topic: `<service_name>/request`
- Response topic: `<service_name>/response`
- Correlation is done via `request_id` and `client_id` fields that IDL types must expose.

## Code Style Guidelines

- **Braces**: Opening brace on its own line (Allman style).
- **Indentation**: 4 spaces (inferred from source).
- **Comments**: English only.
- **No trailing whitespace**; no invisible characters at line ends.
- **Copy/Move semantics**: Most classes disable copy and move (`= delete`). Destructors are marked `noexcept`.
- **Naming**:
  - Classes: `PascalCase` (e.g., `DDSManager`, `PublisherListener`)
  - Methods: `camelCase` (e.g., `createPublisher`, `isInitialized`)
  - Private members: trailing underscore (e.g., `participant_`, `mutex_`)
  - Macros / constants: `SCREAMING_SNAKE_CASE` (e.g., `DDS_REGISTER_TYPE`)
- **Thread safety**: Use `std::mutex` for complex state, `std::atomic` for simple counters. Separate mutexes for unrelated concerns (e.g., `callback_mutex_` distinct from reader/writer mutexes) to avoid deadlocks.
- **Exception handling**: Catch inside destructors and `noexcept` functions; log errors instead of throwing.
- **Raw pointers**: FastDDS entities (`DomainParticipant*`, `DataWriter*`, etc.) are managed as raw pointers because FastDDS API returns/accepts raw pointers. The wrapper owns them and deletes them in reverse creation order.

## Adding a New Message Type

1. Create `idl/YourType.idl`:
   ```idl
   struct YourType
   {
       string id;
       double value;
   };
   ```
2. Rebuild:
   ```bash
   cd build && cmake .. && make
   ```
3. Include `IDLTypes.h` and use:
   ```cpp
   auto pub = manager.createPublisher<YourType>("Topic");
   ```

No changes to wrapper source code are needed.

## Testing Instructions

There is **no formal unit test suite** in the project yet. Validation is done via the example executables:

```bash
# Basic functionality
cd build
./bin/examples/basic_pubsub

# Multiple topics
./bin/examples/multiple_topics

# Reliable QoS
./bin/examples/reliable_communication

# Custom QoS
./bin/examples/custom_qos

# Service request/response
./bin/examples/service_demo

# List registered types
./bin/tools/list_types
```

To check for memory leaks:
```bash
valgrind --leak-check=full ./bin/examples/basic_pubsub
```

### Suggested Test Additions

If you add tests, Google Test is the recommended framework (mentioned in docs but not integrated). Good starting tests:
- `DDSManager::initialize()` success/failure paths
- `Publisher::publish()` with and without matched subscribers
- `Subscriber` callback invocation and exception handling
- `ServiceClient::call()` timeout vs. successful response

## Configuration

`config/default_config.json` controls:

| Key | Default | Description |
|-----|---------|-------------|
| `domain_id` | `0` | DDS domain ID |
| `participant_name` | `DDSWrapper_Participant` | Participant QoS name |
| `qos.reliability` | `reliable` | `reliable` or `best_effort` |
| `qos.durability` | `transient_local` | `volatile`, `transient_local`, `transient`, `persistent` |
| `qos.history_depth` | `10` | Keep-last history depth |
| `logging.level` | `INFO` | `DEBUG`, `INFO`, `WARN`, `ERROR` |
| `logging.file_output` | `true` | Write to `logs/dds_wrapper.log` |
| `logging.console_output` | `true` | Mirror to stdout/stderr |

**Note**: `ConfigManager::loadFromJSON()` currently implements only basic string-parsing for `domain_id`. A full JSON parser (e.g., nlohmann/json) is listed as a future improvement. `loadFromXML()` is a stub.

## Security Considerations

- The wrapper does **not** currently implement DDS-Security. Authentication, encryption, and access control would need to be added at the FastDDS participant QoS level.
- `ConfigManager` does not validate file paths; ensure config files are read from trusted locations.
- The `Subscriber` callback is invoked from the FastDDS listener thread. User-provided callbacks should be thread-safe and must not throw exceptions (the wrapper catches them, but uncaught exceptions in foreign threads are dangerous).
- `ServiceClient` generates request IDs with `std::mt19937_64`. This is sufficient for correlation but not cryptographically secure.

## Known Limitations & Technical Debt

1. **JSON parsing**: Simplified manual parser. Should be replaced with a proper library.
2. **XML config**: Stub implementation only.
3. **ConnectionMonitor**: Mentioned in design docs but only a simplified concept exists.
4. **No unit tests**: Only integration tests via examples.
5. **No CI/CD**: No automated build/test pipeline configured.
6. **Platform assumptions**: Linux paths are primary; Windows/macOS compatibility is present but less tested.

## Common Agent Tasks

- **Fixing build issues**: Ensure `fastddsgen` is on `PATH`. If missing, the build still succeeds but `idl_types` will be an empty interface library.
- **Adding a new example**: Add `.cpp` to `examples/`, add `add_executable(...)` + `target_link_libraries(...)` in `examples/CMakeLists.txt`.
- **Adding a new IDL type**: Drop `.idl` into `idl/` and rebuild. CMake regenerates `IDLTypes.h` automatically.
- **Modifying QoS behavior**: Edit `DDSManager::createDataWriterQos()` and `DDSManager::createDataReaderQos()` in `src/DDSManager.cpp`.
- **Modifying logging**: `Logger` is a singleton with `getInstance()`. It is thread-safe via internal mutex.

## File Inventory (Core)

| File | Purpose |
|------|---------|
| `include/dds_wrapper/DDSManager.h` | Factory methods, template implementations |
| `include/dds_wrapper/Publisher.h` | `Publisher<T>`, `PublisherListener<T>` |
| `include/dds_wrapper/Subscriber.h` | `Subscriber<T>`, `SubscriberListener<T>` |
| `include/dds_wrapper/ServiceClient.h` | Synchronous + async service client |
| `include/dds_wrapper/ServiceServer.h` | Service server with handler wiring |
| `include/dds_wrapper/Types.h` | `DDS_TypeTraits`, callbacks, enums, macros |
| `include/dds_wrapper/Exception.h` | `DDSException` hierarchy |
| `include/dds_wrapper/Config.h` | `DDSConfig`, `ConfigManager` |
| `include/dds_wrapper/Logger.h` | Singleton logger |
| `src/DDSManager.cpp` | Participant lifecycle, topic cache, QoS mapping |
| `src/ConfigManager.cpp` | Config load/save |
| `src/Logger.cpp` | Log file/console output |
| `idl/CMakeLists.txt` | IDL compilation + `IDLTypes.h` generation |
