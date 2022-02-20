#include "gtest/gtest.h"

#include "common/path.hpp"

TEST(path_test, dirname)
{
    // root case
    {
        EXPECT_EQ(ln::dirname("/foo"), "/");
        EXPECT_EQ(ln::dirname("/"), "/");
    }
    // normal case
    {
        EXPECT_EQ(ln::dirname("/foo/bar"), "/foo");
        EXPECT_EQ(ln::dirname("foo/bar"), "foo");

        // NOT this,
        // EXPECT_EQ(ln::dirname("/foo/bar/"), "/foo");
        // EXPECT_EQ(ln::dirname("foo/bar/"), "foo");
        // BUT this, the same behavior as os.path.dirname in Python.
        EXPECT_EQ(ln::dirname("/foo/bar/"), "/foo/bar");
        EXPECT_EQ(ln::dirname("foo/bar/"), "foo/bar");
    }
    // empty input
    {
        EXPECT_EQ(ln::dirname(""), "");
    }
    // non empty, no slashes.
    {
        EXPECT_EQ(ln::dirname("foo"), "");
    }
}
