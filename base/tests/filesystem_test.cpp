#include "gtest/gtest.h"

#include "nhbase/filesystem.hpp"

TEST(filesystem_test, file_exists)
{
    /* Too simple to be useful */
    EXPECT_FALSE(nb::file_exists(""));
    // directory input should return false.
    EXPECT_FALSE(nb::file_exists("./"));
}
