#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include "deflate/libdeflate.h"

void foreach_dirs(const auto &entry, auto &os) {
    os << entry.path() << "\n";
    if (!entry.exists()) {
        return;
    }
    for (const auto &iterator : std::filesystem::directory_iterator(entry)) {
        os << "\t" << iterator.path() << "\n";
        if (iterator.is_directory()) {
            foreach_dirs(iterator, os);
        } else if (iterator.path().extension() == ".log" || iterator.path().extension() == ".dmp" || iterator.path().extension() == ".txt") {
            os << "\t\t" << iterator.path() << "\n";
            std::vector<char> contents;
            auto infile = fopen(iterator.path().string().c_str(), "rb+");
            if (!infile) continue;

            while (!feof(infile)) {
                char ch;
                fread(&ch, 1, 1, infile);
                contents.push_back(ch);
            }
            fclose(infile);

            auto compressor = libdeflate_alloc_compressor(12);
            auto res = new char[contents.size()];
            auto sz = libdeflate_gzip_compress(compressor, contents.data(), contents.size(), res, contents.size());
            auto resfile = iterator.path();
            resfile.replace_extension(resfile.extension().string() + ".gz");
            auto file = fopen(resfile.string().c_str(), "wb+");
            if (!file) continue;
            fwrite(res, 1, sz, file);
            fclose(file);
            delete[] res;
            libdeflate_free_compressor(compressor);
            try {
                std::filesystem::remove(iterator.path());
            } catch (const std::exception &e) {
                os << e.what() << '\n';
            }
        }
    }
}
#ifdef _WIN32
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            atexit([] {
                std::filesystem::directory_entry entry("./logs/");
                foreach_dirs(entry, std::cout);
            });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#else
void __attribute__((constructor)) on_load() {
    auto pstr = std::filesystem::temp_directory_path().string();
    const auto beg = pstr.find("com");
    std::string rstr;
    for (size_t i = beg; i < pstr.size(); i++) {
        if (pstr[i] == '/')
            break;
        else
            rstr += pstr[i];
    }
    rstr = "/sdcard/Android/data/" + rstr + "/files/games/com.mojang/logs/";
    std::filesystem::directory_entry entry(rstr);
    foreach_dirs(entry, std::cout);
}
#endif
