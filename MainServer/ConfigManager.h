#pragma once
#include "json.hpp"
 
struct ServerConfig
{
	std::string dataPath;
};


class ConfigManager
{
public:
	static void LoadConfig();

public:
	static ServerConfig Config;

};

