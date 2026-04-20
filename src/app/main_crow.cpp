#include "http/Server.h"
#include "http/services/WordService.h"
#include "Config.h"

int main()
{
	http::wordService().warmupDictionary();
	http::runServer(Config::getInstance().getServerPort());
    return 0;
}
