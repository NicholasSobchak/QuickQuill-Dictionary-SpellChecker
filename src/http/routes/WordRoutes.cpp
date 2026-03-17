#include "http/routes/WordRoutes.h"
#include "http/handlers/WordHandler.h"

#include <fstream>
#include <sstream>
#include <string>

namespace http
{
	namespace // actual API endpoints
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

		crow::response fileResponseFromFile(const std::string& path, const std::string& contentType)
		{
			std::ifstream file(path, std::ios::binary);
			if (!file)
			{
				return crow::response(404, "File not found.");
			}

			std::ostringstream ss;
			ss << file.rdbuf();
			crow::response response(200, ss.str());
			response.set_header("Content-Type", contentType);
			return response;
		}
	}

	// Requests
	void registerWordRoutes(crow::SimpleApp& app)
	{
		// WEB setup
		CROW_ROUTE(app, "/") ([] {
			return htmlResponseFromFile("web/index.html");
		});

		CROW_ROUTE(app, "/assets/Quotex.otf") ([] {
			return fileResponseFromFile("web/assets/Quotex.otf", "font/otf");
		});

		CROW_ROUTE(app, "/assets/fonts.css") ([] {
			return fileResponseFromFile("web/assets/fonts.css", "text/css; charset=utf-8");
		});

		CROW_ROUTE(app, "/assets/quill.png") ([] {
			return fileResponseFromFile("web/assets/quill.png", "image/png");
		});

		CROW_ROUTE(app, "/assets/nevermore-logo-no-bg.png") ([] {
			return fileResponseFromFile("web/assets/nevermore-logo-no-bg.png", "image/png");
		});

		CROW_ROUTE(app, "/assets/nevermore_backdrop.jpg") ([] {
			return fileResponseFromFile("web/assets/nevermore_backdrop.jpg", "image/jpeg");
		});


		// GET requests
		CROW_ROUTE(app, "/api/health") ([] {
			return jsonResponse("{\"ok\":true}");
		});

		CROW_ROUTE(app, "/api/word/<string>")
		([](const std::string& word) {
			auto result = search(word);
			return jsonResponse(result.body, result.status);
		});
	}
}
