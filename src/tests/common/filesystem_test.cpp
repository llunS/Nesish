#include "gtest/gtest.h"

#include "common/filesystem.hpp"

TEST(filesystem_test, file_exists)
{
    /* Too simple to be useful */
    EXPECT_FALSE(nh::file_exists(""));
    // directory input should return false.
    EXPECT_FALSE(nh::file_exists("./"));
}
