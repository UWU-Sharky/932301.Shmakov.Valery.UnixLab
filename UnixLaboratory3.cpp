#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <openssl/sha.h>
#include <memory>
#include <set>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <iomanip>

namespace fs = std::filesystem;

std::vector<std::string> filelist;

std::string calcSHA1(const fs::path filepath)
{
	std::ifstream file(filepath, std::ios::binary);\

	if (!file.is_open())
	{
        throw std::runtime_error("Не удалось открыть файл: " + filepath.u8string());
	}

    SHA_CTX context;
    SHA1_Init(&context);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        SHA1_Update(&context, buffer, file.gcount());
    }
    // Только если последнее чтение было неполным
    if (file.gcount() > 0) {
        SHA1_Update(&context, buffer, file.gcount());
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &context);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

void searchfiles(std::string path_to_directory)
{
    try {
        for (const auto& entry : fs::directory_iterator(path_to_directory))
        {
            fs::path entry_path = entry.path();

            if (fs::is_regular_file(entry_path))
            {
                filelist.push_back(entry_path.string());
            }
            else if (fs::is_directory(entry_path))
            {
                searchfiles(entry_path.string());
            }

        }
    }
    catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Ошибка файловой системы: " + std::string(e.what()));
    }

}


int main()
{
	std::setlocale(0, "");

    searchfiles(".");

    std::map<std::string, std::string> catalog;

    for (std::string path : filelist)
    {
        catalog.insert({ path, calcSHA1(path) });
    }

    std::vector<std::string> hashlist;

    for (const auto& entry : catalog)
    {
        hashlist.push_back(entry.second);
    }

    std::set<std::string> unique_values(hashlist.begin(), hashlist.end());

    for (std::string hash : hashlist)
    {
        int count = 0;
        std::vector<std::string> checked;
        for (const auto& entry : catalog)
        {
            if (entry.second == hash)
            {
                count++;
                checked.push_back(entry.first);
            }
        }
        if (count >= 2)
        {
            for (std::string file : checked)
            {
                if (file == checked[0])
                {
                    continue;
                }
                std::string linkfile = file + ".link";
                const char* existingfile = checked[0].c_str();
                const char* linkedfile = linkfile.c_str();
                std::remove(file.c_str());
                if (link(existingfile, linkedfile) == 0)
                {
                    catalog[file] = linkedfile;
                    std::cout << "✓ Создана жесткая ссылка: " << linkedfile << std::endl;
                }
                else
                {
                    throw std::runtime_error("✗ Ошибка создания ссылки");
                }
            }
        }


    }

    for (const auto& entry : catalog)
    {
        std::cout << entry.first << "\t" << entry.second << std::endl;
    }
    
  return 0;
}