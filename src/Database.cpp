#include "Database.h"

Database::Database(const std::string& filename) 
{ 
	// open database
	if (sqlite3_open(filename.c_str(), &db)) throw std::runtime_error("Error: Can't open database\n");
}

Database::~Database() { sqlite3_close(db); }

sqlite3* Database::getDB() { return db; }

void Database::createTables() 
{
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

    const char* sql 
	{
        "CREATE TABLE IF NOT EXISTS words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "lemma TEXT UNIQUE);"
	    
	    "CREATE TABLE IF NOT EXISTS etymologies ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "etymology TEXT NOT NULL,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);"
        
	    "CREATE TABLE IF NOT EXISTS forms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "form TEXT NOT NULL,"
	    "tag TEXT,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);"     
        
	    "CREATE TABLE IF NOT EXISTS senses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word_id INTEGER NOT NULL,"
        "pos TEXT,"
        "definition TEXT NOT NULL,"
        "FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE);"
	    	
	    "CREATE TABLE IF NOT EXISTS examples ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "example TEXT NOT NULL,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);"
	    
        "CREATE TABLE IF NOT EXISTS synonyms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "synonym TEXT,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);"
        
	    "CREATE TABLE IF NOT EXISTS antonyms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sense_id INTEGER NOT NULL,"
        "antonyms TEXT,"
        "FOREIGN KEY(sense_id) REFERENCES senses(id) ON DELETE CASCADE);"
	};

    char* errMsg{ nullptr };
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
}

int Database::insertWord(const std::string& lemma) 
{
    sqlite3_stmt* stmt; // prepared SQL statement object
    const char* sql{ "INSERT OR IGNORE INTO words (lemma) VALUES (?);" }; // add new row if word does not exist, otherwise ignore

	/* 
	db → your open database
	sql → the SQL string
	-1 → read full string
	&stmt → where to store the compiled statement
	nullptr → ignore unused output
	*/	
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

	/*
	stmt → prepared statement
	1 → first `?` in SQL
	lemma.c_str() → actual word
	-1 → length (auto)
	SQLITE_STATIC → SQLite can safely reference this memory
	*/
    sqlite3_bind_text(stmt, 1, lemma.c_str(), -1, SQLITE_TRANSIENT);
	
	// run the statement
	if (sqlite3_step(stmt) != SQLITE_DONE) 
	{ 
		sqlite3_finalize(stmt); // free the statement from memory to avoid leaks
		return -1;
	}
	sqlite3_finalize(stmt); // free the statement from memory to avoid leaks
    return getWordID(lemma); // word inserted successfully
}

int Database::insertSense(int word_id, const std::string& pos, const std::string& definition) 
{
    sqlite3_stmt* stmt;
    const char* sql{ "INSERT INTO senses (word_id, pos, definition) VALUES (?, ?, ?);" };
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

	// fill values
    sqlite3_bind_int(stmt, 1, word_id);
    sqlite3_bind_text(stmt, 2, pos.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, definition.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{  
        sqlite3_finalize(stmt);
        return -1;	
	}
    sqlite3_finalize(stmt);
	return static_cast<int>(sqlite3_last_insert_rowid(db));
}

bool Database::insertEtymology(int word_id, const std::vector<std::string> &etymology)
{ 
	sqlite3_stmt* stmt;
	const char* sql{ "INSERT INTO etymologies (word_id, etymology) VALUES (?, ?);" };
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

	// fill value with vector
	for (const auto& line : etymology)
	{

	    sqlite3_bind_int(stmt, 1, word_id);
	    sqlite3_bind_text(stmt, 2, line.c_str(), -1, SQLITE_TRANSIENT);
	    
	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
	        sqlite3_finalize(stmt);	
            return false;
	    }
	    // reuse stmt
	    sqlite3_reset(stmt);
	}
	
	sqlite3_finalize(stmt);	
	return true;
}	

bool Database::insertForm(int word_id, const std::string &form, const std::string &tag)
{ 
	sqlite3_stmt* stmt;
	const char* sql{ "INSERT INTO forms (word_id, form, tag) VALUES (?, ?, ?);" };
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

	// fill value
	sqlite3_bind_int(stmt, 1, word_id);
	sqlite3_bind_text(stmt, 2, form.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, tag.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
	    sqlite3_finalize(stmt);	
        return false;
	}
	sqlite3_finalize(stmt);	
	return true;
}

bool Database::insertExample(int word_id, const std::string &example)
{ 
	sqlite3_stmt* stmt;
	const char* sql{ "INSERT INTO examples (word_id, example) VALUES (?, ?);" };
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

	// fill value
	sqlite3_bind_int(stmt, 1, word_id);
	sqlite3_bind_text(stmt, 2, example.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
	    sqlite3_finalize(stmt);	
        return false;
	}
	sqlite3_finalize(stmt);	
	return true;
}

bool Database::insertSynonym(int word_id, const std::string &synonym)
{ 
	sqlite3_stmt* stmt;
	const char* sql{ "INSERT INTO synonyms (word_id, synonym) VALUES (?, ?);" };
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

	// fill value
	sqlite3_bind_int(stmt, 1, word_id);
	sqlite3_bind_text(stmt, 2, synonym.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
	    sqlite3_finalize(stmt);	
        return false;
	}
	sqlite3_finalize(stmt);	
	return true;
}

bool Database::insertAntonym(int word_id, const std::string &antonym)
{ 
	sqlite3_stmt* stmt;
	const char* sql{ "INSERT INTO antonyms (word_id, antonym) VALUES (?, ?);" };
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

	// fill value
	sqlite3_bind_int(stmt, 1, word_id);
	sqlite3_bind_text(stmt, 2, antonym.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
	    sqlite3_finalize(stmt);	
        return false;
	}
	sqlite3_finalize(stmt);	
	return true;
}

bool Database::isEmpty() const
{ // implement
	return false;
}

bool Database::removeWord(int word_id)
{ // implement
	if (isEmpty()) return false;
	
	return false;
}

int Database::getWordID(const std::string &lemma) const
{
	sqlite3_stmt* stmt;
	const char* sql {"SELECT id FROM words WHERE lemma = ?;"};

	// prepare the sql statement
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

	// bind the word into the ?
	sqlite3_bind_text(stmt, 1, lemma.c_str(), -1, SQLITE_STATIC);

	// execute the query
	int word_id = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		word_id = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return word_id;
}

void Database::clearDB()
{
	if (isEmpty()) return;

	const char* sql 
	{
        "PRAGMA foreign_keys = OFF;"
        "DELETE FROM examples;"
        "DELETE FROM synonyms;"
        "DELETE FROM antonyms;"
        "DELETE FROM senses;"
        "DELETE FROM forms;"
        "DELETE FROM etymology;"
        "DELETE FROM words;"
        "DELETE FROM sqlite_sequence;"
        "PRAGMA foreign_keys = ON;"
	};

	char* errMsg{ nullptr };
    int rc{ sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) };

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error clearing database: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return;
    }
}

void Database::dumpWord() const
{

}
