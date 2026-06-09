#pragma once

#include "result_types.hpp"
#include "data_base.hpp"

#include <cstddef>
#include <string>
#include <vector>

Result WrongType();
bool IsError(const Result& result);
std::string Normalize(std::string key);
bool CheckEqualArguments(const std::vector<std::string>& arguments, std::size_t count);
bool CheckMinimalArguments(const std::vector<std::string>& arguments, std::size_t count);

Result Set(DataBase& data_base, const std::vector<std::string>& arguments);
Result Get(DataBase& data_base, const std::vector<std::string>& arguments);
Result StrLen(DataBase& data_base, const std::vector<std::string>& arguments);
Result Append(DataBase& data_base, const std::vector<std::string>& arguments);

Result Expire(DataBase& data_base, const std::vector<std::string>& arguments);
Result TTL(DataBase& data_base, const std::vector<std::string>& arguments);

Result LPush(DataBase& data_base, const std::vector<std::string>& arguments);
Result RPush(DataBase& data_base, const std::vector<std::string>& arguments);
Result LPop(DataBase& data_base, const std::vector<std::string>& arguments);
Result RPop(DataBase& data_base, const std::vector<std::string>& arguments);
Result LLen(DataBase& data_base, const std::vector<std::string>& arguments);
Result LRange(DataBase& data_base, const std::vector<std::string>& arguments);
Result LIndex(DataBase& data_base, const std::vector<std::string>& arguments);
Result LSet(DataBase& data_base, const std::vector<std::string>& arguments);
Result LInsert(DataBase& data_base, const std::vector<std::string>& arguments);

Result SAdd(DataBase& data_base, const std::vector<std::string>& arguments);
Result SRem(DataBase& data_base, const std::vector<std::string>& arguments);
Result SIsMember(DataBase& data_base, const std::vector<std::string>& arguments);
Result SMembers(DataBase& data_base, const std::vector<std::string>& arguments);
Result SCard(DataBase& data_base, const std::vector<std::string>& arguments);
Result SUnion(DataBase& data_base, const std::vector<std::string>& arguments);
Result SInter(DataBase& data_base, const std::vector<std::string>& arguments);
Result SDiff(DataBase& data_base, const std::vector<std::string>& arguments);
Result SMove(DataBase& data_base, const std::vector<std::string>& arguments);

Result Type(DataBase& data_base, const std::vector<std::string>& arguments);
Result Del(DataBase& data_base, const std::vector<std::string>& arguments);
Result Exists(DataBase& data_base, const std::vector<std::string>& arguments);
Result Keys(DataBase& data_base, const std::vector<std::string>& arguments);
Result FlushDB(DataBase& data_base, const std::vector<std::string>& arguments);
Result Config(DataBase& data_base, const std::vector<std::string>& arguments);
Result DBSize(DataBase& data_base, const std::vector<std::string>& arguments);
Result Memory(DataBase& data_base, const std::vector<std::string>& arguments);

Result GeoAdd(DataBase& data_base, const std::vector<std::string>& arguments);
Result GeoPos(DataBase& data_base, const std::vector<std::string>& arguments);
Result GeoDist(DataBase& data_base, const std::vector<std::string>& arguments);
Result GeoSearch(DataBase& data_base, const std::vector<std::string>& arguments);
Result GeoSearchStore(DataBase& data_base, const std::vector<std::string>& arguments);


