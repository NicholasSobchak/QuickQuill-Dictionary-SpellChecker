#ifndef REDIS_CACHE_H
#define REDIS_CACHE_H

#include <dct/WordInfo.h>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <sw/redis++/redis++.h>

class RedisCache
{
public:
  RedisCache(const std::string &host, int port);

  /**
   * Get WordInfo from Redis, returns nullopt if not found
   */
  std::optional<WordInfo> get(int wordId) const;

  /**
   * Store WordInfo in Redis with optional TTL (default 1 hour)
   */
  void set(int wordId, const WordInfo &info, int ttlSeconds = 3600) const;

  /**
   * Remove from cache
   */
  void del(int wordId) const;

  /**
   * Check if Redis is connected
   */
  bool isConnected() const;

private:
  std::string m_host;
  int m_port;
  mutable std::mutex m_mutex;
  mutable std::unique_ptr<sw::redis::Redis> m_redis;
  mutable bool m_connected;

  bool ensureConnected() const;
  void connect() const;

  nlohmann::json wordInfoToJson(const WordInfo &info) const;
  WordInfo jsonToWordInfo(const nlohmann::json &j) const;
  std::string buildKey(int wordId) const;
};

#endif
