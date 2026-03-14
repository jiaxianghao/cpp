#ifndef DDS_WRAPPER_DDSMANAGER_H
#define DDS_WRAPPER_DDSMANAGER_H

#include "Types.h"
#include "Config.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "ServiceServer.h"
#include "ServiceClient.h"
#include "Exception.h"
#include "Logger.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <mutex>

namespace dds_wrapper
{

class DDSManager
{
public:
    DDSManager();
    ~DDSManager();

    // Disable copy and assignment
    DDSManager(const DDSManager&) = delete;
    DDSManager& operator=(const DDSManager&) = delete;

    bool initialize(const std::string& config_file = "");
    bool initialize(const DDSConfig& config);
    void shutdown() noexcept;

    template<typename T>
    std::shared_ptr<Publisher<T>> createPublisher(const std::string& topic_name);

    template<typename T>
    std::shared_ptr<Subscriber<T>> createSubscriber(
        const std::string& topic_name,
        MessageCallback<T> callback
    );

    // Create a service server that handles Req messages and replies with Resp.
    // Topic convention: "<service_name>/request" and "<service_name>/response".
    template<typename Req, typename Resp>
    std::shared_ptr<ServiceServer<Req, Resp>> createServiceServer(
        const std::string& service_name,
        typename ServiceServer<Req, Resp>::Handler handler
    );

    // Create a service client that sends Req and receives Resp.
    // Topic convention: "<service_name>/request" and "<service_name>/response".
    template<typename Req, typename Resp>
    std::shared_ptr<ServiceClient<Req, Resp>> createServiceClient(
        const std::string& service_name
    );

    bool isInitialized() const noexcept;

    ConfigManager& getConfigManager();

private:
    eprosima::fastdds::dds::Topic* getOrCreateTopic(
        const std::string& topic_name,
        const std::string& type_name
    );

    eprosima::fastdds::dds::DataWriterQos createDataWriterQos() const;
    eprosima::fastdds::dds::DataReaderQos createDataReaderQos() const;

    bool initialized_;
    ConfigManager config_manager_;

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Subscriber* subscriber_;

    std::map<std::string, eprosima::fastdds::dds::Topic*> topics_;
    std::set<std::string> registered_types_;
    mutable std::mutex mutex_;
};

// Template implementation
template<typename T>
std::shared_ptr<Publisher<T>> DDSManager::createPublisher(const std::string& topic_name)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_)
    {
        throw InitializationException("DDSManager not initialized");
    }

    if (topic_name.empty())
    {
        throw PublisherException(ErrorCode::PUBLISHER_CREATE_FAILED, "Topic name cannot be empty");
    }

    try
    {
        // Register type (only once per type)
        eprosima::fastdds::dds::TypeSupport type(new typename DDS_TypeTraits<T>::PubSubType());
        std::string type_name = type.get_type_name();
        
        // Register type if not already registered
        if (registered_types_.find(type_name) == registered_types_.end())
        {
            eprosima::fastdds::dds::ReturnCode_t ret = type.register_type(participant_);
            if (ret != eprosima::fastdds::dds::RETCODE_OK && 
                ret != eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET)
            {
                throw PublisherException(
                    ErrorCode::PUBLISHER_CREATE_FAILED,
                    "Failed to register type: " + type_name
                );
            }
            registered_types_.insert(type_name);
            Logger::getInstance().debug("Type registered: " + type_name);
        }

        // Get or create topic
        eprosima::fastdds::dds::Topic* topic = getOrCreateTopic(topic_name, type_name);

        if (topic == nullptr)
        {
            throw TopicException("Failed to create topic: " + topic_name);
        }

        // Create Publisher with DataWriter using factory method
        // The factory method handles listener and writer creation internally
        eprosima::fastdds::dds::DataWriterQos qos = createDataWriterQos();
        auto pub = Publisher<T>::create(participant_, topic, publisher_, qos);

        Logger::getInstance().info("Publisher created successfully for topic: " + topic_name);
        return pub;
    }
    catch (const DDSException&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw PublisherException(ErrorCode::PUBLISHER_CREATE_FAILED, std::string("Exception: ") + e.what());
    }
}

