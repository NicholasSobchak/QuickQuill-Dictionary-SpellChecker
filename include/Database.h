#ifndef DATABASE_H 
#define DATABASE_H
#include "WordInfo.h"
#include <sqlite3.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <functional>
#include <string>
#include <iostream>

// This class acts as a wrapper around a C library
class Database
{
public:
	using WordRecordProcessor = std::function<void(dct::WordId id, std::string_view text, dct::Frequency frequency)>;

	Database(std::string_view filename);
	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;
	Database(Database&&) noexcept = default;
	Database& operator=(Database&&) noexcept = default;
	~Database() = default;

	bool insertEtymology(dct::WordId word_id, const std::vector<std::string> &etymology);
	bool insertForm(dct::WordId word_id, const std::string &form, const std::string &tag);
	bool insertExample(dct::WordId sense_id, const std::string &example);
	bool insertSynonym(dct::WordId sense_id, const std::string &synonym);
	bool insertAntonym(dct::WordId sense_id, const std::string &antonym);
	bool isEmpty() const;
	bool contains(std::string_view word) const;
		
	dct::WordId insertWord(const std::string &lemma, const std::string &displayLemma, dct::Frequency frequency);
	dct::WordId insertSense(dct::WordId word_id, const std::string &pos, const std::string &definition);

	void createTables();
	void clearDB();
	void streamAllWordsAndForms(const WordRecordProcessor& processor) const;

	WordInfo getInfo(dct::WordId word_id) const;

private:
	struct Sqlite3Deleter
	{
		void operator()(sqlite3* db) const
		{
			if (db)
			{
				sqlite3_close(db);
			}
		}
	};

	std::unique_ptr<sqlite3, Sqlite3Deleter> m_db;
	
	/*********************************
    // Helper declarations go here
    **********************************/
	std::vector<std::string> fetchStrings(std::string_view sql, dct::WordId id) const; // currently unsused
	std::vector<Form> fetchForms(dct::WordId word_id) const;	
	std::unordered_map<int, std::vector<std::string>> fetchGroupedStrings( 
	    std::string_view table, 
	    std::string_view value_col, 
	    std::string_view id_col, 
	    const std::vector<dct::WordId> &ids) const;

};
#endif 
