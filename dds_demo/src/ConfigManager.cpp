#include "dds_wrapper/Config.h"
#include "dds_wrapper/Logger.h"
#include <fstream>
#include <sstream>

namespace dds_wrapper
{

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::loadFromJSON(const std::string& json_file)
{
    try
    {
        std::ifstream file(json_file);
        if (!file.is_open())
        {
            return false;
        }

        // Simple JSON parsing (in production, use a proper JSON library like nlohmann/json)
        // For now, this is a placeholder that sets default values
        // TODO: Implement proper JSON parsing using nlohmann/json

        std::string line;
        while (std::getline(file, line))
        {
            // Parse domain_id
            if (line.find("\"domain_id\"") != std::string::npos)
            {
                size_t pos = line.find(":");
                if (pos != std::string::npos)
                {
                    std::string value = line.substr(pos + 1);
                    // Remove spaces, commas
                    value.erase(std::remove_if(value.begin(), value.end(),
                        [](char c)
                        {
                            return std::isspace(c) || c == ',';
                        }), value.end());
                    config_.domain_id = std::stoi(value);
                }
            }
        }

        file.close();
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(std::string("Failed to load JSON config: ") + e.what());
        return false;
    }
}

bool ConfigManager::loadFromXML(const std::string& xml_file)
{
    // TODO: Implement XML parsing for FastDDS native XML profiles
    Logger::getInstance().warn("XML configuration loading not yet implemented");
    return false;
}

bool ConfigManager::saveToJSON(const std::string& json_file) const
{
    try
    {
        std::ofstream file(json_file);
        if (!file.is_open())
        {
            return false;
        }

        file << "{\n";
        file << "    \"domain_id\": " << config_.domain_id << ",\n";
        file << "    \"participant_name\": \"" << config_.participant_name << "\",\n";
        file << "    \"qos\": {\n";
        file << "        \"reliability\": \"" << (config_.reliability == ReliabilityKind::RELIABLE ? "reliable" : "best_effort") << "\",\n";
        file << "        \"durability\": \"transient_local\",\n";
        file << "        \"history_depth\": " << config_.history_depth << "\n";
        file << "    },\n";
        file << "    \"logging\": {\n";
        file << "        \"level\": \"INFO\",\n";
        file << "        \"file_output\": " << (config_.log_file_output ? "true" : "false") << ",\n";
        file << "        \"log_dir\": \"" << config_.log_dir << "\",\n";
        file << "        \"console_output\": " << (config_.log_console_output ? "true" : "false") << "\n";
        file << "    }\n";
        file << "}\n";

        file.close();
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(std::string("Failed to save JSON config: ") + e.what());
        return false;
    }
}

bool ConfigManager::saveToXML(const std::string& xml_file) const
{
    // TODO: Implement XML saving
    Logger::getInstance().warn("XML configuration saving not yet implemented");
    return false;
}

const DDSConfig& ConfigManager::getConfig() const
{
    return config_;
}

void ConfigManager::setConfig(const DDSConfig& config)
{
    config_ = config;
}

void ConfigManager::setDomainId(int domain_id)
{
    config_.domain_id = domain_id;
}

void ConfigManager::setReliability(ReliabilityKind reliability)
{
    config_.reliability = reliability;
}

void ConfigManager::setDurability(DurabilityKind durability)
{
    config_.durability = durability;
}

void ConfigManager::setHistoryDepth(int depth)
{
    config_.history_depth = depth;
}

} // namespace dds_wrapper
