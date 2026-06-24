#include <gtest/gtest.h>

#include <filesystem>
#include <future>
#include <thread>
#include <vector>

#include "db_handler/db_handler.hpp"

namespace fs = std::filesystem;

using DB = db_handler::Handler<4, 8, 64, 256, Policies::LRU>;

class HandlerTest : public ::testing::Test {
protected:
    fs::path db_path = "data_base.txt";

    void SetUp() override {
        fs::remove(db_path);
    }

    void TearDown() override {
        fs::remove(db_path);
    }
};

TEST_F(HandlerTest, PutNewKey) {
    DB db(db_path);

    EXPECT_TRUE(db.Put("a", "1").get());
}

TEST_F(HandlerTest, GetExistingKey) {
    DB db(db_path);

    ASSERT_TRUE(db.Put("a", "1").get());

    auto result = db.Get("a").get();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "1");
}

TEST_F(HandlerTest, GetMissingKey) {
    DB db(db_path);

    auto result = db.Get("missing").get();

    EXPECT_FALSE(result.has_value());
}

TEST_F(HandlerTest, OverwriteValue) {
    DB db(db_path);

    ASSERT_TRUE(db.Put("a", "1").get());
    ASSERT_TRUE(db.Put("a", "2").get());

    auto result = db.Get("a").get();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "2");
}

TEST_F(HandlerTest, MultipleKeys) {
    DB db(db_path);

    ASSERT_TRUE(db.Put("a", "1").get());
    ASSERT_TRUE(db.Put("b", "2").get());
    ASSERT_TRUE(db.Put("c", "3").get());

    EXPECT_EQ(db.Get("a").get().value(), "1");
    EXPECT_EQ(db.Get("b").get().value(), "2");
    EXPECT_EQ(db.Get("c").get().value(), "3");
}

TEST_F(HandlerTest, EmptyValue) {
    DB db(db_path);

    ASSERT_TRUE(db.Put("key", "").get());

    auto result = db.Get("key").get();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "");
}

TEST_F(HandlerTest, MaxKeySizeBoundary) {
    DB db(db_path);

    std::string key(64, 'k');

    ASSERT_TRUE(db.Put(key, "value").get());

    auto result = db.Get(key).get();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "value");
}

TEST_F(HandlerTest, MaxValueSizeBoundary) {
    DB db(db_path);

    std::string value(256, 'v');

    ASSERT_TRUE(db.Put("key", value).get());

    auto result = db.Get("key").get();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, value);
}

TEST_F(HandlerTest, ConcurrentWritesDifferentKeys) {
    DB db(db_path);

    constexpr int kThreads = 32;
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i] {
            db.Put("key_" + std::to_string(i),
                   "value_" + std::to_string(i))
                .get();
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    for (int i = 0; i < kThreads; ++i) {
        auto value =
            db.Get("key_" + std::to_string(i)).get();

        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(*value, "value_" + std::to_string(i));
    }
}

TEST_F(HandlerTest, ConcurrentReadsAndWrites) {
    DB db(db_path);

    constexpr int kThreads = 16;

    std::vector<std::thread> writers;
    std::vector<std::thread> readers;

    for (int i = 0; i < kThreads; ++i) {
        writers.emplace_back([&, i] {
            db.Put("key_" + std::to_string(i),
                   "value_" + std::to_string(i))
                .get();
        });
    }

    for (int i = 0; i < kThreads; ++i) {
        readers.emplace_back([&, i] {
            db.Get("key_" + std::to_string(i)).get();
        });
    }

    for (auto& t : writers) {
        t.join();
    }

    for (auto& t : readers) {
        t.join();
    }

    for (int i = 0; i < kThreads; ++i) {
        auto result =
            db.Get("key_" + std::to_string(i)).get();

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "value_" + std::to_string(i));
    }
}