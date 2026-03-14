#ifndef DDS_WRAPPER_TYPES_H
#define DDS_WRAPPER_TYPES_H

#include <string>
#include <memory>
#include <functional>

namespace dds_wrapper
{

// Forward declarations
template<typename T>
class Publisher;

template<typename T>
class Subscriber;

// Callback type for subscribers
template<typename T>
using MessageCallback = std::function<void(const T&)>;

// Type traits for mapping data types to their PubSubTypes
// This template must be specialized for each IDL-generated type
template<typename T>
struct DDS_TypeTraits;

// Example specialization (users should add these for their types):
// template<>
// struct DDS_TypeTraits<CommandMessage>
// {
//     using PubSubType = CommandMessagePubSubType;
// };

// Connection status
enum class ConnectionStatus
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

// QoS reliability types
enum class ReliabilityKind
{
    BEST_EFFORT,
    RELIABLE
};

// QoS durability types
enum class DurabilityKind
{
    VOLATILE,
    TRANSIENT_LOCAL,
    TRANSIENT,
    PERSISTENT
};

// Log levels
enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

// Error codes
enum class ErrorCode
{
    SUCCESS = 0,
    INIT_FAILED,
    ALREADY_INITIALIZED,
    NOT_INITIALIZED,
    INVALID_CONFIG,
    TOPIC_CREATE_FAILED,
    PUBLISHER_CREATE_FAILED,
    SUBSCRIBER_CREATE_FAILED,
    PARTICIPANT_CREATE_FAILED,
    PUBLISH_FAILED,
    CONNECTION_LOST,
    TIMEOUT,
    UNKNOWN_ERROR
};

// Convert error code to string
inline std::string errorCodeToString(ErrorCode code)
{
    switch(code)
    {
        case ErrorCode::SUCCESS: return "Success";
        case ErrorCode::INIT_FAILED: return "Initialization failed";
        case ErrorCode::ALREADY_INITIALIZED: return "Already initialized";
        case ErrorCode::NOT_INITIALIZED: return "Not initialized";
        case ErrorCode::INVALID_CONFIG: return "Invalid configuration";
        case ErrorCode::TOPIC_CREATE_FAILED: return "Topic creation failed";
        case ErrorCode::PUBLISHER_CREATE_FAILED: return "Publisher creation failed";
        case ErrorCode::SUBSCRIBER_CREATE_FAILED: return "Subscriber creation failed";
        case ErrorCode::PARTICIPANT_CREATE_FAILED: return "Participant creation failed";
        case ErrorCode::PUBLISH_FAILED: return "Publish failed";
        case ErrorCode::CONNECTION_LOST: return "Connection lost";
        case ErrorCode::TIMEOUT: return "Operation timeout";
        case ErrorCode::UNKNOWN_ERROR: return "Unknown error";
        default: return "Undefined error";
    }
}

// Macro to simplify type traits definition
#define DDS_REGISTER_TYPE(DataType, PubSubTypeName) \
    namespace dds_wrapper { \
    template<> \
    struct DDS_TypeTraits<DataType> \
    { \
        using PubSubType = PubSubTypeName; \
    }; \
    }

} // namespace dds_wrapper

#endif // DDS_WRAPPER_TYPES_H
