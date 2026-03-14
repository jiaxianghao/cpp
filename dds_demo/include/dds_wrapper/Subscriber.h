#ifndef DDS_WRAPPER_SUBSCRIBER_H
#define DDS_WRAPPER_SUBSCRIBER_H

#include "Types.h"
#include "Exception.h"
#include "Logger.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>

namespace dds_wrapper
{

// Forward declaration
template<typename T>
class Subscriber;

// Separate Listener class
template<typename T>
class SubscriberListener : public eprosima::fastdds::dds::DataReaderListener
{
public:
    explicit SubscriberListener(std::weak_ptr<Subscriber<T>> subscriber)
        : subscriber_(subscriber)
    {
    }

    void on_data_available(eprosima::fastdds::dds::DataReader* reader) override
    {
        auto sub = subscriber_.lock();
        if (!sub)
        {
            return;  // Subscriber已销毁
        }

        sub->onDataAvailable(reader);
    }

    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader* reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
    {
        auto sub = subscriber_.lock();
        if (!sub)
        {
            return;  // Subscriber已销毁
        }

        int change = info.current_count_change;
        sub->onPublisherMatched(change);
    }

private:
    std::weak_ptr<Subscriber<T>> subscriber_;  // 弱引用，避免循环引用
};

// Subscriber class
template<typename T>
class Subscriber : public std::enable_shared_from_this<Subscriber<T>>
{
public:
    // Factory method to create Subscriber with DataReader
    static std::shared_ptr<Subscriber<T>> create(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastdds::dds::Topic* topic,
        eprosima::fastdds::dds::Subscriber* subscriber,
        const eprosima::fastdds::dds::DataReaderQos& qos,
        MessageCallback<T> callback
    )
    {
        // Create Subscriber object first
        auto sub = std::shared_ptr<Subscriber<T>>(new Subscriber(participant, topic, subscriber, callback));

        // Now initialize it (create listener and reader)
        sub->initialize(qos);

        return sub;
    }

private:
    // Private constructor - use create() instead
    Subscriber(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastdds::dds::Topic* topic,
        eprosima::fastdds::dds::Subscriber* subscriber,
        MessageCallback<T> callback
    )
        : participant_(participant)
        , topic_(topic)
        , subscriber_(subscriber)
        , reader_(nullptr)
        , listener_(nullptr)
        , callback_(callback)
        , running_(false)
        , matched_publishers_(0)
        , total_received_(0)
        , callback_exceptions_(0)
    {
        if (topic_ != nullptr)
        {
            Logger::getInstance().info("Subscriber created for topic: " + std::string(topic_->get_name()));
        }
    }

    // Initialize after construction (when shared_ptr exists)
    void initialize(const eprosima::fastdds::dds::DataReaderQos& qos)
    {
        // Validate pointers
        if (participant_ == nullptr || topic_ == nullptr || subscriber_ == nullptr)
        {
            throw SubscriberException(
                ErrorCode::SUBSCRIBER_CREATE_FAILED,
                "Invalid DDS entities (null pointer)"
            );
        }

        // Create listener
        listener_ = new SubscriberListener<T>(this->weak_from_this());

        // Create DataReader
        reader_ = subscriber_->create_datareader(topic_, qos, listener_);
        if (reader_ == nullptr)
        {
            delete listener_;
            listener_ = nullptr;
            throw SubscriberException(ErrorCode::SUBSCRIBER_CREATE_FAILED, "Failed to create DataReader");
        }

        Logger::getInstance().debug("DataReader initialized for topic: " + std::string(topic_->get_name()));
    }

public:

    ~Subscriber() noexcept
    {
        try
        {
            stop();
            
            // Delete DataReader first (it holds reference to listener)
            if (reader_ != nullptr && subscriber_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
                reader_ = nullptr;
            }
            
            // Then delete listener
            if (listener_)
            {
                delete listener_;
                listener_ = nullptr;
            }

            Logger::getInstance().info("Subscriber destroyed");
        }
        catch (const std::exception& e)
        {
            Logger::getInstance().error(std::string("Exception in Subscriber destructor: ") + e.what());
        }
        catch (...)
        {
            Logger::getInstance().error("Unknown exception in Subscriber destructor");
        }
    }

    // Disable copy, move, and assignment
    // Move is unsafe because listener holds weak_ptr to this object
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;
    Subscriber(Subscriber&&) = delete;
    Subscriber& operator=(Subscriber&&) = delete;

    // Called by DDSManager after construction
    // Returns the listener to be used when creating DataReader
    void start() noexcept
    {
        running_.store(true);
        Logger::getInstance().info("Subscriber started");
    }

    void stop() noexcept
    {
        running_.store(false);
        Logger::getInstance().info("Subscriber stopped");
    }

    bool isRunning() const noexcept
    {
        return running_.load();
    }

    bool isConnected() const noexcept
    {
        return matched_publishers_.load() > 0;
    }

    int getMatchedPublishers() const noexcept
    {
        return matched_publishers_.load();
    }

    uint64_t getTotalReceived() const noexcept
    {
        return total_received_.load();
    }

    uint64_t getCallbackExceptions() const noexcept
    {
        return callback_exceptions_.load();
    }

    void setCallback(MessageCallback<T> callback)
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback_ = callback;
    }


    std::string getTopicName() const
    {
        if (topic_ != nullptr)
        {
            return topic_->get_name();
        }
        return "";
    }

    // Called by SubscriberListener
    void onDataAvailable(eprosima::fastdds::dds::DataReader* reader)
    {
        if (!running_.load())
        {
            return;
        }

        T data;
        eprosima::fastdds::dds::SampleInfo info;

        while (reader->take_next_sample(&data, &info) == eprosima::fastdds::dds::RETCODE_OK)
        {
            if (info.valid_data)
            {
                total_received_.fetch_add(1);
                
                // Copy callback to avoid holding lock during callback
                MessageCallback<T> callback_copy;
                {
                    std::lock_guard<std::mutex> lock(callback_mutex_);
                    callback_copy = callback_;
                }
                
                if (callback_copy)
                {
                    try
                    {
                        callback_copy(data);
                        Logger::getInstance().debug("Message received and callback executed");
                    }
                    catch (const std::exception& e)
                    {
                        callback_exceptions_.fetch_add(1);
                        Logger::getInstance().error(
                            std::string("Exception in user callback: ") + e.what()
                        );
                    }
                    catch (...)
                    {
                        callback_exceptions_.fetch_add(1);
                        Logger::getInstance().error("Unknown exception in user callback");
                    }
                }
            }
        }
    }

    void onPublisherMatched(int change)
    {
        matched_publishers_.fetch_add(change);

        if (change > 0)
        {
            Logger::getInstance().info(
                "Subscriber [" + getTopicName() + "] matched with " + std::to_string(change) +
                " new publisher(s). Total: " + std::to_string(matched_publishers_.load())
            );
        }
        else if (change < 0)
        {
            Logger::getInstance().warn(
                "Subscriber [" + getTopicName() + "] lost " + std::to_string(-change) +
                " publisher(s). Remaining: " + std::to_string(matched_publishers_.load())
            );
        }
    }

private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::DataReader* reader_;
    SubscriberListener<T>* listener_;  // 拥有Listener
    MessageCallback<T> callback_;
    std::mutex callback_mutex_;
    std::atomic<bool> running_;
    std::atomic<int> matched_publishers_;
    std::atomic<uint64_t> total_received_;
    std::atomic<uint64_t> callback_exceptions_;
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_SUBSCRIBER_H
