#include "Database.h"
#include "Utils.h"

Database::Database(std::string_view filename) {
    sqlite3 *db_ptr = nullptr;
    if (sqlite3_open(filename.data(), &db_ptr)) {
        throw std::runtime_error("Error: Can't open database\n");
    }
    m_db.reset(db_ptr);
}

void Database::createTables() {
    /*
        Word
        ├── Etymology (one)
        └── Senses (many)
            ├── Definition (one)
            ├── POS (part of speech) (one)
            ├── Examples (many)
            ├── Synonyms (many)
            └── Antonyms (many)
    */

    const char *stmts[] = {
        "CREATE TABLE IF NOT EXISTS words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "lemma TEXT UNIQUE,"
        "display_lemma TEXT);",

        "CREATE TABLE IF NOT EXISTS etymologies ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "etymology TEXT NOT NULL,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS forms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "form TEXT NOT NULL,"
        "tag TEXT,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS senses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "pos TEXT,"
        "definition TEXT NOT NULL,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS examples ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "example TEXT NOT NULL,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS synonyms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "synonym TEXT,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS antonyms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "antonym TEXT,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);"};

    char *errMsg = nullptr;

    for (auto s : stmts) {
        if (sqlite3_exec(m_db.get(), s, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << '\n';
            sqlite3_free(errMsg);
        }
    }

    const char *idx[] = {
        "CREATE INDEX IF NOT EXISTS idx_words_lemma ON words(lemma);",
        "CREATE INDEX IF NOT EXISTS idx_etymologies_word_id ON "
        "etymologies(word_id);",
        "CREATE INDEX IF NOT EXISTS idx_forms_word_id ON forms(word_id);",
        "CREATE INDEX IF NOT EXISTS idx_senses_word_id ON senses(word_id);",
        "CREATE INDEX IF NOT EXISTS idx_examples_sense_id ON examples(sense_id);",
        "CREATE INDEX IF NOT EXISTS idx_synonyms_sense_id ON synonyms(sense_id);",
        "CREATE INDEX IF NOT EXISTS idx_antonyms_sense_id ON "
        "antonyms(sense_id);",
    };

    for (auto s : idx) {
        if (sqlite3_exec(m_db.get(), s, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << '\n';
            sqlite3_free(errMsg);
        }
    }
}

int Database::insertWord(const std::string &lemma,
                         const std::string &displayLemma) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT OR IGNORE INTO words (lemma, display_lemma) VALUES (?, "
                    "?);"};

    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return dct::g_defaultId;

    sqlite3_bind_text(stmt, 1, lemma.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, displayLemma.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return dct::g_defaultId;
    }
    if (stmt)
        sqlite3_finalize(stmt);

    if (sqlite3_changes(m_db.get()) > 0) {
        return static_cast<int>(sqlite3_last_insert_rowid(m_db.get()));
    }

    const char *selectSql{"SELECT id FROM words WHERE lemma = ?;"};
    if (sqlite3_prepare_v2(m_db.get(), selectSql, -1, &stmt, nullptr) !=
        SQLITE_OK)
        return dct::g_defaultId;

    sqlite3_bind_text(stmt, 1, lemma.c_str(), -1, SQLITE_TRANSIENT);

    int existingId = dct::g_defaultId;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existingId = sqlite3_column_int(stmt, 0);
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return existingId;
}

int Database::insertSense(int word_id, const std::string &pos,
                          const std::string &definition) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{
        "INSERT INTO senses (word_id, pos, definition) VALUES (?, ?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    // fill values
    sqlite3_bind_int(stmt, 1, word_id);
    sqlite3_bind_text(stmt, 2, pos.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, definition.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return dct::g_defaultId;
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(m_db.get()));
}

bool Database::insertEtymology(int word_id,
                               const std::vector<std::string> &etymology) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT INTO etymologies (word_id, etymology) VALUES (?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // fill value with vector
    for (const auto &line : etymology) {

        sqlite3_bind_int(stmt, 1, word_id);
        sqlite3_bind_text(stmt, 2, line.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            if (stmt)
                sqlite3_finalize(stmt);
            return false;
        }
        // reuse stmt
        sqlite3_reset(stmt);
    }

    if (stmt)
        sqlite3_finalize(stmt);
    return true;
}

bool Database::insertForm(int word_id, const std::string &form,
                          const std::string &tag) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT INTO forms (word_id, form, tag) VALUES (?, ?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // fill value
    sqlite3_bind_int(stmt, 1, word_id);
    sqlite3_bind_text(stmt, 2, form.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, tag.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (stmt)
            sqlite3_finalize(stmt);
        return false;
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return true;
}

bool Database::insertExample(int sense_id, const std::string &example) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT INTO examples (sense_id, example) VALUES (?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, sense_id);
    sqlite3_bind_text(stmt, 2, example.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (stmt)
            sqlite3_finalize(stmt);
        return false;
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return true;
}

bool Database::insertSynonym(int sense_id, const std::string &synonym) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT INTO synonyms (sense_id, synonym) VALUES (?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, sense_id);
    sqlite3_bind_text(stmt, 2, synonym.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (stmt)
            sqlite3_finalize(stmt);
        return false;
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return true;
}

bool Database::insertAntonym(int sense_id, const std::string &antonym) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"INSERT INTO antonyms (sense_id, antonym) VALUES (?, ?);"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, sense_id);
    sqlite3_bind_text(stmt, 2, antonym.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (stmt)
            sqlite3_finalize(stmt);
        return false;
    }
    if (stmt)
        sqlite3_finalize(stmt);
    return true;
}

