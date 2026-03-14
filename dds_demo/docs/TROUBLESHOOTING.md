# Troubleshooting Guide

Common issues and solutions for FastDDS Wrapper.

## Build Issues

### CMake Cannot Find FastDDS

**Error:**
```
CMake Error: Could not find a package configuration file provided by "fastrtps"
```

**Solutions:**

1. Install FastDDS:
```bash
# Ubuntu/Debian
sudo apt install libfastrtps-dev libfastcdr-dev

# From source
git clone https://github.com/eProsima/Fast-DDS.git
cd Fast-DDS
mkdir build && cd build
cmake ..
make
sudo make install
```

2. Set CMake prefix path:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/fastdds/install ..
```

### fastddsgen Not Found

**Error:**
```
fastddsgen not found. IDL files will not be compiled automatically.
```

**Solutions:**

1. Install FastDDS-Gen:
```bash
# Download from GitHub
git clone --recursive https://github.com/eProsima/Fast-DDS-Gen.git
cd Fast-DDS-Gen
./gradlew assemble
sudo mkdir -p /usr/local/share/fastddsgen
sudo cp -r share/fastddsgen /usr/local/share/
sudo cp scripts/fastddsgen /usr/local/bin/
sudo chmod +x /usr/local/bin/fastddsgen
```

2. Check PATH:
```bash
which fastddsgen
export PATH=$PATH:/path/to/fastddsgen
```

### IDL Compilation Errors

**Error:**
```
Error generating C++ code from IDL
```

**Solutions:**

1. Validate IDL syntax:
```bash
cd tools
./validate_idl.sh ../idl
```

2. Check IDL file manually:
```bash
fastddsgen -help
fastddsgen -example x64Linux2.6gcc YourFile.idl
```

3. Common IDL syntax errors:
```idl
// Wrong: Missing semicolon
struct Message
{
    string content
}

// Correct:
struct Message
{
    string content;
};
```

## Runtime Issues

### No Messages Received

**Symptoms:** Publisher publishes, subscriber callback never called.

**Checklist:**

1. Same domain ID?
```cpp
// Both must use same domain
config.domain_id = 0;
```

2. Same topic name?
```cpp
// Must match exactly
pub = createPublisher<T>("MyTopic");
sub = createSubscriber<T>("MyTopic", callback);
```

3. Discovery time?
```cpp
// Add delay for discovery
std::this_thread::sleep_for(std::chrono::seconds(1));
pub->publish(msg);
```

4. Firewall blocking?
```bash
# Allow UDP ports 7400-7700
sudo ufw allow 7400:7700/udp
```

5. Check logs:
```cpp
config.log_level = LogLevel::DEBUG;
```

### Messages Dropped

**Symptoms:** Some messages not received.

**Solutions:**

1. Use RELIABLE QoS:
```cpp
config.reliability = ReliabilityKind::RELIABLE;
```

2. Increase history:
```cpp
config.history_depth = 100;
```

3. Check subscriber is running:
```cpp
if (sub->isRunning())
{
    // Subscriber active
}
```

### Late Joiner Misses Messages

**Symptoms:** Subscriber created after publishing misses messages.

**Solution:** Use TRANSIENT_LOCAL durability:

```cpp
config.durability = DurabilityKind::TRANSIENT_LOCAL;
config.history_depth = 10;  // Keep last 10 messages
```

### Memory Leaks

**Symptoms:** Memory usage grows over time.

**Checklist:**

1. Call shutdown:
```cpp
manager.shutdown();  // Always cleanup
```

2. Shared pointers cleanup:
```cpp
{
    auto pub = manager.createPublisher<T>("Topic");
    // pub auto-destroyed when out of scope
}
```

3. Check callback exceptions:
```cpp
auto sub = manager.createSubscriber<T>("Topic",
    [](const T& msg)
    {
        try
        {
            // Handle message
        }
        catch (...)
        {
            // Handle errors
        }
    });
```

## Performance Issues

### High CPU Usage

**Causes and Solutions:**

1. Too frequent heartbeats:
```cpp
config.heartbeat_interval_ms = 5000;  // Increase interval
```

2. Debug logging:
```cpp
config.log_level = LogLevel::WARN;  // Reduce logging
```

3. Many small messages:
```cpp
// Batch messages instead
std::vector<Message> batch;
// ... collect messages ...
for (auto& msg : batch)
{
    pub->publish(msg);
}
```

### High Latency

**Solutions:**

1. Use BEST_EFFORT:
```cpp
config.reliability = ReliabilityKind::BEST_EFFORT;
```

2. Reduce history:
```cpp
config.history_depth = 1;
```

3. Same machine optimization:
```cpp
// FastDDS uses shared memory automatically
// Ensure not disabled in XML config
```

### High Memory Usage

**Solutions:**

1. Reduce history:
```cpp
config.history_depth = 10;  // Smaller buffer
```

2. Use VOLATILE durability:
```cpp
config.durability = DurabilityKind::VOLATILE;
```

## Error Messages

### "DDSManager not initialized"

```cpp
// Error
auto pub = manager.createPublisher<T>("Topic");  // Throws!

