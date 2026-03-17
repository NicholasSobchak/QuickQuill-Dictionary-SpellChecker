#include "http/Server.h"
#include "http/handlers/WordHandler.h"

int main()
{
	// construct dictionary and database
	http::warmupDictionary();
	http::runServer(8080);	
    return 0;
}