bool Database::isEmpty() const {
    sqlite3_stmt *stmt = nullptr;
    const char *sql{"SELECT 1 FROM words LIMIT 1;"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return true; // error = empty

    int rc = sqlite3_step(stmt);
    if (stmt)
        sqlite3_finalize(stmt);

    return (rc != SQLITE_ROW);
}

bool Database::contains(std::string_view word) const {
    if (isEmpty())
        return false;

    sqlite3_stmt *stmt = nullptr;
    const char *sql{"SELECT 1 FROM words WHERE lemma = ? LIMIT 1;"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    if (sqlite3_bind_text(stmt, 1, word.data(), static_cast<int>(word.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        if (stmt)
            sqlite3_finalize(stmt);
        return false;
    }

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    if (stmt)
        sqlite3_finalize(stmt);
    return exists;
}

void Database::clearDB() {
    if (isEmpty())
        return;

    const char *stmts[] = {"PRAGMA foreign_keys = OFF;",
                           "DELETE FROM examples;",
                           "DELETE FROM synonyms;",
                           "DELETE FROM antonyms;",
                           "DELETE FROM senses;",
                           "DELETE FROM forms;",
                           "DELETE FROM etymologies;",
                           "DELETE FROM words;",
                           "DELETE FROM sqlite_sequence;",
                           "PRAGMA foreign_keys = ON;"};

    char *errMsg = nullptr;

    for (auto s : stmts) {
        int rc = sqlite3_exec(m_db.get(), s, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Error clearing database: " << errMsg << "\n";
            sqlite3_free(errMsg);
        }
    }
}

WordInfo Database::getInfo(int word_id) const {
    WordInfo info;
    info.id = word_id;

    sqlite3_stmt *stmt = nullptr;
    const char *wordSql = "SELECT lemma, display_lemma FROM words WHERE id = ?;";
    if (sqlite3_prepare_v2(m_db.get(), wordSql, -1, &stmt, nullptr) ==
        SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, word_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *lemmaTxt =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            const char *displayTxt =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            info.lemma = lemmaTxt ? lemmaTxt : "";
            info.displayLemma = displayTxt ? displayTxt : info.lemma;
        }
    }
    if (stmt)
        sqlite3_finalize(stmt);

    // Etymology
    info.etymology =
        fetchStrings("SELECT etymology FROM etymologies WHERE word_id = ?;", word_id);

    // Forms
    info.forms = fetchForms(word_id);

    // Senses
    const char *senseSql = "SELECT id, pos, definition FROM senses WHERE word_id = ?;";
    sqlite3_stmt *senseStmt = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), senseSql, -1, &senseStmt, nullptr) ==
        SQLITE_OK) {
        sqlite3_bind_int(senseStmt, 1, word_id);
        while (sqlite3_step(senseStmt) == SQLITE_ROW) {
            Sense sense;
            int sense_id = sqlite3_column_int(senseStmt, 0);
            sense.id = sense_id; // preserve DB id for callers
            const char *pos =
                reinterpret_cast<const char *>(sqlite3_column_text(senseStmt, 1));
            const char *def =
                reinterpret_cast<const char *>(sqlite3_column_text(senseStmt, 2));
            sense.pos = pos ? pos : "";
            sense.definition = def ? def : "";

            sense.examples =
                fetchStrings("SELECT example FROM examples WHERE sense_id = ?;", sense_id);
            sense.synonyms =
                fetchStrings("SELECT synonym FROM synonyms WHERE sense_id = ?;", sense_id);
            sense.antonyms =
                fetchStrings("SELECT antonym FROM antonyms WHERE sense_id = ?;", sense_id);

            info.senses.push_back(sense);
        }
    }
    if (senseStmt)
        sqlite3_finalize(senseStmt);

    return info;
}

