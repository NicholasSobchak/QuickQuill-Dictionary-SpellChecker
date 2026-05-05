#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "dct/WordInfo.h"

/**
 * This class acts as a wrapper around a C library
 */
class Database
{
public:
  // callback function for word frequency
  using WordRecordProcessor =
      std::function<void(dct::WordId id, std::string_view text, dct::Frequency frequency)>;

  Database(std::string_view filename);

  /**
   * Rule of 5 necessary for creating a mock DB for tests
   */
  Database(const Database &) = delete;
  Database &operator=(const Database &) = delete;
  Database(Database &&) noexcept = default;
  Database &operator=(Database &&) noexcept = default;
  ~Database() = default;

  void createTables();
  void clearDB();

  /**
   * Escape hatch for import/maintenance tools that need prepared
   * statements. The Database instance retains ownership; callers must not
   * close the handle.
   */
  sqlite3 *handle() const noexcept;

  /**
   * Database INSERT functions
   */
  bool insertEtymology(dct::WordId word_id, const std::vector<std::string> &etymology);
  bool insertForm(dct::WordId word_id, const std::string &form, const std::string &tag);
  bool insertExample(dct::WordId sense_id, const std::string &example);
  bool insertSynonym(dct::WordId sense_id, const std::string &synonym);
  bool insertAntonym(dct::WordId sense_id, const std::string &antonym);
  dct::WordId
  insertWord(const std::string &lemma, const std::string &displayLemma, dct::Frequency frequency);
  dct::WordId
  insertSense(dct::WordId word_id, const std::string &pos, const std::string &definition);

  /**
   * Return true for an empty database
   */
  bool isEmpty() const;

  /**
   * Database SELECT function
   */
  bool contains(std::string_view word) const;

  /**
   * Gets all words and forms to load into the Trie class
   */
  void streamAllWordsAndForms(const WordRecordProcessor &processor) const;

  /**
   * Return all info into WordInfo
   */
  WordInfo getInfo(dct::WordId word_id) const;

  /**
   * Finds alternative words that use the same lemma
   */
  std::vector<dct::WordId> findMatchingWordIds(std::string_view word) const;

private:
  struct Sqlite3Deleter
  {
    void operator()(sqlite3 *db) const
    {
      if (db)
      {
        sqlite3_close(db);
      }
    }
  };

  std::unique_ptr<sqlite3, Sqlite3Deleter> m_db;

  /**
   * Helpers for "fetching" (SELECT) for vectors
   */
  std::vector<Form> fetchForms(dct::WordId word_id) const;
  std::vector<std::string> fetchStrings(std::string_view sql, dct::WordId id) const;
};
#endif
