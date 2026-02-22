// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <iostream>

using json = nlohmann::json;

class Tester
{
public:
	Tester(Dictionary& d) 
		: dict_{ d }, checker_{ dict_ } {}

	void testDump() { dict_.m_trie.dump(); }
	void testDumpWord(std::string_view word) { dict_.m_trie.dumpWord(word); }
	void testGetWordInfo(std::string_view word) 
	{ 
		WordInfo info = dict_.getWordInfo(word);
		dict_.printInfo(info);
	}	


private:
	Dictionary& dict_;
	SpellChecker checker_;
};
int main()
{
    Dictionary dict;
    SpellChecker checker(dict);
	// Tester test(dict);

    std::cout << "Hosting Dictionary Spell Checker...\n";

	// basic HTTP TCP server build
#if 0
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // create the server socket
														// AF_INET: IPv4 protocol
														// SOCK_STREAM: TCP socket

	sockaddr_in serverAddress; //{} sockaddr_in: datatype used to store the address of the socket
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080); // htons: used to convert unsigned int from machine byte order to netword byte order
	serverAddress.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY: don't want to bind socket to any particular IP address, listens to all available IPs

	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)); // bind socket to address
	listen(serverSocket, 5); // listen for incoming connections

	while (true)
	{
		int clientSocket = accept(serverSocket, nullptr, nullptr); // accept client connection

		// revieve data from client
		char buffer[2048] = {0};
		int n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (n <= 0) { close(clientSocket); continue; }

		// read request
		std::string request(buffer, n);
		std::istringstream iss(request);
		std::string method, target;
		iss >> method >> target;

		// find query
		std::string word;
		auto qpos = target.find("?word=");
		if (qpos != std::string::npos) word = target.substr(qpos + 6);

		json body;
		int status = 200;
		std::string statusText = "OK";

		if (method != "GET" || target.rfind("/lookup", 0) != 0 || word.empty())
		{
			status = 400;
			statusText = "Bad Request";
			body = {{"error", "use /lookup?word=..."}};
		}
		else
		{
			WordInfo info = dict.getWordInfo(word);
			if (info.lemma.empty())
			{
				status = 404;
				statusText = "Not Found";
				body = {{"error", "word not found"}};
			}
			else
			{
				body = {{"lemma", info.lemma}, {"id", info.id}};
			}
		}

		
		std::string out = body.dump();
		std::ostringstream response;
		response << "HTTP/1.1 " << status << " " << statusText << "\r\n";
		response << "Content-Type: application/json\r\n";
		response << "Content-Length: " << out.size() << "\r\n";
		response << "Connection: close\r\n\r\n";
		response << out;
		
		std::cout << response.str() << std::endl; // debugging
		
		// return body and status
		auto s = response.str();
		send(clientSocket, s.c_str(), s.size(), 0);
		close(clientSocket);
	}

	close(serverSocket); // close server socket
#endif

	// terminal build
#if 0
    std::cout << "Type command (lookup <word>, suggest <prefix>, correct <word>, autofill <prefix>, exit):\n";

    std::string command;
    while (std::cout << "> " && std::cin >> command)
    {
        if (command == "exit") break;

        if (command == "lookup")
        {
            std::string word;
            std::cin >> word;
            WordInfo info = dict.getWordInfo(word);
            if (info.lemma.empty())
            {
                std::cout << "Word not found" << '\n';
            }
            else
            {
                dict.printInfo(info);
            }
        }
        else if (command == "suggest")
        {
            std::string prefix;
            std::cin >> prefix;
            auto suggestions = checker.suggest(prefix);
            checker.printSuggest(suggestions);
        }
        else if (command == "correct")
        {
            std::string word;
            std::cin >> word;
            auto corrections = checker.correct(word);
            if (corrections.empty())
            {
                std::cout << "No corrections found" << '\n';
            }
            else
            {
                checker.printSuggest(corrections);
            }
        }
        else if (command == "autofill")
        {
            std::string prefix;
            std::cin >> prefix;
            std::cout << checker.autofill(prefix) << '\n';
        }
        else
        {
            std::cout << "Unknown command" << '\n';
        }
    }
#endif

    return 0;
}
