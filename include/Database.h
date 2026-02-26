#ifndef DATABASE_H 
#define DATABASE_H
#include "WordInfo.h"
#include <sqlite3.h>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <string>
#include <iostream>

// This class acts as a wrapper around a C library
class Database
{
public:
	friend class Tester;
	Database(std::string_view filename);
	~Database();

	sqlite3 *getDB();

	bool insertEtymology(int word_id, const std::vector<std::string> &etymology);
	bool insertForm(int word_id, const std::string &form, const std::string &tag);
	bool insertExample(int sense_id, const std::string &example);
	bool insertSynonym(int sense_id, const std::string &synonym);
	bool insertAntonym(int sense_id, const std::string &antonym);
	bool isEmpty() const;
	bool contains(std::string_view word) const;
		
	int insertWord(const std::string &lemma);
	int insertSense(int word_id, const std::string &pos, const std::string &definition);

	void createTables();
	void clearDB();

	WordInfo getInfo(int word_id) const;

	/* implement??
	bool addWord(string_view word);	
	bool removeWord(int word_id); 
	*/

private:
	sqlite3 *m_db;
	
	/*********************************
    // Helper declarations go here
    **********************************/
	std::vector<std::string> fetchStrings(std::string_view sql, int id) const; // currently unsused
	std::vector<Form> fetchForms(int word_id) const;	
	std::unordered_map<int, std::vector<std::string>> fetchGroupedStrings( 
	    std::string_view table, 
	    std::string_view value_col, 
	    std::string_view id_col, 
	    const std::vector<int> &ids) const;

};
#endif 
