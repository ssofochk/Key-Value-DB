#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "data_base.hpp"
#include "result_types.hpp"

IntegerResult GetInteger(const Result& result) {
    auto* value = std::get_if<IntegerResult>(&result);

    if (!value) {
        throw std::runtime_error("Expected IntegerResult");
    }

    return *value;
}

StringResult GetString(const Result& result) {
    auto* value = std::get_if<StringResult>(&result);

    if (!value) {
        throw std::runtime_error("Expected StringResult");
    }

    return *value;
}

ListResult GetList(const Result& result) {
    auto* value = std::get_if<ListResult>(&result);

    if (!value) {
        throw std::runtime_error("Expected ListResult");
    }

    return *value;
}

GeoListResult GetGeoList(const Result& result) {
    auto* value = std::get_if<GeoListResult>(&result);

    if (!value) {
        throw std::runtime_error("Expected GeoListResult");
    }

    return *value;
}

bool IsOk(const Result& result) {
    return std::get_if<OkResult>(&result);
}

bool IsNil(const Result& result) {
    return std::get_if<NilResult>(&result);
}

bool IsErr(const Result& result) {
    return std::get_if<ErrorResult>(&result);
}

TEST(StringTest, SetGet) {
    DataBase data_base;

    ASSERT_TRUE(IsOk(data_base.Execute("SET aaa 111")));

    StringResult result = GetString(data_base.Execute("GET aaa"));

    ASSERT_EQ(result.value_, "111");
}

TEST(StringTest, GetMissing) {
    DataBase data_base;

    ASSERT_TRUE(IsNil(data_base.Execute("GET aaa")));
}

TEST(StringTest, Append) {
    DataBase data_base;

    ASSERT_TRUE(IsOk(data_base.Execute("SET aaa 111")));

    IntegerResult append = GetInteger(data_base.Execute("APPEND aaa 222"));
    StringResult value = GetString(data_base.Execute("GET aaa"));

    ASSERT_EQ(append.value_, 6);
    ASSERT_EQ(value.value_, "111222");
}

TEST(StringTest, QuotedValue) {
    DataBase data_base;

    ASSERT_TRUE(IsOk(data_base.Execute("SET aaa \"111 222\"")));

    StringResult result = GetString(data_base.Execute("GET aaa"));

    ASSERT_EQ(result.value_, "111 222");
}

TEST(StringTest, WrongType) {
    DataBase data_base;

    ASSERT_EQ(GetInteger(data_base.Execute("RPUSH aaa 111")).value_, 1);

    ASSERT_TRUE(IsErr(data_base.Execute("GET aaa")));
}

TEST(StringTest, WrongArguments) {
    DataBase data_base;

    ASSERT_TRUE(IsErr(data_base.Execute("GET")));
}

TEST(ListTest, PushRange) {
    DataBase data_base;

    ASSERT_EQ(GetInteger(data_base.Execute("RPUSH aaa 111 222 333")).value_, 3);

    ListResult result = GetList(data_base.Execute("LRANGE aaa 0 -1"));

    ASSERT_EQ(result.value_, std::vector<std::string>({"111", "222", "333"}));
}

TEST(ListTest, PopCount) {
    DataBase data_base;

    data_base.Execute("RPUSH aaa 111 222 333 444");

    ListResult result = GetList(data_base.Execute("LPOP aaa 2"));
    ListResult rest = GetList(data_base.Execute("LRANGE aaa 0 -1"));

    ASSERT_EQ(result.value_, std::vector<std::string>({"111", "222"}));
    ASSERT_EQ(rest.value_, std::vector<std::string>({"333", "444"}));
}

TEST(ListTest, IndexAndSet) {
    DataBase data_base;

    data_base.Execute("RPUSH aaa 111 222 333");

    ASSERT_TRUE(IsOk(data_base.Execute("LSET aaa -1 444")));

    StringResult result = GetString(data_base.Execute("LINDEX aaa -1"));

    ASSERT_EQ(result.value_, "444");
}

TEST(ListTest, Insert) {
    DataBase data_base;

    data_base.Execute("RPUSH aaa 111 333");

    IntegerResult size = GetInteger(data_base.Execute("LINSERT aaa BEFORE 333 222"));
    ListResult result = GetList(data_base.Execute("LRANGE aaa 0 -1"));

    ASSERT_EQ(size.value_, 3);
    ASSERT_EQ(result.value_, std::vector<std::string>({"111", "222", "333"}));
}

TEST(SetTest, AddMembers) {
    DataBase data_base;

    IntegerResult added = GetInteger(data_base.Execute("SADD aaa 111 222 222"));

    ASSERT_EQ(added.value_, 2);
    ASSERT_EQ(GetInteger(data_base.Execute("SCARD aaa")).value_, 2);
}

TEST(SetTest, RemoveMember) {
    DataBase data_base;

    data_base.Execute("SADD aaa 111 222 333");

    IntegerResult removed = GetInteger(data_base.Execute("SREM aaa 222 444"));

    ASSERT_EQ(removed.value_, 1);
    ASSERT_EQ(GetInteger(data_base.Execute("SISMEMBER aaa 222")).value_, 0);
}

TEST(SetTest, Union) {
    DataBase data_base;

    data_base.Execute("SADD aaa 111 222");
    data_base.Execute("SADD bbb 222 333");

    ListResult result = GetList(data_base.Execute("SUNION aaa bbb"));

    ASSERT_EQ(result.value_.size(), 3);
}

TEST(SetTest, Move) {
    DataBase data_base;

    data_base.Execute("SADD aaa 111 222");
    data_base.Execute("SADD bbb 333");

    ASSERT_EQ(GetInteger(data_base.Execute("SMOVE aaa bbb 111")).value_, 1);
    ASSERT_EQ(GetInteger(data_base.Execute("SISMEMBER aaa 111")).value_, 0);
    ASSERT_EQ(GetInteger(data_base.Execute("SISMEMBER bbb 111")).value_, 1);
}

