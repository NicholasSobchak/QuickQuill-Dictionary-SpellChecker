#include "http/Server.h"
#include "http/handlers/WordHandler.h"
#include "Config.h"

int main()
{
	// construct dictionary and database
	http::warmupDictionary();
	http::runServer(Config::getInstance().getServerPort());
    return 0;
}
