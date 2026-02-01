#include "Trie.h"
#include "Utils.h"

Trie::Trie() : m_root{ new TrieNode() } {}

Trie::~Trie() { deleteTrie(m_root); }

bool Trie::insert(std::string_view word, int word_id)
{
    TrieNode *node{ m_root };

    // traverse to the last node in the word
    for (char c : word)
    {
        int index{ c - 'a' }; 

        // check if there is an existing child node
        if (!node->m_children[index]) 
		{
            node->m_children[index] = new TrieNode();
        }
        node = node->m_children[index];
    }

    // check if word is already in Trie (node is last letter of the word)
    if (node->m_isEndOfWord) return false;

    node->m_isEndOfWord = true;
	node->m_wordID = word_id;
    return true;
}

bool Trie::remove(std::string &word) { return removeWord(m_root, word); }

bool Trie::contains(std::string_view word) const
{
    TrieNode *node{ m_root };

    // traverse to the last node in the word (DFS)
    for (char c : word)
    {
        int index{ c - 'a' };
        if (!node->m_children[index]) return false; // not found
        node = node->m_children[index];
    }

    return node->m_isEndOfWord; // word not found
}

std::string Trie::getPrefix(std::string_view word) const
{
	const TrieNode *node{ m_root };
	std::string prefix;

	// DFS
	for (char c : word)
	{
		int index{ c - 'a' };
		if (!node || !node->m_children[index]) break;
		
		prefix.push_back(c);
		node = node->m_children[index];
	}

	return prefix;
}

void Trie::collectWithPrefix(std::string_view prefix, std::vector<std::string> &out, std::size_t limit) const
{
	const TrieNode *node{ m_root };
	std::string currentWord;

	// DFS
	for (char c : prefix)
	{
		int index{ c - 'a' };
		if (!node || !node->m_children[index]) return; // prefix not found

		currentWord.push_back(c);
		node = node->m_children[index];
	}
	
	// build words
	wordsFromNode(node, currentWord, out, limit);
}

void Trie::dump() const
{
	std::cout << "(root)\n";
    dumpNode(m_root, ""); // call recursive dump node function
}

void Trie::dumpWord(std::string_view word) const
{
	if (!contains(word)) return;
	const TrieNode *node{ m_root };
	if (!node) return;

	std::cout << "(root)\n";
	size_t depth{ 0 }; // depth to left of screen

	for (char c : word) 
	{
		int index{ c - 'a' };

		// print graphics
		if (!node->m_children[index])
		{
			std::cout << std::string(depth * 4, ' ') 
				<< "└── " << c << "(missing)\n";
			return;
		}

		node = node->m_children[index];

		std::cout << std::string(depth * 4, ' ') 
			<< "└── " << c; 
		if (node->m_isEndOfWord) std::cout << " *";

		std::cout << '\n';
		++depth;
	}
}

void Trie::clear()
{
	deleteTrie(m_root);
	m_root = new TrieNode(); // initalize new root
}

bool Trie::isEmpty() const
{
	if (!m_root) return true;
	if (m_root->m_isEndOfWord) return false;

	for (int i{0}; i < dct::g_alpha; ++i)
	{
		if (m_root->m_children[i]) return false;
	}

	return true;
}

/*********************************
// Trie Helper Functions
*********************************/
void Trie::deleteTrie(TrieNode *node)
{
	if (!node) return;
	
	// DFS
	for (int i{0}; i < dct::g_alpha; ++i)
	{
		deleteTrie(node->m_children[i]);
	}
	
	delete node;
}

void Trie::dumpNode(const TrieNode *node, const std::string &prefix) const
{ 
	if (!node) return; 
	
	// DFS
	for (int i{0}; i < dct::g_alpha; ++i)
	{
		char letter{ static_cast<char>('a' + i) };
		const TrieNode *child = node->m_children[i];
		if (!child) continue;
		bool isLast = true;
		
		// check for another sibling
		for (int j{i+1}; j < dct::g_alpha; ++j)
		{
			// check for later children
			if (node->m_children[j])
			{
				// current child is not the last sibling
				isLast = false;
				break; 
			}
		}

		// print graphics
		std::cout << prefix << (isLast ? "└── " : "├── ") << letter;
		if (child->m_isEndOfWord) std::cout << " *"; 
		std::cout << '\n';

		dumpNode(node->m_children[i], prefix + (isLast ? "    " : "│   "));	
	}
}

bool Trie::removeWord(TrieNode *&node, std::string_view word)
{
	if (!node) return false;

	// check if end of word (words because of word.substr(1) on line 267) 	
	if (word.empty())
	{
		if (!node->m_isEndOfWord) return false; // word is not stored
	
		node->m_isEndOfWord = false;

		// check if the node has children
		for (int i{0}; i < dct::g_alpha; ++i)
		{
			// if the node has children it's still needed for another word
			if (node->m_children[i]) return true;
		}

		// if it has no children we can safely remove the node
		delete node;
		node = nullptr;
		return true;
	}
	
	// find child index
	int index{ word[0] - 'a' };
	if (removeWord(node->m_children[index], word.substr(1))) // recursively remove the rest of the word
	{
		if (node->m_isEndOfWord) return true; // if the current node marks the end of another word, preserve it

		// check if node has any children and is still needed
		for (int i{0}; i < dct::g_alpha; ++i)
		{
			if (node->m_children[i]) return true;
		}
		
		// if the node is not the end of a word and has no children
		delete node;
		node = nullptr;
		return true;
	}
	
	return false;
}

void Trie::wordsFromNode(const TrieNode *node, std::string &currentWord, std::vector<std::string> &out, std::size_t limit) const
{ // similar to rewrite
	if (!node || out.size() >= limit) return;
	if (node->m_isEndOfWord) out.push_back(currentWord); // add complete word to results vector

	for (int i{0}; i < dct::g_alpha && out.size() < limit; ++i)
	{
		if (node->m_children[i])
		{
			char letter{ static_cast<char>('a' + i) };
			currentWord.push_back(letter); // build word
			wordsFromNode(node->m_children[i], currentWord, out, limit);
			currentWord.pop_back(); // backtrack (undo complete word) works because of recursive collectFromNode 
		}
	}	
}
