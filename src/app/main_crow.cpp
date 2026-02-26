#include "http/Server.h"
#include "http/handlers/WordHandler.h"

// TODO: Crow entrypoint.
// Suggested flow:
// 1) Load dictionary/service state.
// 2) Call http::runServer(port).

int main()
{
	// construct dictionary and database
	http::warmupDictionary();
	http::runServer(8080);	
    return 0;
}
