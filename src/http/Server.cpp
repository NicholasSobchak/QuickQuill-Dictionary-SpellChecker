#include "http/Server.h"
#include "http/routes/WordRoutes.h"
#include "logging.h"

#if __has_include(<crow.h>)
#include <crow.h>
#elif __has_include(<crow/crow.h>)
#include <crow/crow.h>
#else
#error "Crow headers not found (need crow.h or crow/crow.h)"
#endif

#include <atomic>
#include <chrono>
#include <signal.h>
#include <thread>

namespace http
{
std::atomic<bool> g_shutdown{false};

void signalHandler(int) { g_shutdown.store(true); }

void runServer(const int port)
{
  // Setup signal handlers for graceful shutdown
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);

  crow::SimpleApp app;
  registerWordRoutes(app);

  CROW_LOG_INFO << "Starting server on port " << port << " with 4 threads";
  app.port(port).concurrency(4).timeout(5 /*s*/);

  // Run server in a way that can be interrupted
  auto serverThread = std::thread([&app]() { app.run(); });

  // Wait for shutdown signal
  while (!g_shutdown.load())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  app.stop();
  serverThread.join();
}
} // namespace http