template<typename T>
std::shared_ptr<Subscriber<T>> DDSManager::createSubscriber(
    const std::string& topic_name,
    MessageCallback<T> callback
)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_)
    {
        throw InitializationException("DDSManager not initialized");
    }

    if (topic_name.empty())
    {
        throw SubscriberException(ErrorCode::SUBSCRIBER_CREATE_FAILED, "Topic name cannot be empty");
    }

    if (!callback)
    {
        throw SubscriberException(ErrorCode::SUBSCRIBER_CREATE_FAILED, "Callback cannot be null");
    }

    try
    {
        // Register type (only once per type)
        eprosima::fastdds::dds::TypeSupport type(new typename DDS_TypeTraits<T>::PubSubType());
        std::string type_name = type.get_type_name();
        
        // Register type if not already registered
        if (registered_types_.find(type_name) == registered_types_.end())
        {
            eprosima::fastdds::dds::ReturnCode_t ret = type.register_type(participant_);
            if (ret != eprosima::fastdds::dds::RETCODE_OK && 
                ret != eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET)
            {
                throw SubscriberException(
                    ErrorCode::SUBSCRIBER_CREATE_FAILED,
                    "Failed to register type: " + type_name
                );
            }
            registered_types_.insert(type_name);
            Logger::getInstance().debug("Type registered: " + type_name);
        }

        // Get or create topic
        eprosima::fastdds::dds::Topic* topic = getOrCreateTopic(topic_name, type_name);

        if (topic == nullptr)
        {
            throw TopicException("Failed to create topic: " + topic_name);
        }

        // Create Subscriber with DataReader using factory method
        // The factory method handles listener and reader creation internally
        eprosima::fastdds::dds::DataReaderQos qos = createDataReaderQos();
        auto sub = Subscriber<T>::create(participant_, topic, subscriber_, qos, callback);

        // Start receiving messages
        sub->start();

        Logger::getInstance().info("Subscriber created successfully for topic: " + topic_name);
        return sub;
    }
    catch (const DDSException&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw SubscriberException(ErrorCode::SUBSCRIBER_CREATE_FAILED, std::string("Exception: ") + e.what());
    }
}

template<typename Req, typename Resp>
std::shared_ptr<ServiceServer<Req, Resp>> DDSManager::createServiceServer(
    const std::string& service_name,
    typename ServiceServer<Req, Resp>::Handler handler
)
{
    if (service_name.empty())
    {
        throw PublisherException(ErrorCode::PUBLISHER_CREATE_FAILED, "Service name cannot be empty");
    }

    // Publisher on the response topic must exist before wiring the handler,
    // because the handler lambda captures it by value.
    auto pub = createPublisher<Resp>(service_name + "/response");

    // Temporary placeholder callback; will be replaced by setHandler() below.
    auto sub = createSubscriber<Req>(
        service_name + "/request",
        [](const Req&) {}
    );

    auto server = ServiceServer<Req, Resp>::create(service_name, pub, sub);

    // Install the real handler now that both pub and sub are alive.
    server->setHandler(handler, pub);

    Logger::getInstance().info("ServiceServer created for service: " + service_name);
    return server;
}

template<typename Req, typename Resp>
std::shared_ptr<ServiceClient<Req, Resp>> DDSManager::createServiceClient(
    const std::string& service_name
)
{
    if (service_name.empty())
    {
        throw PublisherException(ErrorCode::PUBLISHER_CREATE_FAILED, "Service name cannot be empty");
    }

    auto pub = createPublisher<Req>(service_name + "/request");

    // Create client before subscriber so the weak_ptr inside the lambda is valid.
    auto client = ServiceClient<Req, Resp>::create(service_name, pub);

    std::weak_ptr<ServiceClient<Req, Resp>> weak_client = client;
    auto sub = createSubscriber<Resp>(
        service_name + "/response",
        [weak_client](const Resp& resp)
        {
            auto c = weak_client.lock();
            if (c)
            {
                c->onResponse(resp);
            }
        }
    );

    client->setSubscriber(sub);

    Logger::getInstance().info("ServiceClient created for service: " + service_name);
    return client;
}

} // namespace dds_wrapper

#endif // DDS_WRAPPER_DDSMANAGER_H
