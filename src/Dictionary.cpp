#include "Dictionary.h"
#include "Utils.h"

Dictionary::Dictionary() : m_db{dct::g_dictDb}
{
	m_db.createTables();	
	buildTrie(m_db);  
}

bool Dictionary::addWord(std::string_view word)
{ // fix logic
	std::string cleanWord{ normalize(word) };
	if (cleanWord.empty()) return false;

	// IMPLEMENT 
	
	return true;	
}

// Dictionary 
bool Dictionary::removeWord(std::string_view word)
{ 
	if (m_trie.isEmpty()) return false;
	
	// IMPLEMENT 
	std::string cleanWord{ normalize(word) };
	if (cleanWord.empty()) return false;

	if (!m_trie.remove(cleanWord)) return false;

	return true;
}

bool Dictionary::loadInfo(const std::string &filename)
{
	/*
	   compatible file type:
        .csv
        .tsv
        .json
        .xml
	*/
	// parse filename
	std::size_t p {filename.find_last_of('.')};
	std::string extension{ "" };
	bool success{ false };

	if (p == std::string::npos) std::cerr << "Error: file has no extension!" << std::endl;
	else extension = filename.substr(p);

	// temporary
	if (extension == ".json")
	{
	    success = loadjson(filename);
	} else
	{
		std::cerr << "Error: file extension not recognized." << std::endl;
	}

#if 0
	// control file calls
	if (extension == ".csv") 
		opencsv(filename); // .csv
	
	else if (extension == ".tsv") 
		opentsv(filename); // .tsv
	
	else if (extension == ".json") 
		openjson(filename); // .json
	
	else if (extension == ".xml") 
		openxml(filename); // .xml
	
	else std::cout << "Error: file extension not recognized." << std::endl;
#endif

	return success;
}

void Dictionary::suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const 
{
	std::string cleanPrefix{ normalize(prefix) };
	if (cleanPrefix.empty()) return;
	m_trie.collectWithPrefix(cleanPrefix, results, limit+1); // LOGIC ERROR: (off by 1) - balance the prefix being removed from the results

	// remove the prefix from the results vector
	results.erase(std::remove(results.begin(), results.end(), prefix), results.end());
}

// Trie wrapper
bool Dictionary::trieContainsWord(std::string_view word) const
{
	if (m_trie.isEmpty()) return false;

	std::string cleanWord{ normalize(word) };
	if (cleanWord.empty()) return false;

	return m_trie.contains(cleanWord);
}

bool Dictionary::isTrieEmpty() const { return m_trie.isEmpty(); }

void Dictionary::printTrie() const { m_trie.print(); } 

void Dictionary::dumpTrie() const { m_trie.dump(); }

void Dictionary::dumpTrieWord(std::string_view word) const { m_trie.dumpWord(word); }

void Dictionary::clearTrie() { m_trie.clear(); }

// Database wrapper
void Dictionary::clearDB() { m_db.clearDB(); }

bool Dictionary::isDBEmpty() const { return m_db.isEmpty(); }

/*********************************
// Dictionary Helper Functions
**********************************/
std::string Dictionary::normalize(std::string_view word) const 
{
	std::string cleanWord{ "" };
	for (char c : word)
	{
		if (!isalpha(static_cast<unsigned char>(c))) continue;
		{
		    // tolower() returns int, passing a negative value = undefined behavior
		    cleanWord.push_back(tolower(static_cast<unsigned char>(c))); 
		}	
	}
	return cleanWord;
}

void Dictionary::buildTrie(Database &db) 
{
    sqlite3* sqlDB{ m_db.getDB() };
    sqlite3_stmt* stmt;

	// insert all lemmas
    const char* q1 = "SELECT id, lemma FROM words;";
    sqlite3_prepare_v2(sqlDB, q1, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) 
	{
	    int id{ sqlite3_column_int(stmt, 0) };
        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        std::string lemma{ reinterpret_cast<const char*>(text) };
        m_trie.insert(lemma, id);  
	}

	// insert all forms
    const char* q2 = "SELECT word_id, form FROM forms;";
    sqlite3_prepare_v2(sqlDB, q2, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id{ sqlite3_column_int(stmt, 0) };
        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        std::string lemma{ reinterpret_cast<const char*>(text) };
        std::string form = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m_trie.insert(form, id);
    }
    sqlite3_finalize(stmt);
}

