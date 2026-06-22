#include <gtest/gtest.h>

#include "cache.hpp"

TEST(StringTest, SetGet) {
    Cache<Policies::NOEVICTION> cache;

    ASSERT_TRUE(cache.Put("Avito", "Tech"));

    auto* result = cache.Get("Avito");

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(*result, "Tech");
}