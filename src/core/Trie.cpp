#include "Trie.h"

int Trie::indexForChar(char c)
{
    unsigned char lowercase = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
    if (lowercase < 'a' || lowercase > 'z') return -1; // all non-alpha will return -1
    return lowercase - 'a';
}

Trie::Trie() : m_root{ new TrieNode() } {}

Trie::~Trie() { deleteTrie(m_root); }

bool Trie::insert(std::string_view word, int word_id)
{
    if (word.empty()) return false;

    TrieNode *node{ m_root };

    for (char c : word)
    {
        int index = indexForChar(c);
        if (index < 0) return false; 

        if (!node->m_children[index])
        {
            node->m_children[index] = new TrieNode();
        }
        node = node->m_children[index];
    }

    if (node->m_isEndOfWord) return false;

    node->m_isEndOfWord = true;
    node->m_wordID = word_id;
    return true;
}

bool Trie::remove(const std::string &word)
{
    bool removed = removeWord(m_root, word);
    if (!m_root) m_root = new TrieNode(); // recreate root just in case if its deleted
    return removed;
}

bool Trie::contains(std::string_view word) const
{
    if (word.empty()) return false;

    TrieNode *node{ m_root };

    for (char c : word)
    {
        int index = indexForChar(c);
        if (index < 0 || !node->m_children[index]) return false;
        node = node->m_children[index];
    }

    return node->m_isEndOfWord;
}

std::string Trie::getPrefix(std::string_view word) const
{
	const TrieNode *node{ m_root };
	std::string prefix;

	for (char c : word)
	{
		int index = indexForChar(c);
		if (index < 0 || !node || !node->m_children[index]) break;
		
		prefix.push_back(static_cast<char>('a' + index));
		node = node->m_children[index];
	}

	return prefix;
}

int Trie::getWordId(std::string_view word) const
{
    if (word.empty()) return dct::g_defaultId;

    TrieNode *node{ m_root };

    for (char c : word)
    {
        int index = indexForChar(c);
        if (index < 0 || !node->m_children[index]) return dct::g_defaultId;
        node = node->m_children[index];
    }

    if (node->m_isEndOfWord) return node->m_wordID;
    return dct::g_defaultId;
}

void Trie::collectWithPrefix(std::string_view prefix, std::vector<std::string> &out, std::size_t limit) const
{
	if (prefix.empty() || limit == 0) return; // collect nothing
	const TrieNode *node{ m_root };
	std::string currentWord;

	// DFS
	for (char c : prefix)
	{
		int index = indexForChar(c);
		if (index < 0 || !node || !node->m_children[index]) return; // prefix not found

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
		int index = indexForChar(c);

		if (index < 0 || !node->m_children[index])
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
		node->m_children[i] = nullptr;
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

    if (word.empty())
    {
        if (!node->m_isEndOfWord) return false;

        node->m_isEndOfWord = false;
        node->m_wordID = dct::g_defaultId;

        for (int i{0}; i < dct::g_alpha; ++i)
        {
            if (node->m_children[i]) return true;
        }

        delete node;
        node = nullptr;
        return true;
    }

    int index = indexForChar(word.front());
    if (index < 0) return false;

    if (removeWord(node->m_children[index], word.substr(1)))
    {
        if (node->m_isEndOfWord) return true;

        for (int i{0}; i < dct::g_alpha; ++i)
        {
            if (node->m_children[i]) return true;
        }

        delete node;
        node = nullptr;
        return true;
    }

    return false;
}

void Trie::wordsFromNode(const TrieNode *node, std::string &currentWord, std::vector<std::string> &out, std::size_t limit) const
{
    if (!node || out.size() >= limit) return;

    if (node->m_isEndOfWord)
    {
        out.push_back(currentWord);
        if (out.size() >= limit) return;
    }

    for (int i{0}; i < dct::g_alpha && out.size() < limit; ++i)
    {
        if (node->m_children[i])
        {
            char letter{ static_cast<char>('a' + i) };
            currentWord.push_back(letter);
            wordsFromNode(node->m_children[i], currentWord, out, limit);
            currentWord.pop_back();
        }
    }
}
