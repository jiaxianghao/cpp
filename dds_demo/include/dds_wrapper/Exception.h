#ifndef DDS_WRAPPER_EXCEPTION_H
#define DDS_WRAPPER_EXCEPTION_H

#include "Types.h"
#include <exception>
#include <string>

namespace dds_wrapper
{

// Base exception class
class DDSException : public std::exception
{
public:
    DDSException(ErrorCode code, const std::string& message)
        : code_(code), message_(message)
    {
    }

    virtual ~DDSException() noexcept = default;

    const char* what() const noexcept override
    {
        return message_.c_str();
    }

    ErrorCode getErrorCode() const
    {
        return code_;
    }

    std::string getMessage() const
    {
        return message_;
    }

protected:
    ErrorCode code_;
    std::string message_;
};

// Initialization exception
class InitializationException : public DDSException
{
public:
    InitializationException(const std::string& message)
        : DDSException(ErrorCode::INIT_FAILED, message)
    {
    }
};

// Configuration exception
class ConfigurationException : public DDSException
{
public:
    ConfigurationException(const std::string& message)
        : DDSException(ErrorCode::INVALID_CONFIG, message)
    {
    }
};

// Topic exception
class TopicException : public DDSException
{
public:
    TopicException(const std::string& message)
        : DDSException(ErrorCode::TOPIC_CREATE_FAILED, message)
    {
    }
};

// Publisher exception
class PublisherException : public DDSException
{
public:
    PublisherException(ErrorCode code, const std::string& message)
        : DDSException(code, message)
    {
    }
};

// Subscriber exception
class SubscriberException : public DDSException
{
public:
    SubscriberException(ErrorCode code, const std::string& message)
        : DDSException(code, message)
    {
    }
};

// Communication exception
class CommunicationException : public DDSException
{
public:
    CommunicationException(const std::string& message)
        : DDSException(ErrorCode::PUBLISH_FAILED, message)
    {
    }
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_EXCEPTION_H
