#include "Config.h"
#include <fstream>
#include <iostream>

Config::Config() 
{
    std::ifstream f("config.json");
    if (f) 
	{
        data = nlohmann::json::parse(f);
    } 
	else {
        std::cerr << "Warning: config.json not found. Using default values.\n"; 
    }
}

std::string Config::getDatabasePath() const 
{
    if (data.contains("database_path")) 
	{
        return data["database_path"].get<std::string>();
    }
    return "dictionary.db"; // Default value
}

int Config::getServerPort() const 
{
    if (data.contains("server_port")) 
	{
        return data["server_port"].get<int>();
    }
    return 8080; // Default value
}
