#pragma once

#include "storage_types.hpp"
#include "result_types.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <concepts>

template <DataBaseStorage Storage>
class DataBase;

Result WrongType();
bool IsError(const Result& result);
std::string Normalize(std::string key);
bool CheckEqualArguments(const std::vector<std::string>& arguments, std::size_t count);
bool CheckMinimalArguments(const std::vector<std::string>& arguments, std::size_t count);

template <DataBaseStorage Storage>
Result Set(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Get(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result StrLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Append(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Expire(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result TTL(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result RPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result RPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LRange(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LIndex(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LSet(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result LInsert(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SAdd(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SRem(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SIsMember(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SMembers(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SCard(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SUnion(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SInter(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SDiff(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result SMove(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Type(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Del(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Exists(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Keys(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result FlushDB(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Config(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result DBSize(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result Memory(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result GeoAdd(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result GeoPos(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result GeoDist(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result GeoSearch(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <DataBaseStorage Storage>
Result GeoSearchStore(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);