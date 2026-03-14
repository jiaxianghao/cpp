#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace tcp_network {

// Forward declarations
class IMessageBoundaryDetector;

// Message boundary configuration using polymorphic strategy pattern
struct MessageBoundaryConfig {
    std::unique_ptr<IMessageBoundaryDetector> detector;
    
    // Constructor with sensible defaults
    MessageBoundaryConfig();
    
    // Move constructor and assignment for unique_ptr
    MessageBoundaryConfig(MessageBoundaryConfig&& other) noexcept;
    MessageBoundaryConfig& operator=(MessageBoundaryConfig&& other) noexcept;
    
    // Copy constructor and assignment for unique_ptr
    MessageBoundaryConfig(const MessageBoundaryConfig&);
    MessageBoundaryConfig& operator=(const MessageBoundaryConfig&);
    
    // Factory methods for common configurations
    static MessageBoundaryConfig newline_delimited();
    static MessageBoundaryConfig length_prefixed(size_t length_size = 4, bool network_order = true);
    static MessageBoundaryConfig create_fixed_length(size_t length);
    static MessageBoundaryConfig tlv_format(size_t type_size = 1, size_t length_size = 4, bool network_order = true);
    static MessageBoundaryConfig magic_bytes(const std::vector<uint8_t>& start_magic, 
                                           const std::vector<uint8_t>& end_magic);
};

// Abstract base class for message boundary detection strategies
class IMessageBoundaryDetector {
public:
    virtual ~IMessageBoundaryDetector() = default;
    
    // Extract complete messages from buffer, removing processed data
    virtual std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) = 0;
    
    // Get strategy description for debugging/logging
    virtual std::string get_strategy_name() const = 0;
    
    // Clone method for copying detector instances
    virtual std::unique_ptr<IMessageBoundaryDetector> clone() const = 0;
};

// Delimiter-based message boundary detection
class DelimiterDetector : public IMessageBoundaryDetector {
public:
    explicit DelimiterDetector(const std::vector<uint8_t>& delimiter = {'\n'});
    
    std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IMessageBoundaryDetector> clone() const override;

private:
    std::vector<uint8_t> delimiter_;
};

// Fixed-length message boundary detection
class FixedLengthDetector : public IMessageBoundaryDetector {
public:
    explicit FixedLengthDetector(size_t length);
    
    std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IMessageBoundaryDetector> clone() const override;

private:
    size_t message_length_;
};

// Length-prefixed message boundary detection
class LengthPrefixedDetector : public IMessageBoundaryDetector {
public:
    LengthPrefixedDetector(size_t length_size, bool network_order = true);
    
    std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IMessageBoundaryDetector> clone() const override;

private:
    uint64_t extract_length_field(const uint8_t* data) const;
    size_t length_field_size_;
    bool network_byte_order_;
};

// TLV (Type-Length-Value) message boundary detection
class TlvDetector : public IMessageBoundaryDetector {
public:
    TlvDetector(size_t type_size, size_t length_size, bool network_order = true);
    
    std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IMessageBoundaryDetector> clone() const override;

private:
    uint64_t extract_length_field(const uint8_t* data) const;
    size_t type_field_size_;
    size_t length_field_size_;
    bool network_byte_order_;
};

// Magic bytes-based message boundary detection
class MagicBytesDetector : public IMessageBoundaryDetector {
public:
    MagicBytesDetector(const std::vector<uint8_t>& start_magic, 
                      const std::vector<uint8_t>& end_magic);
    
    std::vector<std::vector<uint8_t>> extract_messages(std::vector<uint8_t>& buffer) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IMessageBoundaryDetector> clone() const override;

private:
    std::vector<uint8_t> start_magic_;
    std::vector<uint8_t> end_magic_;
};

} // namespace tcp_network