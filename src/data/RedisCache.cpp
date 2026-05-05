#include "data/RedisCache.h"
#include <iostream>
#include <stdexcept>

RedisCache::RedisCache(const std::string &host, int port) : m_connected(false)
{
  try
  {
    std::string url = "tcp://" + host + ":" + std::to_string(port);
    m_redis = std::make_unique<sw::redis::Redis>(url);
    // Test connection
    m_redis->ping();
    m_connected = true;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Redis connection failed: " << e.what() << '\n';
    m_connected = false;
  }
}

bool RedisCache::isConnected() const { return m_connected; }

std::optional<WordInfo> RedisCache::get(int wordId) const
{
  if (!m_connected || !m_redis)
  {
    return std::nullopt;
  }

  try
  {
    std::string key = buildKey(wordId);
    auto reply = m_redis->get(key);
    if (!reply)
    {
      return std::nullopt;
    }

    nlohmann::json j = nlohmann::json::parse(*reply);
    return jsonToWordInfo(j);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Redis get failed: " << e.what() << '\n';
    return std::nullopt;
  }
}

void RedisCache::set(int wordId, const WordInfo &info, int ttlSeconds) const
{
  if (!m_connected || !m_redis)
  {
    return;
  }

  try
  {
    std::string key = buildKey(wordId);
    nlohmann::json j = wordInfoToJson(info);
    std::string value = j.dump();

    if (ttlSeconds > 0)
    {
      m_redis->set(key, value, std::chrono::seconds(ttlSeconds));
    }
    else
    {
      m_redis->set(key, value);
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Redis set failed: " << e.what() << '\n';
  }
}

void RedisCache::del(int wordId) const
{
  if (!m_connected || !m_redis)
  {
    return;
  }

  try
  {
    std::string key = buildKey(wordId);
    m_redis->del(key);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Redis del failed: " << e.what() << '\n';
  }
}

std::string RedisCache::buildKey(int wordId) const { return "wordinfo:" + std::to_string(wordId); }

nlohmann::json RedisCache::wordInfoToJson(const WordInfo &info) const
{
  nlohmann::json j;
  j["id"] = info.id.value;
  j["lemma"] = info.lemma;
  j["displayLemma"] = info.displayLemma;
  j["frequency"] = info.frequency.value;

  // Etymology
  j["etymology"] = info.etymology;

  // Forms
  nlohmann::json forms = nlohmann::json::array();
  for (const auto &form : info.forms)
  {
    nlohmann::json f;
    f["form"] = form.form;
    f["tag"] = form.tag;
    forms.push_back(f);
  }
  j["forms"] = forms;

  // Senses
  nlohmann::json senses = nlohmann::json::array();
  for (const auto &sense : info.senses)
  {
    nlohmann::json s;
    s["id"] = sense.id.value;
    s["pos"] = sense.pos;
    s["definition"] = sense.definition;
    s["examples"] = sense.examples;
    s["synonyms"] = sense.synonyms;
    s["antonyms"] = sense.antonyms;
    senses.push_back(s);
  }
  j["senses"] = senses;

  return j;
}

WordInfo RedisCache::jsonToWordInfo(const nlohmann::json &j) const
{
  WordInfo info;
  info.id.value = j["id"].get<int>();
  info.lemma = j["lemma"].get<std::string>();
  info.displayLemma = j["displayLemma"].get<std::string>();
  info.frequency = dct::Frequency(j["frequency"].get<int>());

  // Etymology
  if (j.contains("etymology") && j["etymology"].is_array())
  {
    info.etymology = j["etymology"].get<std::vector<std::string>>();
  }

  // Forms
  if (j.contains("forms") && j["forms"].is_array())
  {
    for (const auto &f : j["forms"])
    {
      Form form;
      form.form = f["form"].get<std::string>();
      form.tag = f["tag"].get<std::string>();
      info.forms.push_back(form);
    }
  }

  // Senses
  if (j.contains("senses") && j["senses"].is_array())
  {
    for (const auto &s : j["senses"])
    {
      Sense sense;
      sense.id.value = s["id"].get<int>();
      sense.pos = s["pos"].get<std::string>();
      sense.definition = s["definition"].get<std::string>();
      sense.examples = s["examples"].get<std::vector<std::string>>();
      sense.synonyms = s["synonyms"].get<std::vector<std::string>>();
      sense.antonyms = s["antonyms"].get<std::vector<std::string>>();
      info.senses.push_back(sense);
    }
  }

  return info;
}
