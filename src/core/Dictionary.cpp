#include "Dictionary.h"
#include "Utils.h"

namespace
{
    std::vector<std::string> extractJsonTextList(const nlohmann::json& arrayField, std::string_view preferredKey)
    {
        std::vector<std::string> values;
        if (!arrayField.is_array()) return values;

        for (const auto& item : arrayField)
        {
            std::string value;

            if (item.is_string())
            {
                value = item.get<std::string>();
            }
            else if (item.is_object())
            {
                auto takeIfString = [&](const char* key) -> bool
                {
                    auto it = item.find(key);
                    if (it != item.end() && it->is_string())
                    {
                        value = it->get<std::string>();
                        return true;
                    }
                    return false;
                };

                takeIfString(preferredKey.data());
                if (value.empty()) takeIfString("text");
                if (value.empty()) takeIfString("sense");
                if (value.empty()) takeIfString("translation");
            }

            if (!value.empty()) values.push_back(value);
        }

        return values;
    }
}

Dictionary::Dictionary() : m_db{ dct::g_dictDb }
{
    m_db.createTables();
    loadTrie(m_db);
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
	bool success{ true };

	if (p == std::string::npos)
	{
	    std::cerr << "Error: file has no extension!" << std::endl;
	    return false;
	}

	extension = filename.substr(p);

	if (extension == ".json")
	{
	    success = success && loadjson(filename);
	}
	else
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

WordInfo Dictionary::getWordInfo(std::string_view word) const
{
    WordInfo info;
    std::string clean = cleanWord(word);
    if (clean.empty()) return info;

    int id = m_trie.getWordId(clean);
    if (id == dct::g_defaultId) return info;

    auto cached = m_cache.find(id);
    if (cached != m_cache.end())
    {
        return cached->second;
    }

    info = m_db.getInfo(id);
    m_cache.try_emplace(id, info);

    return info;
}

bool Dictionary::contains(std::string_view word) const
{
    std::string clean = cleanWord(word);
    if (clean.empty()) return false;

    int id = m_trie.getWordId(clean);
    if (id == dct::g_defaultId) return false;

    return m_cache.find(id) != m_cache.end();
}

void Dictionary::suggestFromPrefix(std::string_view prefix, std::vector<std::string> &results, std::size_t limit) const 
{
	if (m_trie.isEmpty()) return;
	std::string cleanPrefix{ cleanWord(prefix) };
	if (cleanPrefix.empty()) return;
	m_trie.collectWithPrefix(cleanPrefix, results, limit);

	results.erase(std::remove(results.begin(), results.end(), cleanPrefix), results.end());
}

/*********************************
// Dictionary Helper Functions
**********************************/
// temp
void Dictionary::printInfo(const WordInfo &word) const
{
	std::cout << "Word: " << word.lemma << std::endl;
    if (!word.etymology.empty())
    {
        std::cout << "Etymology:" << std::endl;
        for (const auto &line : word.etymology)
            std::cout << "  " << line << std::endl;
    }

    if (!word.forms.empty())
    {
        std::cout << "Forms:" << std::endl;
        for (const auto &form : word.forms)
            std::cout << "  " << form.form << " (" << form.tag << ")" << std::endl;
    }

    if (!word.senses.empty())
    {
        std::cout << "Senses:" << std::endl;
        for (const auto &sense : word.senses)
        {
            std::cout << "  - POS: " << sense.pos << std::endl;
            std::cout << "    Definition: " << sense.definition << std::endl;

            if (!sense.examples.empty())
            {
                std::cout << "    Examples:" << std::endl;
                for (const auto &ex : sense.examples)
                    std::cout << "      - " << ex << std::endl;
            }

            if (!sense.synonyms.empty())
            {
                std::cout << "    Synonyms:" << std::endl;
                for (const auto &syn : sense.synonyms)
                    std::cout << "      - " << syn << std::endl;
            }

            if (!sense.antonyms.empty())
            {
                std::cout << "    Antonyms:" << std::endl;
                for (const auto &ant : sense.antonyms)
                    std::cout << "      - " << ant << std::endl;
            }
        }
    }	
}

std::string Dictionary::cleanWord(std::string_view word) const { return dct::sanitizeWord(word); }

void Dictionary::loadTrie(Database &db) 
{
    sqlite3* sqlDB{ m_db.getDB() };
    sqlite3_stmt* stmt{ nullptr };

    // insert all lemmas
    const char* q1 = "SELECT id, lemma FROM words;";
    if (sqlite3_prepare_v2(sqlDB, q1, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id{ sqlite3_column_int(stmt, 0) };

        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        if (!text) continue;

        std::string lemma{ reinterpret_cast<const char*>(text) };
        m_trie.insert(cleanWord(lemma), id);
    }

    sqlite3_finalize(stmt); // finalize before next preparation


    // insert all forms
    const char* q2 = "SELECT word_id, form FROM forms;";
    if (sqlite3_prepare_v2(sqlDB, q2, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id{ sqlite3_column_int(stmt, 0) };

        const unsigned char* text{ sqlite3_column_text(stmt, 1) };
        if (!text) continue;

        std::string form{ reinterpret_cast<const char*>(text) };
        m_trie.insert(cleanWord(form), id);
    }

    sqlite3_finalize(stmt);
}

bool Dictionary::loadjson(const std::string &filename)
{	
	std::ifstream file(filename);
	if (!file)
    {
        throw std::runtime_error("Error: Cannot open external dictionary.\n");
    }

	std::string line;
    sqlite3* dbHandle = m_db.getDB();
    char* err = nullptr;
    if (sqlite3_exec(dbHandle, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
    {
        std::cerr << "Failed to start transaction: " << (err ? err : "?") << '\n';
        sqlite3_free(err);
        err = nullptr;
    }

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

            auto cleanLemma = cleanWord(word.lemma);
            if (cleanLemma.empty()) continue;

            word.id = m_db.insertWord(cleanLemma);

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
                    Form form;
                    
					// Form
                    form.form = cleanWord(f.value("form", "")); // clean word for Trie lookup
                    if (form.form.empty()) continue;

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
                    Sense sense;

                    // POS (part of speech)
                    sense.pos = sense_json.value("pos", j.value("pos", ""));

                    // Definitions / glosses
                    if (sense_json.contains("glosses"))
                    {
                        for (auto &g : sense_json["glosses"])
                            sense.definition += g.get<std::string>() + " "; // concatenate multiple glosses
                    }

					// These fields can be string arrays or object arrays in Kaikki dumps.
					if (sense_json.contains("examples"))
                        sense.examples = extractJsonTextList(sense_json["examples"], "text");

                    if (sense_json.contains("synonyms"))
                        sense.synonyms = extractJsonTextList(sense_json["synonyms"], "word");

                    if (sense_json.contains("antonyms"))
                        sense.antonyms = extractJsonTextList(sense_json["antonyms"], "word");

                    word.senses.push_back(sense); // vector of senses for potentinal quick lookups

                    int sense_id{ m_db.insertSense(word.id, sense.pos, sense.definition) };

                    if (sense_id == dct::g_defaultId)
                    {
                        std::cerr << "Failed to insert sense for word: " << word.lemma << '\n';
                        continue;
                    }

                    for (const auto &ex : sense.examples)
                        m_db.insertExample(sense_id, ex);

                    for (const auto &syn : sense.synonyms)
                        m_db.insertSynonym(sense_id, syn);

                    for (const auto &ant : sense.antonyms)
                        m_db.insertAntonym(sense_id, ant);
                }
            }

			//m_cache[word.lemma] = word; // add to Dictionary storage??? (map<string, WordInfo>)
        }
        catch (const std::exception& e)
        {
            std::cerr << "Skipping bad line: " << e.what() << "\n";
        }
    }

    if (sqlite3_exec(dbHandle, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
    {
        std::cerr << "Failed to commit transaction: " << (err ? err : "?") << '\n';
        sqlite3_free(err);
        err = nullptr;
    }

    return true;
}