TEST(GeoTest, AddAndType) {
    DataBase data_base;

    IntegerResult added = GetInteger(data_base.Execute(
        "GEOADD aaa 13.361389 38.115556 bbb 15.087269 37.502669 ccc"
    ));

    StringResult type = GetString(data_base.Execute("TYPE aaa"));

    ASSERT_EQ(added.value_, 2);
    ASSERT_EQ(type.value_, "zset");
}

TEST(GeoTest, Pos) {
    DataBase data_base;

    data_base.Execute("GEOADD aaa 13.361389 38.115556 bbb");

    GeoListResult result = GetGeoList(data_base.Execute("GEOPOS aaa bbb ccc"));

    ASSERT_EQ(result.value_.size(), 2);
    ASSERT_NE(std::get_if<GeoResult>(&result.value_[0]), nullptr);
    ASSERT_NE(std::get_if<NilResult>(&result.value_[1]), nullptr);
}

TEST(GeoTest, Distance) {
    DataBase data_base;

    data_base.Execute("GEOADD aaa 13.361389 38.115556 bbb 15.087269 37.502669 ccc");

    StringResult result = GetString(data_base.Execute("GEODIST aaa bbb ccc KM"));

    ASSERT_FALSE(result.value_.empty());
}

TEST(GeoTest, SearchCount) {
    DataBase data_base;

    data_base.Execute(
        "GEOADD aaa 13.361389 38.115556 bbb 15.087269 37.502669 ccc "
        "12.496366 41.902782 ddd"
    );

    ListResult result = GetList(data_base.Execute(
        "GEOSEARCH aaa FROMLONLAT 13.361389 38.115556 BYRADIUS 200 KM ASC COUNT 1"
    ));

    ASSERT_EQ(result.value_.size(), 1);
    ASSERT_EQ(result.value_[0], "bbb");
}

TEST(GeoTest, SearchStore) {
    DataBase data_base;

    data_base.Execute("GEOADD aaa 13.361389 38.115556 bbb 15.087269 37.502669 ccc");

    IntegerResult stored = GetInteger(data_base.Execute(
        "GEOSEARCHSTORE ddd aaa FROMLONLAT 13.361389 38.115556 BYRADIUS 200 KM ASC"
    ));

    ASSERT_EQ(stored.value_, 2);
    ASSERT_EQ(GetString(data_base.Execute("TYPE ddd")).value_, "zset");
}

TEST(TTLTest, ExpireMissing) {
    DataBase data_base;

    ASSERT_EQ(GetInteger(data_base.Execute("EXPIRE aaa 10")).value_, 0);
}

TEST(TTLTest, TtlWithoutExpire) {
    DataBase data_base;

    data_base.Execute("SET aaa 111");

    ASSERT_EQ(GetInteger(data_base.Execute("TTL aaa")).value_, -1);
}

TEST(TTLTest, ExpireZero) {
    DataBase data_base;

    data_base.Execute("SET aaa 111");

    ASSERT_EQ(GetInteger(data_base.Execute("EXPIRE aaa 0")).value_, 1);
    ASSERT_TRUE(IsNil(data_base.Execute("GET aaa")));
    ASSERT_EQ(GetInteger(data_base.Execute("TTL aaa")).value_, -2);
}

TEST(TTLTest, ExpireAfterTime) {
    DataBase data_base;

    data_base.Execute("SET aaa 111");
    data_base.Execute("EXPIRE aaa 1");

    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    ASSERT_TRUE(IsNil(data_base.Execute("GET aaa")));
    ASSERT_EQ(GetInteger(data_base.Execute("EXISTS aaa")).value_, 0);
}

TEST(MemoryTest, UsageMissing) {
    DataBase data_base;

    ASSERT_TRUE(IsNil(data_base.Execute("MEMORY USAGE aaa")));
}

TEST(MemoryTest, UsageGrows) {
    DataBase data_base;

    data_base.Execute("SET aaa 111");
    int64_t first = GetInteger(data_base.Execute("MEMORY USAGE aaa")).value_;

    data_base.Execute("APPEND aaa 222222222222222222222222222222");
    int64_t second = GetInteger(data_base.Execute("MEMORY USAGE aaa")).value_;

    ASSERT_GE(second, first);
}

TEST(MemoryTest, MaxMemoryRejectsNewKey) {
    DataBase data_base;

    data_base.Execute("CONFIG SET maxmemory 50");

    Result result = data_base.Execute("SET aaa 111111111111111111111111111111111111");

    ASSERT_TRUE(IsErr(result));
    ASSERT_TRUE(IsNil(data_base.Execute("GET aaa")));
}

TEST(MemoryTest, MaxMemoryAllowsSmallerReplacement) {
    DataBase data_base;

    data_base.Execute("SET aaa 111111111111111111111111111111111111");
    data_base.Execute("CONFIG SET maxmemory 150");

    Result result = data_base.Execute("SET aaa 111");

    ASSERT_TRUE(IsOk(result));
    ASSERT_EQ(GetString(data_base.Execute("GET aaa")).value_, "111");
}

TEST(MemoryTest, DeleteFreesMemory) {
    DataBase data_base;

    data_base.Execute("CONFIG SET maxmemory 150");
    data_base.Execute("SET aaa 111");

    ASSERT_TRUE(IsErr(data_base.Execute("SET bbb 222")));

    data_base.Execute("DEL aaa");

    ASSERT_TRUE(IsOk(data_base.Execute("SET bbb 222")));
}