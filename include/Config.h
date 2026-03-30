#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

class Config 
{
public:
    static Config& getInstance() 
	{
        static Config instance;
        return instance;
    }

    Config(Config const&) = delete;
    void operator=(Config const&) = delete;

    std::string getDatabasePath() const;
    int getServerPort() const;

private:
    Config();
    nlohmann::json data;
};
#endif // CONFIG_H
