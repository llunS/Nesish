#include "gtest/gtest.h"

#include "common/filesystem.hpp"

TEST(filesystem_test, file_exists)
{
    EXPECT_FALSE(ln::file_exists(""));
    // directory input should return false.
    EXPECT_FALSE(ln::file_exists("./"));
}