bool Dictionary::loadjson(const std::string &filename)
{	
	std::ifstream file(filename);
	if (!file) throw std::runtime_error("Error: Cannot open external dictionary.\n");

	std::string line;
    while (std::getline(file, line))
    {
        try
        {
            nlohmann::json j = nlohmann::json::parse(line);

            // skip entries without a word
            if (!j.contains("word")) continue;

			// retrieve json word data into "word" and insert into DB
            WordInfo word;
            word.lemma = j["word"];

            // insert and get word ID from database
            word.id = m_db.insertWord((word.lemma));

            // Etymology
            if (j.contains("etymology_text"))
            {
                std::string etymology = j["etymology_text"];
                
				// split by lines into vector
                std::istringstream ety_stream(etymology);
                std::string ety_line;
                while (std::getline(ety_stream, ety_line))
                    word.etymology.push_back(ety_line); // build the etymology vector

                m_db.insertEtymology(word.id, word.etymology);
            }

            // Forms (plurals, alt spellings)
            if (j.contains("forms"))
            {
                for (auto &f : j["forms"])
                {
                    WordInfo::Form form;
                    
					// Form
                    form.form = f.value("form", "");

					// Tag (if there is one)
                    form.tag = f.contains("tags") && !f["tags"].empty() ? f["tags"][0].get<std::string>() : "";
                    word.forms.push_back(form); // vector of Forms for autocomplete / sepllchecking

                    m_db.insertForm(word.id, form.form, form.tag);
                }
            }

            // Senses
            if (j.contains("senses"))
            {
                for (auto &sense_json : j["senses"])
                {
                    WordInfo::Sense sense;

                    // POS (part of speech)
                    sense.pos = sense_json.value("pos", j.value("pos", ""));

                    // Definitions / glosses
                    if (sense_json.contains("glosses"))
                    {
                        for (auto &g : sense_json["glosses"])
                            sense.definition += g.get<std::string>() + " "; // concatenate multiple glosses
                    }

                    // Examples
                    if (sense_json.contains("examples"))
                    {
                        for (auto &ex : sense_json["examples"])
                            sense.examples.push_back(ex.get<std::string>()); // build examples vector
                    }

                    // Synonyms
                    if (sense_json.contains("synonyms"))
                    {
                        for (auto &syn : sense_json["synonyms"])
                            sense.synonyms.push_back(syn.get<std::string>()); // build synonyms vector
                    }

                    // Antonyms
                    if (sense_json.contains("antonyms"))
                    {
                        for (auto &ant : sense_json["antonyms"])
                            sense.antonyms.push_back(ant.get<std::string>()); // build antonyms vector
                    }

                    word.senses.push_back(sense); // vector of senses for potentinal quick lookups

                    // Insert into DB
                    int sense_id{ m_db.insertSense(word.id, sense.pos, sense.definition) };

                    for (const auto &ex : sense.examples)
                        m_db.insertExample(sense_id, ex);

                    for (const auto &syn : sense.synonyms)
                        m_db.insertSynonym(sense_id, syn);

                    for (const auto &ant : sense.antonyms)
                        m_db.insertAntonym(sense_id, ant);
                }
            }

			//m_words[word.lemma] = word; // add to Dictionary storage??? (map<string, WordInfo>)
        }
        catch (const std::exception& e)
        {
            std::cerr << "Skipping bad line: " << e.what() << "\n";
        }
    }

	// build the trie
	buildTrie(m_db);

    return true;
}

bool Dictionary::search(std::string_view word) const
{
	return false;
}
