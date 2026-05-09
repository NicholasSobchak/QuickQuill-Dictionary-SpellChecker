#include "http/routes/WordRoutes.h"
#include "http/services/WordService.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace http
{
namespace
{
const std::string kDistRoot = "web/dist";

/**
 * Type safety: wrapper carrying a MIME type string
 */
struct ContentType
{
  std::string value;
  explicit ContentType(std::string v) : value(std::move(v)) {}
};

crow::response jsonResponse(const std::string &body, int status = 200)
{
  crow::response response(status, body);
  response.set_header("Content-Type", "application/json");
  return response;
}

crow::response htmlResponseFromFile(const std::string &path)
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

std::string guessContentType(const std::string &path)
{
  static const std::vector<std::pair<std::string, std::string>> mapping = {
      {".html", "text/html; charset=utf-8"},
      {".js", "application/javascript"},
      {".css", "text/css; charset=utf-8"},
      {".png", "image/png"},
      {".jpg", "image/jpeg"},
      {".jpeg", "image/jpeg"},
      {".svg", "image/svg+xml"},
      {".gif", "image/gif"},
      {".webp", "image/webp"},
      {".woff2", "font/woff2"},
      {".woff", "font/woff"},
      {".ttf", "font/ttf"},
      {".otf", "font/otf"},
  };

  for (const auto &[ext, mime] : mapping)
  {
    if (path.size() >= ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0)
    {
      return mime;
    }
  }
  return "application/octet-stream";
}

crow::response fileResponseFromFile(const std::string &path, const ContentType &contentType)
{
  std::ifstream file(path, std::ios::binary);
  if (!file)
  {
    return crow::response(404, "File not found.");
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  crow::response response(200, ss.str());
  response.set_header("Content-Type", contentType.value);
  return response;
}
} // namespace

/**
 * Requests
 */
void registerWordRoutes(crow::SimpleApp &app)
{
  /**
   * Static frontend from Vite build (web/dist)
   */
  CROW_ROUTE(app, "/")
  ([] { return htmlResponseFromFile(kDistRoot + "/index.html"); });

  /**
   * Serve hashed assets (and public assets) with a catch-all in /assets/
   */
  CROW_ROUTE(app, "/assets/<path>")
  (
      [](const std::string &path)
      {
        const std::string full = kDistRoot + "/assets/" + path;
        const auto mime = guessContentType(full);
        return fileResponseFromFile(full, ContentType(mime));
      });

  /**
   * GET requests
   */
  CROW_ROUTE(app, "/api/health")
  ([] { return jsonResponse("{\"ok\":true}"); });

  CROW_ROUTE(app, "/api/word/")
  ([] { return jsonResponse("{\"error\":\"Enter a valid word\"}", 400); });

  CROW_ROUTE(app, "/api/word/<string>")
  (
      [](const std::string &word)
      {
        auto result = wordService().search(word);
        return jsonResponse(result.body, result.status);
      });

  CROW_ROUTE(app, "/api/suggest/<string>")
  (
      [](const std::string &word)
      {
        auto result = wordService().suggest(word);
        return jsonResponse(result.body, result.status);
      });

  CROW_ROUTE(app, "/api/synonym/<string>")
  (
      [](const std::string &word)
      {
        const auto res = wordService().suggestSynonym(word);
        return jsonResponse(res.body, res.status);
      });

  CROW_ROUTE(app, "/api/autofill/<string>")
  (
      [](const crow::request &req, const std::string &word)
      {
        std::vector<std::string> history;
        std::vector<std::string> suggested;

        if (const char *h = req.url_params.get("history"))
        {
          std::string hstr(h);
          std::istringstream ss(hstr);
          std::string item;
          while (std::getline(ss, item, ','))
          {
            if (!item.empty())
            {
              history.push_back(std::move(item));
            }
          }
        }

        if (const char *s = req.url_params.get("suggested"))
        {
          std::string sstr(s);
          std::istringstream ss(sstr);
          std::string item;
          while (std::getline(ss, item, ','))
          {
            if (!item.empty())
            {
              suggested.push_back(std::move(item));
            }
          }
        }

        auto result = wordService().autofill(word, history, suggested);
        return jsonResponse(result.body, result.status);
      });
}
} // namespace http
