#include "gtest/gtest.h"
#include "utils/base64.h"

TEST(TestBase64, Basic) {
    std::string src = "this is a test base64";
    auto encoded = utils::Base64::Encode(src);
    EXPECT_EQ("dGhpcyBpcyBhIHRlc3QgYmFzZTY0", encoded);

    std::string decoded;
    auto ret = utils::Base64::Decode(encoded, decoded);
    EXPECT_TRUE(ret);
    EXPECT_EQ(src, decoded);
}
