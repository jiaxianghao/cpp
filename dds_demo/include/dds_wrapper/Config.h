#ifndef DDS_WRAPPER_CONFIG_H
#define DDS_WRAPPER_CONFIG_H

#include "Types.h"
#include <string>
#include <map>

namespace dds_wrapper
{

// Configuration structure
struct DDSConfig
{
    int domain_id = 0;
    std::string participant_name = "DDSWrapper_Participant";

    // QoS settings
    ReliabilityKind reliability = ReliabilityKind::RELIABLE;
    DurabilityKind durability = DurabilityKind::TRANSIENT_LOCAL;
    int history_depth = 10;

    // Logging settings
    LogLevel log_level = LogLevel::INFO;
    bool log_file_output = true;
    std::string log_dir = "logs";
    bool log_console_output = true;

    // Custom properties
    std::map<std::string, std::string> custom_properties;
};

// Configuration manager
class ConfigManager
{
public:
    ConfigManager();
    ~ConfigManager();

    bool loadFromJSON(const std::string& json_file);
    bool loadFromXML(const std::string& xml_file);
    bool saveToJSON(const std::string& json_file) const;
    bool saveToXML(const std::string& xml_file) const;

    const DDSConfig& getConfig() const;
    void setConfig(const DDSConfig& config);

    void setDomainId(int domain_id);
    void setReliability(ReliabilityKind reliability);
    void setDurability(DurabilityKind durability);
    void setHistoryDepth(int depth);

private:
    DDSConfig config_;
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_CONFIG_H
