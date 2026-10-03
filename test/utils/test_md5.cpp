#include <gtest/gtest.h>

#include "utils/md5.h"

TEST(TestMD5, Basic) {
    std::string src = "this is a test md5";
    auto encoded = utils::MD5::Digest(src);
    EXPECT_EQ(strcasecmp(encoded.c_str(), "8AD9C65947107A6BCA12F19EB2145348"), 0);
}
