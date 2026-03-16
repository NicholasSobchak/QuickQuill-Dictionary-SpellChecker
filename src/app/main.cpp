// Fall 2025 - Dictionary/Spell Checker - Collaborative Project
#include "Utils.h"
#include "Dictionary.h"
#include "SpellChecker.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

namespace
{
	void printWordInfo() //const WordInfo& info)
	{
		
	}
}

int main()
{
    Dictionary dict;
    SpellChecker checker(dict);
    std::cout << "TEST BUILD: lookup <word>, correct <word>, suggest <word>, exit\n";

	// currently testing funcitons needed to be implemented (printWordInfo, checker.correct, checker.suggest
	std::string line;
	while (std::cout << "> " && std::getline(std::cin, line))
	{
		if (line.empty()) continue;
		std::istringstream iss(line);
		std::string command;
		std::string arg;
		iss >> command >> arg;

		if (command == "exit")
		{
			break;
		}

		if (arg.empty())
		{
			std::cout << "Usage: lookup <word> | correct <word> | suggest <word> | exit\n";
			continue;
		}

		if (command == "lookup")
		{
			WordInfo info = dict.getWordInfo(arg);
			if (info.lemma.empty())
			{
				std::cout << "word not found\n";
				continue;
			}

			printWordInfo(info);
			continue;
		}

		if (command == "correct")
		{
			const std::string correction = checker.correct(arg);
			if (correction.empty())
			{
				std::cout << "No suggestions found.\n";
				continue;
			}

			std::cout << "Did you mean " << correction << "?\n";
			continue;
		}

		if (command == "suggest")
		{
			checker.printSuggest(checker.suggest(arg));	
			continue;
		}

		std::cout << "Unknown command. Use: lookup <word> | correct <word> | suggest <word> | exit\n";
	}

    // basic HTTP TCP server build
#if 0
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	listen(serverSocket, 5);

	while (true)
	{
		int clientSocket = accept(serverSocket, nullptr, nullptr);

		char buffer[2048] = {0};
		int n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (n <= 0) { close(clientSocket); continue; }

		std::string request(buffer, n);
		std::istringstream iss(request);
		std::string method, target;
		iss >> method >> target;

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
				body = toJson(info);
			}
		}

		std::string out = body.dump();
		std::ostringstream response;
		response << "HTTP/1.1 " << status << " " << statusText << "\r\n";
		response << "Content-Type: application/json\r\n";
		response << "Content-Length: " << out.size() << "\r\n";
		response << "Connection: close\r\n\r\n";
		response << out;

		auto s = response.str();
		send(clientSocket, s.c_str(), s.size(), 0);
		close(clientSocket);
	}

	close(serverSocket);
#endif

    return 0;
}
