#pragma once

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <utility>
#include <nlohmann/json.hpp>

#include "Database.h"
#include "Dictionary.h"
#include "SpellChecker.h"

namespace test_support {

inline std::filesystem::path tempDbPath(const std::string& name = "qq_test.sqlite") {
    return std::filesystem::temp_directory_path() / name;
}

inline void writeConfigForDb(const std::filesystem::path& dbPath) {
    nlohmann::json j;
    j["database_path"] = dbPath.string();
    j["server_port"] = 0;
    std::ofstream out("config.json", std::ios::trunc);
    out << j.dump(2);
    // Also set env so Config picks it up without relying on cwd
    setenv("DATABASE_PATH", dbPath.string().c_str(), 1);
}

inline Database makeFreshDb(const std::filesystem::path& dbPath) {
    std::filesystem::remove(dbPath);
    writeConfigForDb(dbPath);
    Database db(dbPath.string());
    db.createTables();
    return std::move(db);
}
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
inline void seedWord(Database& db,
                     const std::string& lemma,
                     const std::string& definition,
                     const std::vector<std::string>& synonyms = {},
                     int freq = 1) {
    auto id = db.insertWord(lemma, lemma, dct::Frequency{freq});
    auto sense = db.insertSense(id, "", definition);
    for (const auto& s : synonyms) db.insertSynonym(sense, s);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
inline Dictionary makeDictionaryOnDb(const std::filesystem::path& dbPath) {
    // Ensures Config reads the right path when Dictionary constructs
    setenv("DATABASE_PATH", dbPath.string().c_str(), 1);
    return Dictionary{};
}

inline SpellChecker makeSpellCheckerOnDb(const std::filesystem::path& dbPath) {
    auto dict = makeDictionaryOnDb(dbPath);
    return SpellChecker{dict};
}

} // namespace test_support