// Fix
manager.initialize();
auto pub = manager.createPublisher<T>("Topic");  // OK
```

### "Topic creation failed"

**Causes:**
- Invalid topic name
- Type registration failed
- Participant not created

**Debug:**
```cpp
try
{
    auto pub = manager.createPublisher<T>("Topic");
}
catch (const TopicException& e)
{
    std::cerr << "Topic error: " << e.getMessage() << std::endl;
}
```

### "Failed to create DataWriter"

**Solutions:**

1. Check QoS compatibility
2. Verify participant is valid
3. Check resource limits

### Compilation Errors

#### "Type has no member 'PubSubType'"

**Cause:** IDL not properly generated.

**Fix:**
```bash
cd build
rm -rf *
cmake ..
make
```

#### "undefined reference to XxxPubSubType"

**Cause:** Not linking idl_types library.

**Fix in CMakeLists.txt:**
```cmake
target_link_libraries(your_app dds_wrapper idl_types)
```

## Debugging Tips

### Enable Verbose Logging

```cpp
DDSConfig config;
config.log_level = LogLevel::DEBUG;
config.log_file_output = true;
config.log_dir = "debug_logs";
manager.initialize(config);
```

### Check Connection Status

```cpp
if (manager.getStatus() == ConnectionStatus::CONNECTED)
{
    std::cout << "Connected OK" << std::endl;
}
else
{
    std::cout << "Connection problem" << std::endl;
}
```

### Verify Publisher/Subscriber

```cpp
if (pub->isConnected())
{
    std::cout << "Publisher has subscribers" << std::endl;
}
```

### Use Monitoring Example

```bash
./build/bin/examples/monitoring
```

### FastDDS Tools

Use FastDDS discovery server:
```bash
# Terminal 1
fast-discovery-server -i 0

# Terminal 2 (in your app config)
export ROS_DISCOVERY_SERVER=127.0.0.1:11811
```

## Platform-Specific Issues

### Windows

**Issue:** DLL not found

**Fix:**
```bash
# Add FastDDS to PATH
set PATH=%PATH%;C:\Program Files\fastrtps\bin
```

### Linux

**Issue:** Permission denied on shared memory

**Fix:**
```bash
# Add user to group
sudo usermod -aG ipc $USER
```

**Issue:** Cannot bind to port

**Fix:**
```bash
# Check if port in use
netstat -tulpn | grep 7400

# Kill conflicting process or change domain
config.domain_id = 5;
```

### macOS

**Issue:** Firewall blocking

**Fix:**
```bash
# Allow incoming connections
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --add /path/to/your/app
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --unblockapp /path/to/your/app
```

## Getting More Help

### Check Logs

```bash
cat logs/dds_wrapper.log
```

### Increase Verbosity

```cpp
Logger::getInstance().setLogLevel(LogLevel::DEBUG);
```

### FastDDS Environment Variables

```bash
export FASTDDS_VERBOSITY=debug
export FASTDDS_LOG_FILE=fastdds_debug.log
```

### Report Issues

When reporting issues, include:

1. Operating system and version
2. FastDDS version: `dpkg -l | grep fastrtps`
3. Compiler version: `g++ --version`
4. CMake version: `cmake --version`
5. Log output with DEBUG level
6. Minimal reproducible example

## Quick Diagnostics

Run this to check your setup:

```bash
# Check installations
which fastddsgen
ldconfig -p | grep fastrtps

# Check example
cd build
./bin/examples/basic_pubsub

# Check types
./bin/tools/list_types
```

## Common Fixes Summary

| Problem | Quick Fix |
|---------|-----------|
| No messages | Add 1s delay after creation |
| Dropped messages | Use RELIABLE QoS |
| Late joiner | Use TRANSIENT_LOCAL |
| High CPU | Reduce heartbeat frequency |
| High latency | Use BEST_EFFORT |
| Build fails | Check FastDDS installation |
| IDL errors | Run validate_idl.sh |

## Still Stuck?

1. Review the [Quick Start](QUICK_START.md)
2. Check [examples/](../examples/)
3. Read [FastDDS documentation](https://fast-dds.docs.eprosima.com/)
4. Open an issue on GitHub
