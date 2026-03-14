#include <algorithm>
#include <arpa/inet.h>
#include <tcp_network/message_boundary_detector.h>

using namespace tcp_network;

// ========== Message Boundary Detection Polymorphic Implementation ==========

// MessageBoundaryConfig implementation
MessageBoundaryConfig::MessageBoundaryConfig() {
    // Default to newline-delimited
    detector = std::make_unique<DelimiterDetector>();
}

MessageBoundaryConfig::MessageBoundaryConfig(MessageBoundaryConfig&& other) noexcept
    : detector(std::move(other.detector)) {
}

MessageBoundaryConfig& MessageBoundaryConfig::operator=(MessageBoundaryConfig&& other) noexcept {
    if (this != &other) {
        detector = std::move(other.detector);
    }
    return *this;
}

MessageBoundaryConfig::MessageBoundaryConfig(const MessageBoundaryConfig& other)
{
    detector = other.detector->clone();
}

MessageBoundaryConfig& MessageBoundaryConfig::operator=(const MessageBoundaryConfig& other)
{
    if (this != &other) {
        detector = other.detector->clone();
    }
    return *this;
}

MessageBoundaryConfig MessageBoundaryConfig::newline_delimited() {
    MessageBoundaryConfig config;
    config.detector = std::make_unique<DelimiterDetector>(std::vector<uint8_t>{'\n'});
    return config;
}

MessageBoundaryConfig MessageBoundaryConfig::length_prefixed(size_t length_size, bool network_order) {
    MessageBoundaryConfig config;
    config.detector = std::make_unique<LengthPrefixedDetector>(length_size, network_order);
    return config;
}

MessageBoundaryConfig MessageBoundaryConfig::create_fixed_length(size_t length) {
    MessageBoundaryConfig config;
    config.detector = std::make_unique<FixedLengthDetector>(length);
    return config;
}

MessageBoundaryConfig MessageBoundaryConfig::tlv_format(size_t type_size, size_t length_size, bool network_order) {
    MessageBoundaryConfig config;
    config.detector = std::make_unique<TlvDetector>(type_size, length_size, network_order);
    return config;
}

MessageBoundaryConfig MessageBoundaryConfig::magic_bytes(const std::vector<uint8_t>& start_magic, 
                                                       const std::vector<uint8_t>& end_magic) {
    MessageBoundaryConfig config;
    config.detector = std::make_unique<MagicBytesDetector>(start_magic, end_magic);
    return config;
}

// DelimiterDetector implementation
DelimiterDetector::DelimiterDetector(const std::vector<uint8_t>& delimiter) 
    : delimiter_(delimiter) {
}

std::vector<std::vector<uint8_t>> DelimiterDetector::extract_messages(std::vector<uint8_t>& buffer) {
    std::vector<std::vector<uint8_t>> messages;
    
    if (delimiter_.empty()) {
        return messages;
    }
    
    auto search_start = buffer.begin();
    while (search_start != buffer.end()) {
        auto pos = std::search(search_start, buffer.end(), delimiter_.begin(), delimiter_.end());
        if (pos != buffer.end()) {
            // Extract message including delimiter
            std::vector<uint8_t> message(search_start, pos + delimiter_.size());
            messages.push_back(std::move(message));
            search_start = pos + delimiter_.size();
        } else {
            break;
        }
    }
    
    // Remove processed data from buffer
    if (!messages.empty()) {
        auto bytes_processed = search_start - buffer.begin();
        buffer.erase(buffer.begin(), buffer.begin() + bytes_processed);
    }
    
    return messages;
}

std::string DelimiterDetector::get_strategy_name() const {
    return "DelimiterDetector";
}

std::unique_ptr<IMessageBoundaryDetector> DelimiterDetector::clone() const {
    return std::make_unique<DelimiterDetector>(delimiter_);
}

// FixedLengthDetector implementation
FixedLengthDetector::FixedLengthDetector(size_t length)
    : message_length_(length) {
}

std::vector<std::vector<uint8_t>> FixedLengthDetector::extract_messages(std::vector<uint8_t>& buffer) {
    std::vector<std::vector<uint8_t>> messages;
    
    while (buffer.size() >= message_length_) {
        std::vector<uint8_t> message(buffer.begin(), buffer.begin() + message_length_);
        messages.push_back(std::move(message));
        buffer.erase(buffer.begin(), buffer.begin() + message_length_);
    }
    
    return messages;
}

std::string FixedLengthDetector::get_strategy_name() const {
    return "FixedLengthDetector";
}

std::unique_ptr<IMessageBoundaryDetector> FixedLengthDetector::clone() const {
    return std::make_unique<FixedLengthDetector>(message_length_);
}

// LengthPrefixedDetector implementation
LengthPrefixedDetector::LengthPrefixedDetector(size_t length_size, bool network_order)
    : length_field_size_(length_size), network_byte_order_(network_order) {
}

uint64_t LengthPrefixedDetector::extract_length_field(const uint8_t* data) const {
    if (network_byte_order_) {
        switch (length_field_size_) {
            case 1: return data[0];
            case 2: return ntohs(*reinterpret_cast<const uint16_t*>(data));
            case 4: return ntohl(*reinterpret_cast<const uint32_t*>(data));
            case 8: {
                // Manual big-endian conversion for 64-bit
                uint64_t value = 0;
                for (int i = 0; i < 8; ++i) {
                    value = (value << 8) | data[i];
                }
                return value;
            }
            default: return 0;
        }
    } else {
        switch (length_field_size_) {
            case 1: return data[0];
            case 2: return *reinterpret_cast<const uint16_t*>(data);
            case 4: return *reinterpret_cast<const uint32_t*>(data);
            case 8: return *reinterpret_cast<const uint64_t*>(data);
            default: return 0;
        }
    }
}

