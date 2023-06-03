#include "gtest/gtest.h"

#include "nhbase/filesystem.hpp"

TEST(filesystem_test, file_exists)
{
    // empty path
    EXPECT_FALSE(nb::file_exists(""));
    // directory input
    EXPECT_FALSE(nb::file_exists("./"));
}