void Database::streamAllWordsAndForms(
    const WordRecordProcessor &processor) const {
    // NOTE: This assumes m_db is a smart pointer, so we use .get()
    sqlite3 *sqlDB = m_db.get();
    sqlite3_stmt *stmt = nullptr;

    // A more efficient single query to get both words and forms
    const char *query = "SELECT id, lemma FROM words UNION ALL SELECT word_id, "
                        "form FROM forms;";

    if (sqlite3_prepare_v2(sqlDB, query, -1, &stmt, nullptr) != SQLITE_OK) {
        // You should probably log an error here
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *text = sqlite3_column_text(stmt, 1);
        if (text) {
            // Execute the callback with the data from the current row
            processor(id, reinterpret_cast<const char *>(text));
        }
    }

    sqlite3_finalize(stmt);
}

/*********************************
// Helper functions go here
**********************************/

std::unordered_map<int, std::vector<std::string>>
Database::fetchGroupedStrings( // currently unsused
    std::string_view table, std::string_view value_col, // NOLINT(bugprone-easily-swappable-parameters)
    std::string_view id_col, const std::vector<int> &ids) const {
    std::unordered_map<int, std::vector<std::string>> out;
    if (ids.empty())
        return out;

    std::string sql{"SELECT "};
    sql += std::string(id_col) + ", " + std::string(value_col);
    sql += " FROM " + std::string(table) + " WHERE ";
    sql += std::string(id_col) + " IN (";
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i)
            sql += ",";
        sql += "?";
    }
    sql += ");";

    sqlite3_stmt *stmt{nullptr};
    if (sqlite3_prepare_v2(m_db.get(), sql.c_str(), -1, &stmt, nullptr) !=
        SQLITE_OK)
        return out;

    for (size_t i = 0; i < ids.size(); ++i)
        sqlite3_bind_int(stmt, static_cast<int>(i + 1), ids[i]);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *text =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        if (text)
            out[id].push_back(text);
    }

    if (stmt)
        sqlite3_finalize(stmt);
    return out;
}

std::vector<std::string> Database::fetchStrings(std::string_view sql,
                                                int id) const {
    std::vector<std::string> result;
    sqlite3_stmt *stmt{nullptr};

    if (sqlite3_prepare_v2(m_db.get(), sql.data(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *text =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            if (text)
                result.push_back(text);
        }
    }
    if (stmt)
        sqlite3_finalize(stmt);

    return result;
}

std::vector<Form> Database::fetchForms(int word_id) const {
    std::vector<Form> forms;
    sqlite3_stmt *stmt{nullptr};

    const char *sql{"SELECT form, tag FROM forms WHERE word_id = ?;"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, word_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Form f;
            const char *form =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            const char *tag =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            f.form = form ? form : "";
            f.tag = tag ? tag : "";
            forms.push_back(f);
        }
    }
    if (stmt)
        sqlite3_finalize(stmt);

    return forms;
}
