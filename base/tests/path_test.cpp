#include "gtest/gtest.h"

#include "nhbase/path.hpp"

TEST(path_test, dirname)
{
    // root case
    {
        EXPECT_EQ(nb::dirname("/foo"), "/");
        EXPECT_EQ(nb::dirname("/"), "/");
    }
    // normal case
    {
        EXPECT_EQ(nb::dirname("/foo/bar"), "/foo");
        EXPECT_EQ(nb::dirname("foo/bar"), "foo");

        // NOT this,
        // EXPECT_EQ(nb::dirname("/foo/bar/"), "/foo");
        // EXPECT_EQ(nb::dirname("foo/bar/"), "foo");
        // BUT this, the same behavior as os.path.dirname in Python.
        EXPECT_EQ(nb::dirname("/foo/bar/"), "/foo/bar");
        EXPECT_EQ(nb::dirname("foo/bar/"), "foo/bar");
    }
    // empty input
    {
        EXPECT_EQ(nb::dirname(""), "");
    }
    // non empty, no slashes.
    {
        EXPECT_EQ(nb::dirname("foo"), "");
    }
}
