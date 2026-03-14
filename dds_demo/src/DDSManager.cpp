#include "dds_wrapper/DDSManager.h"
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;

namespace dds_wrapper
{

DDSManager::DDSManager()
    : initialized_(false)
    , participant_(nullptr)
    , publisher_(nullptr)
    , subscriber_(nullptr)
{
}

DDSManager::~DDSManager()
{
    shutdown();
}

bool DDSManager::initialize(const std::string& config_file)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_)
    {
        Logger::getInstance().warn("DDSManager already initialized");
        return true;
    }

    try
    {
        // Load configuration
        if (!config_file.empty())
        {
            if (!config_manager_.loadFromJSON(config_file))
            {
                Logger::getInstance().error("Failed to load configuration from: " + config_file);
                return false;
            }
        }

        const DDSConfig& config = config_manager_.getConfig();

        // Initialize logger first
        Logger::getInstance().initialize(
            config.log_level,
            config.log_console_output,
            config.log_file_output,
            config.log_dir
        );

        Logger::getInstance().info("Initializing DDSManager...");
        Logger::getInstance().info("Domain ID: " + std::to_string(config.domain_id));
        Logger::getInstance().info("Participant: " + config.participant_name);

        // Create DomainParticipant
        DomainParticipantQos pqos;
        pqos.name(config.participant_name);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(
            config.domain_id,
            pqos
        );

        if (participant_ == nullptr)
        {
            Logger::getInstance().error("Failed to create DomainParticipant");
            return false;
        }

        // Create Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (publisher_ == nullptr)
        {
            Logger::getInstance().error("Failed to create Publisher");
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
            participant_ = nullptr;
            return false;
        }

        // Create Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        if (subscriber_ == nullptr)
        {
            Logger::getInstance().error("Failed to create Subscriber");
            participant_->delete_publisher(publisher_);
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
            participant_ = nullptr;
            publisher_ = nullptr;
            return false;
        }

        initialized_ = true;
        Logger::getInstance().info("DDSManager initialized successfully");
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(std::string("Exception during initialization: ") + e.what());
        shutdown();
        return false;
    }
}

bool DDSManager::initialize(const DDSConfig& config)
{
    config_manager_.setConfig(config);
    return initialize("");
}

void DDSManager::shutdown() noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_)
        {
            return;
        }

        Logger::getInstance().info("Shutting down DDSManager...");

        // Delete all topics
        for (auto& pair : topics_)
        {
            try
            {
                if (participant_ != nullptr && pair.second != nullptr)
                {
                    participant_->delete_topic(pair.second);
                }
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().error(std::string("Error deleting topic: ") + e.what());
            }
        }
        topics_.clear();
        registered_types_.clear();

        // Delete subscriber
        if (subscriber_ != nullptr && participant_ != nullptr)
        {
            try
            {
                participant_->delete_subscriber(subscriber_);
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().error(std::string("Error deleting subscriber: ") + e.what());
            }
            subscriber_ = nullptr;
        }

        // Delete publisher
        if (publisher_ != nullptr && participant_ != nullptr)
        {
            try
            {
                participant_->delete_publisher(publisher_);
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().error(std::string("Error deleting publisher: ") + e.what());
            }
            publisher_ = nullptr;
        }

        // Delete participant
        if (participant_ != nullptr)
        {
            try
            {
                DomainParticipantFactory::get_instance()->delete_participant(participant_);
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().error(std::string("Error deleting participant: ") + e.what());
            }
            participant_ = nullptr;
        }

        initialized_ = false;
        Logger::getInstance().info("DDSManager shut down successfully");
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(std::string("Exception in shutdown: ") + e.what());
    }
    catch (...)
    {
        Logger::getInstance().error("Unknown exception in shutdown");
    }
}

bool DDSManager::isInitialized() const noexcept
{
    return initialized_;
}

ConfigManager& DDSManager::getConfigManager()
{
    return config_manager_;
}

Topic* DDSManager::getOrCreateTopic(const std::string& topic_name, const std::string& type_name)
{
    auto it = topics_.find(topic_name);
    if (it != topics_.end())
    {
        // Verify type name matches
        if (it->second->get_type_name() != type_name)
        {
            Logger::getInstance().error(
                "Topic " + topic_name + " already exists with different type: " +
                std::string(it->second->get_type_name()) + " vs " + type_name
            );
            return nullptr;
        }
        return it->second;
    }

    Topic* topic = participant_->create_topic(topic_name, type_name, TOPIC_QOS_DEFAULT);
    if (topic != nullptr)
    {
        topics_[topic_name] = topic;
        Logger::getInstance().info("Topic created: " + topic_name + " (type: " + type_name + ")");
    }
    else
    {
        Logger::getInstance().error("Failed to create topic: " + topic_name);
    }

    return topic;
}

DataWriterQos DDSManager::createDataWriterQos() const
{
    const DDSConfig& config = config_manager_.getConfig();
    
    DataWriterQos qos = DATAWRITER_QOS_DEFAULT;

    // Set reliability
    if (config.reliability == ReliabilityKind::RELIABLE)
    {
        qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    // Set durability
    switch (config.durability)
    {
        case DurabilityKind::VOLATILE:
            qos.durability().kind = VOLATILE_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT_LOCAL:
            qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT:
            qos.durability().kind = TRANSIENT_DURABILITY_QOS;
            break;
        case DurabilityKind::PERSISTENT:
            qos.durability().kind = PERSISTENT_DURABILITY_QOS;
            break;
    }

    // Set history depth
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = config.history_depth;

    Logger::getInstance().debug(
        "DataWriter QoS: reliability=" + 
        std::string(config.reliability == ReliabilityKind::RELIABLE ? "RELIABLE" : "BEST_EFFORT") +
        ", history_depth=" + std::to_string(config.history_depth)
    );

    return qos;
}

DataReaderQos DDSManager::createDataReaderQos() const
{
    const DDSConfig& config = config_manager_.getConfig();
    
    DataReaderQos qos = DATAREADER_QOS_DEFAULT;

    // Set reliability
    if (config.reliability == ReliabilityKind::RELIABLE)
    {
        qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    // Set durability
    switch (config.durability)
    {
        case DurabilityKind::VOLATILE:
            qos.durability().kind = VOLATILE_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT_LOCAL:
            qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT:
            qos.durability().kind = TRANSIENT_DURABILITY_QOS;
            break;
        case DurabilityKind::PERSISTENT:
            qos.durability().kind = PERSISTENT_DURABILITY_QOS;
            break;
    }

    // Set history depth
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = config.history_depth;

    Logger::getInstance().debug(
        "DataReader QoS: reliability=" + 
        std::string(config.reliability == ReliabilityKind::RELIABLE ? "RELIABLE" : "BEST_EFFORT") +
        ", history_depth=" + std::to_string(config.history_depth)
    );

    return qos;
}

} // namespace dds_wrapper
