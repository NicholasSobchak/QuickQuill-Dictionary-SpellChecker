#ifndef REDIS_CACHE_H
#define REDIS_CACHE_H

#include <dct/WordInfo.h>
#include <memory>
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
  std::unique_ptr<sw::redis::Redis> m_redis;
  bool m_connected;

  /**
   * Serialization helpers
   */
  nlohmann::json wordInfoToJson(const WordInfo &info) const;
  WordInfo jsonToWordInfo(const nlohmann::json &j) const;
  std::string buildKey(int wordId) const;
};

#endif
