#ifndef DDS_WRAPPER_PUBLISHER_H
#define DDS_WRAPPER_PUBLISHER_H

#include "Types.h"
#include "Exception.h"
#include "Logger.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <mutex>
#include <atomic>
#include <memory>
#include <chrono>

namespace dds_wrapper
{

// Forward declaration
template<typename T>
class Publisher;

// Separate Listener class
template<typename T>
class PublisherListener : public eprosima::fastdds::dds::DataWriterListener
{
public:
    explicit PublisherListener(std::weak_ptr<Publisher<T>> publisher)
        : publisher_(publisher)
    {
    }

    void on_publication_matched(
        eprosima::fastdds::dds::DataWriter* writer,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
    {
        auto pub = publisher_.lock();
        if (!pub)
        {
            return;  // Publisher has been destroyed
        }

        int change = info.current_count_change;
        pub->onSubscriberMatched(change);
    }

private:
    std::weak_ptr<Publisher<T>> publisher_;  // Weak reference to avoid circular dependency
};

// Publisher class
template<typename T>
class Publisher : public std::enable_shared_from_this<Publisher<T>>
{
public:
    // Factory method to create Publisher with DataWriter
    static std::shared_ptr<Publisher<T>> create(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastdds::dds::Topic* topic,
        eprosima::fastdds::dds::Publisher* publisher,
        const eprosima::fastdds::dds::DataWriterQos& qos
    )
    {
        // Create Publisher object first
        auto pub = std::shared_ptr<Publisher<T>>(new Publisher(participant, topic, publisher));

        // Now initialize it (create listener and writer)
        pub->initialize(qos);

        return pub;
    }

private:
    // Private constructor - use create() instead
    Publisher(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastdds::dds::Topic* topic,
        eprosima::fastdds::dds::Publisher* publisher
    )
        : participant_(participant)
        , topic_(topic)
        , publisher_(publisher)
        , writer_(nullptr)
        , listener_(nullptr)
        , matched_subscribers_(0)
        , total_published_(0)
        , failed_publishes_(0)
        , last_warn_time_(std::chrono::steady_clock::now())
    {
        if (topic_ != nullptr)
        {
            Logger::getInstance().info("Publisher created for topic: " + std::string(topic_->get_name()));
        }
    }

    // Initialize after construction (when shared_ptr exists)
    void initialize(const eprosima::fastdds::dds::DataWriterQos& qos)
    {
        // Validate pointers
        if (participant_ == nullptr || topic_ == nullptr || publisher_ == nullptr)
        {
            throw PublisherException(
                ErrorCode::PUBLISHER_CREATE_FAILED,
                "Invalid DDS entities (null pointer)"
            );
        }

        // Create listener
        listener_ = new PublisherListener<T>(this->weak_from_this());

        // Create DataWriter
        writer_ = publisher_->create_datawriter(topic_, qos, listener_);
        if (writer_ == nullptr)
        {
            delete listener_;
            listener_ = nullptr;
            throw PublisherException(ErrorCode::PUBLISHER_CREATE_FAILED, "Failed to create DataWriter");
        }

        Logger::getInstance().debug("DataWriter initialized for topic: " + std::string(topic_->get_name()));
    }

public:

    ~Publisher() noexcept
    {
        try
        {
            // Delete DataWriter first (it holds reference to listener)
            if (writer_ != nullptr && publisher_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
                writer_ = nullptr;
            }

            // Then delete listener
            if (listener_)
            {
                delete listener_;
                listener_ = nullptr;
            }

            Logger::getInstance().info("Publisher destroyed");
        }
        catch (const std::exception& e)
        {
            Logger::getInstance().error(std::string("Exception in Publisher destructor: ") + e.what());
        }
        catch (...)
        {
            Logger::getInstance().error("Unknown exception in Publisher destructor");
        }
    }

    // Disable copy, move, and assignment
    // Move is unsafe because listener holds weak_ptr to this object
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher(Publisher&&) = delete;
    Publisher& operator=(Publisher&&) = delete;

    bool publish(const T& data)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (writer_ == nullptr)
        {
            Logger::getInstance().error("DataWriter is null, cannot publish");
            failed_publishes_.fetch_add(1);
            return false;
        }

        // Check if there are matched subscribers (only warn periodically)
        if (matched_subscribers_.load() == 0)
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_warn_time_);
            if (elapsed.count() >= 5)
            {
                Logger::getInstance().warn("No subscribers matched for extended period");
                last_warn_time_ = now;
            }
        }

        try
        {
            if (writer_->write(const_cast<T*>(&data)))
            {
                total_published_.fetch_add(1);
                Logger::getInstance().debug("Message published successfully");
                return true;
            }
            else
            {
                failed_publishes_.fetch_add(1);
                Logger::getInstance().error("Failed to publish message");
                return false;
            }
        }
        catch (const std::exception& e)
        {
            failed_publishes_.fetch_add(1);
            Logger::getInstance().error(std::string("Exception during publish: ") + e.what());
            return false;
        }
    }

    bool waitForAcknowledgments(std::chrono::milliseconds timeout)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (writer_ == nullptr)
        {
            return false;
        }

        eprosima::fastrtps::Duration_t dds_timeout(
            timeout.count() / 1000,
            (timeout.count() % 1000) * 1000000
        );

        return writer_->wait_for_acknowledgments(dds_timeout);
    }

    bool isConnected() const noexcept
    {
        return matched_subscribers_.load() > 0;
    }

    int getMatchedSubscribers() const noexcept
    {
        return matched_subscribers_.load();
    }

    uint64_t getTotalPublished() const noexcept
    {
        return total_published_.load();
    }

    uint64_t getFailedPublishes() const noexcept
    {
        return failed_publishes_.load();
    }


    std::string getTopicName() const
    {
        if (topic_ != nullptr)
        {
            return topic_->get_name();
        }
        return "";
    }

    // Called by PublisherListener
    void onSubscriberMatched(int change)
    {
        matched_subscribers_.fetch_add(change);

        if (change > 0)
        {
            Logger::getInstance().info(
                "Publisher [" + getTopicName() + "] matched with " + std::to_string(change) +
                " new subscriber(s). Total: " + std::to_string(matched_subscribers_.load())
            );
        }
        else if (change < 0)
        {
            Logger::getInstance().warn(
                "Publisher [" + getTopicName() + "] lost " + std::to_string(-change) +
                " subscriber(s). Remaining: " + std::to_string(matched_subscribers_.load())
            );
        }
    }

private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::DataWriter* writer_;
    PublisherListener<T>* listener_;  // Owns the listener
    mutable std::mutex mutex_;
    std::atomic<int> matched_subscribers_;
    std::atomic<uint64_t> total_published_;
    std::atomic<uint64_t> failed_publishes_;
    std::chrono::steady_clock::time_point last_warn_time_;  // Protected by mutex_
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_PUBLISHER_H
