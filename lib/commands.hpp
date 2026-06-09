#pragma once

#include "result_types.hpp"

#include <cstddef>
#include <string>
#include <vector>

template <class Storage>
class DataBase;

Result WrongType();
bool IsError(const Result& result);
std::string Normalize(std::string key);
bool CheckEqualArguments(const std::vector<std::string>& arguments, std::size_t count);
bool CheckMinimalArguments(const std::vector<std::string>& arguments, std::size_t count);

template <class Storage>
Result Set(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Get(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result StrLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Append(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Expire(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result TTL(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result RPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result RPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LRange(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LIndex(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LSet(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result LInsert(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SAdd(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SRem(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SIsMember(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SMembers(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SCard(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SUnion(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SInter(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SDiff(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result SMove(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Type(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Del(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Exists(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Keys(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result FlushDB(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Config(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result DBSize(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result Memory(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result GeoAdd(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result GeoPos(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result GeoDist(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result GeoSearch(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);

template <class Storage>
Result GeoSearchStore(DataBase<Storage>& data_base, const std::vector<std::string>& arguments);