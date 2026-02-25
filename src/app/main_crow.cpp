#include "http/Server.h"

// TODO: Crow entrypoint.
// Suggested flow:
// 1) Load dictionary/service state.
// 2) Call http::runServer(port).

int main()
{
	http::runServer(8080);	
    return 0;
}
