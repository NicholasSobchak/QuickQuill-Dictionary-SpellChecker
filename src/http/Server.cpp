#include "http/Server.h"
#include "http/routes/WordRoutes.h"

#if __has_include(<crow.h>)
#include <crow.h>
#elif __has_include(<crow/crow.h>)
#include <crow/crow.h>
#else
#error "Crow headers not found (need crow.h or crow/crow.h)"
#endif

namespace http
{
void runServer(const int port)
{
  crow::SimpleApp app;
  registerWordRoutes(app);
  app.port(port).concurrency(2).run();
}
} // namespace http
