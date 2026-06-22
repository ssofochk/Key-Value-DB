#include <gtest/gtest.h>

#include "cache.hpp"

TEST(StringTest, SetGet) {
    Cache<Policies::NOEVICTION, std::string> cache;

    ASSERT_TRUE(cache.Put("Avito", "Tech"));

    auto* result = cache.Get("Avito");

    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(std::holds_alternative<std::string>(*result));
    ASSERT_EQ(std::get<std::string>(*result), "Tech");
}