std::vector<std::vector<uint8_t>> LengthPrefixedDetector::extract_messages(std::vector<uint8_t>& buffer) {
    std::vector<std::vector<uint8_t>> messages;
    
    while (buffer.size() >= length_field_size_) {
        uint64_t message_length = extract_length_field(buffer.data());
        
        // Check if we have the complete message
        if (buffer.size() >= length_field_size_ + message_length) {
            // Extract the complete message (length + data)
            std::vector<uint8_t> message(buffer.begin(), buffer.begin() + length_field_size_ + message_length);
            messages.push_back(std::move(message));
            buffer.erase(buffer.begin(), buffer.begin() + length_field_size_ + message_length);
        } else {
            break; // Wait for more data
        }
    }
    
    return messages;
}

std::string LengthPrefixedDetector::get_strategy_name() const {
    return "LengthPrefixedDetector";
}

std::unique_ptr<IMessageBoundaryDetector> LengthPrefixedDetector::clone() const {
    return std::make_unique<LengthPrefixedDetector>(length_field_size_, network_byte_order_);
}

// TlvDetector implementation
TlvDetector::TlvDetector(size_t type_size, size_t length_size, bool network_order)
    : type_field_size_(type_size), length_field_size_(length_size), network_byte_order_(network_order) {
}

uint64_t TlvDetector::extract_length_field(const uint8_t* data) const {
    if (network_byte_order_) {
        switch (length_field_size_) {
            case 1: return data[0];
            case 2: return ntohs(*reinterpret_cast<const uint16_t*>(data));
            case 4: return ntohl(*reinterpret_cast<const uint32_t*>(data));
            case 8: {
                uint64_t value = 0;
                for (int i = 0; i < 8; ++i) {
                    value = (value << 8) | data[i];
                }
                return value;
            }
            default: return 0;
        }
    } else {
        switch (length_field_size_) {
            case 1: return data[0];
            case 2: return *reinterpret_cast<const uint16_t*>(data);
            case 4: return *reinterpret_cast<const uint32_t*>(data);
            case 8: return *reinterpret_cast<const uint64_t*>(data);
            default: return 0;
        }
    }
}

std::vector<std::vector<uint8_t>> TlvDetector::extract_messages(std::vector<uint8_t>& buffer) {
    std::vector<std::vector<uint8_t>> messages;
    size_t header_size = type_field_size_ + length_field_size_;
    
    while (buffer.size() >= header_size) {
        // Extract length field (skip type field)
        const uint8_t* length_ptr = buffer.data() + type_field_size_;
        uint64_t value_length = extract_length_field(length_ptr);
        
        // Check if we have the complete TLV message
        if (buffer.size() >= header_size + value_length) {
            // Extract the complete TLV message
            std::vector<uint8_t> message(buffer.begin(), buffer.begin() + header_size + value_length);
            messages.push_back(std::move(message));
            buffer.erase(buffer.begin(), buffer.begin() + header_size + value_length);
        } else {
            break; // Wait for more data
        }
    }
    
    return messages;
}

std::string TlvDetector::get_strategy_name() const {
    return "TlvDetector";
}

std::unique_ptr<IMessageBoundaryDetector> TlvDetector::clone() const {
    return std::make_unique<TlvDetector>(type_field_size_, length_field_size_, network_byte_order_);
}

// MagicBytesDetector implementation
MagicBytesDetector::MagicBytesDetector(const std::vector<uint8_t>& start_magic, 
                                     const std::vector<uint8_t>& end_magic)
    : start_magic_(start_magic), end_magic_(end_magic) {
}

std::vector<std::vector<uint8_t>> MagicBytesDetector::extract_messages(std::vector<uint8_t>& buffer) {
    std::vector<std::vector<uint8_t>> messages;
    
    if (start_magic_.empty() || end_magic_.empty()) {
        return messages;
    }
    
    auto search_start = buffer.begin();
    while (search_start != buffer.end()) {
        // Find start magic
        auto start_pos = std::search(search_start, buffer.end(), start_magic_.begin(), start_magic_.end());
        if (start_pos == buffer.end()) {
            break; // No more start magic found
        }
        
        // Find end magic after start magic
        auto end_search_start = start_pos + start_magic_.size();
        auto end_pos = std::search(end_search_start, buffer.end(), end_magic_.begin(), end_magic_.end());
        if (end_pos == buffer.end()) {
            break; // No end magic found, wait for more data
        }
        
        // Extract complete message (start magic + content + end magic)
        std::vector<uint8_t> message(start_pos, end_pos + end_magic_.size());
        messages.push_back(std::move(message));
        search_start = end_pos + end_magic_.size();
    }
    
    // Remove processed data from buffer
    if (!messages.empty()) {
        auto bytes_processed = search_start - buffer.begin();
        buffer.erase(buffer.begin(), buffer.begin() + bytes_processed);
    }
    
    return messages;
}

std::string MagicBytesDetector::get_strategy_name() const {
    return "MagicBytesDetector";
}

std::unique_ptr<IMessageBoundaryDetector> MagicBytesDetector::clone() const {
    return std::make_unique<MagicBytesDetector>(start_magic_, end_magic_);
}