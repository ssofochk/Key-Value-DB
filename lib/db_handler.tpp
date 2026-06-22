#include <string>
#include "policies.hpp"

template <Policies Policy>
std::string* db_handler::Handler<Policy>::Get(const db_handler::Handler<Policy>::Type& key) {
    // todo
}


template <Policies Policy>
bool db_handler::Handler<Policy>::Set(const std::string& key, db_handler::Handler<Policy>::Type value) {
    // todo
}