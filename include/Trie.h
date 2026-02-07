#ifndef TRIE_H
#define TRIE_H
#include <string_view>
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include "Utils.h"

class Trie
{
public:
	friend class Tester;
    Trie();
    ~Trie();

    bool insert(std::string_view word, int word_id);
    bool remove(const std::string &word);
    bool contains(std::string_view word) const;
	bool isEmpty() const;

	int getWordId(std::string_view word) const;

	void collectWithPrefix(std::string_view prefix, std::vector<std::string> &out, std::size_t limit) const;
    void dump() const;
	void dumpWord(std::string_view word) const;
    void clear();

	std::string getPrefix(std::string_view word) const;

private:
    struct TrieNode {
        TrieNode *m_children[dct::g_alpha];
        bool m_isEndOfWord{ false };
        int m_wordID{ dct::g_defaultId };

        TrieNode()
        {
            for (int i = 0; i < dct::g_alpha; ++i)
            {
                m_children[i] = nullptr;
            }
        }
    };

    TrieNode *m_root;

    static int indexForChar(char c);

    /*********************************
    // Helper declarations go here
    **********************************/
    bool removeWord(TrieNode *&node, std::string_view word);

    void deleteTrie(TrieNode *node);
    void dumpNode(const TrieNode *node, const std::string &prefix) const;
    void wordsFromNode(const TrieNode *node, std::string &currentWord, std::vector<std::string> &out, std::size_t limit) const;
};
#endif
