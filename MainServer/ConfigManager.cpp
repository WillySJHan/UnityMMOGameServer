#include "pch.h"
#include "ConfigManager.h"
#include <fstream>

ServerConfig ConfigManager::Config;

void ConfigManager::LoadConfig()
{
    std::ifstream configFile("../Resource/Config/config.json");
    if (configFile.is_open()) {
        nlohmann::json jsonConfig;
        configFile >> jsonConfig;

        if (jsonConfig.find("dataPath") != jsonConfig.end()) {
            Config.dataPath = jsonConfig["dataPath"];
        }
        else 
        {
            std::cout << "���� ���� ����" << std::endl;
        }


    }
    else 
    {
        std::cout << "���� ���� ����" << std::endl;
    }
}
