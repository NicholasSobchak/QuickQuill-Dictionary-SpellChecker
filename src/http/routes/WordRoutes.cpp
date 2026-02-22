#include "http/routes/WordRoutes.h"
#include "http/handlers/WordHandler.h"

namespace http
{
	void registerWordRoutes(crow::SimpleApp& app)
	{
		// TODO: Implement Crow route definitions for dictionary endpoints.
		CROW_ROUTE(app, "/api/health") ([] { 
			return crow::response(200, "{\"ok\":true}"); 
		});
	
		CROW_ROUTE(app, "/api/word/<string>") 
		([](const std::string& word) { 
			return crow::response(200, search(word));
		});
	}
}
