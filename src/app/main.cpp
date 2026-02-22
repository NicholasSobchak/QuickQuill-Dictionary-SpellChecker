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
    void printList(const std::vector<std::string>& values, const std::string& emptyText)
    {
        if (values.empty())
        {
            std::cout << emptyText << '\n';
            return;
        }

        for (const auto& v : values)
        {
            std::cout << "- " << v << '\n';
        }
    }

    void printWord(const WordInfo& info)
    {
        std::vector<std::string> synonyms;
        std::vector<std::string> antonyms;

        std::cout << "Lemma: " << (info.lemma.empty() ? "Unknown" : info.lemma) << "\n\n";

        std::cout << "Definition\n";
        if (!info.senses.empty())
        {
            for (const auto& s : info.senses)
            {
                const std::string pos = s.pos.empty() ? "" : "[" + s.pos + "] ";
                std::cout << "- " << pos << s.definition << '\n';

                synonyms.insert(synonyms.end(), s.synonyms.begin(), s.synonyms.end());
                antonyms.insert(antonyms.end(), s.antonyms.begin(), s.antonyms.end());
            }
        }
        else
        {
            std::cout << "No definitions available.\n";
        }

        std::cout << "\nSynonyms\n";
        printList(synonyms, "No synonyms available.");

        std::cout << "\nAntonyms\n";
        printList(antonyms, "No antonyms available.");

        std::cout << "\nForms\n";
        if (!info.forms.empty())
        {
            for (const auto& f : info.forms)
            {
                if (f.tag.empty()) std::cout << "- " << f.form << '\n';
                else std::cout << "- " << f.form << " (" << f.tag << ")\n";
            }
        }
        else
        {
            std::cout << "No forms available.\n";
        }

        std::cout << "\nEtymology\n";
        printList(info.etymology, "No etymology available.");
        std::cout << '\n';
    }

    [[maybe_unused]] json toJson(const WordInfo& info)
	{
		json j;
		j["id"] = info.id;
		j["lemma"] = info.lemma;
		j["forms"] = json::array();
		for (const auto& f : info.forms)
		{
			j["forms"].push_back({{"form", f.form}, {"tag", f.tag}});
		}
		j["senses"] = json::array();
		for (const auto& s : info.senses)
		{
			j["senses"].push_back({
				{"pos", s.pos},
				{"definition", s.definition},
				{"examples", s.examples},
				{"synonyms", s.synonyms},
				{"antonyms", s.antonyms}
			});
		}
		j["etymology"] = info.etymology;
		return j;
	}
}

int main()
{
    Dictionary dict;
    SpellChecker checker(dict);
    std::cout << "TEST BUILD: lookup <word>, correct <word>, exit\n";

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
			std::cout << "Usage: lookup <word> | correct <word> | exit\n";
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

            printWord(info);
			continue;
		}

		if (command == "correct")
		{
			const auto corrections = checker.correct(arg);
			std::cout << json({
				{"word", arg},
				{"corrections", corrections}
			}).dump(2) << '\n';
			continue;
		}

		std::cout << "Unknown command. Use: lookup <word> | correct <word> | exit\n";
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
