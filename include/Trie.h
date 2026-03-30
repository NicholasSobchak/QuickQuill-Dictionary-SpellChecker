#ifndef TRIE_H
#define TRIE_H
#include "Utils.h"
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Trie 
{
public:
    Trie();
    ~Trie() = default;

    bool insert(std::string_view word, int word_id);
    bool remove(const std::string &word);
    bool contains(std::string_view word) const;
    bool isEmpty() const;

    int getWordId(std::string_view word) const;

    void collectWithPrefix(std::string_view prefix,
                           std::vector<std::string> &out, std::size_t limit) const;
    void dump() const;
    void dumpWord(std::string_view word) const;
    void clear();

    std::string getPrefix(std::string_view word) const;

private:
    struct TrieNode {
        std::array<std::unique_ptr<TrieNode>, dct::g_alpha> m_children;
        bool m_isEndOfWord{false};
        int m_wordID{dct::g_defaultId};
    };

    std::unique_ptr<TrieNode> m_root;

    static int indexForChar(char c);

    /*********************************
     * Helper declarations go here
     **********************************/
    // Returns true if the node should be deleted by its parent.
    bool removeWord(TrieNode *node, std::string_view word);
    void dumpNode(const TrieNode *node, const std::string &prefix) const;
    void wordsFromNode(const TrieNode *node, std::string &currentWord,
                       std::vector<std::string> &out, std::size_t limit) const;
};
#endif
