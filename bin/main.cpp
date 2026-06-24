#include "cache/cache.hpp"
#include "db_handler/db_handler.hpp"
#include <filesystem>
#include <iostream>
#include <string>

int main () {
    

    const char* c = "data_base.txt";
    db_handler::Handler<1, 1, 256, 1024, Policies::NOEVICTION> handler{c};
    // FileWorker<256, 1024> f;
    std::filesystem::path p(std::filesystem::current_path().string() + "\\data_base.txt");
    std::filesystem::path cur = std::filesystem::current_path();
    std::cout << p << '\n';
    std::cout << cur;
    // db_handler::Handler
    // Cache<Policies::NOEVICTION> c;
}