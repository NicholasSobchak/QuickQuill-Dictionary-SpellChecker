#include "app/Config.h"
#include "http/Server.h"
#include "http/services/WordService.h"

int main() {
  http::wordService().warmupDictionary();
  http::runServer(Config::getInstance().getServerPort());
  return 0;
}
