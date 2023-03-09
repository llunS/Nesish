#include "gtest/gtest.h"

#include "common/path.hpp"

TEST(path_test, dirname)
{
    // root case
    {
        EXPECT_EQ(nh::dirname("/foo"), "/");
        EXPECT_EQ(nh::dirname("/"), "/");
    }
    // normal case
    {
        EXPECT_EQ(nh::dirname("/foo/bar"), "/foo");
        EXPECT_EQ(nh::dirname("foo/bar"), "foo");

        // NOT this,
        // EXPECT_EQ(nh::dirname("/foo/bar/"), "/foo");
        // EXPECT_EQ(nh::dirname("foo/bar/"), "foo");
        // BUT this, the same behavior as os.path.dirname in Python.
        EXPECT_EQ(nh::dirname("/foo/bar/"), "/foo/bar");
        EXPECT_EQ(nh::dirname("foo/bar/"), "foo/bar");
    }
    // empty input
    {
        EXPECT_EQ(nh::dirname(""), "");
    }
    // non empty, no slashes.
    {
        EXPECT_EQ(nh::dirname("foo"), "");
    }
}
