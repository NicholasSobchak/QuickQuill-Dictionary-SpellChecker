#include "http/routes/WordRoutes.h"
#include "http/handlers/WordHandler.h"

#include <fstream>
#include <sstream>
#include <string>

namespace http
{
	namespace
	{
		crow::response jsonResponse(const std::string& body, int status = 200)
		{
			crow::response response(status, body);
			response.set_header("Content-Type", "application/json");
			return response;
		}

		crow::response htmlResponseFromFile(const std::string& path)
		{
			std::ifstream file(path);
			if (!file)
			{
				return crow::response(500, "Failed to load frontend HTML.");
			}

			std::ostringstream ss;
			ss << file.rdbuf();
			crow::response response(200, ss.str());
			response.set_header("Content-Type", "text/html; charset=utf-8");
			return response;
		}
	}


	void registerWordRoutes(crow::SimpleApp& app)
	{
		// GET requests
		CROW_ROUTE(app, "/") ([] {
			return htmlResponseFromFile("web/index.html");
		});

		CROW_ROUTE(app, "/api/health") ([] {
			return jsonResponse("{\"ok\":true}");
		});

		CROW_ROUTE(app, "/api/word/<string>")
		([](const std::string& word) {
			return jsonResponse(search(word));
		});
	}
}
