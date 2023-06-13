#include "alioth/utils/clan.h"

#include "gtest/gtest.h"

TEST(CHAR_SET, ParseEmpty) {
    std::string notion = "";
    auto set = alioth::Clan::Parse(notion).GetChars();
    std::set<unsigned char> expect;
    ASSERT_EQ(set, expect);
}

TEST(CHAR_SET, ParsePoint) {
    std::string notion = "0";
    auto set = alioth::Clan::Parse(notion).GetChars();
    std::set<unsigned char> expect;
    expect.insert(0);
    ASSERT_EQ(set, expect);

    notion = "1";
    set = alioth::Clan::Parse(notion).GetChars();
    expect.clear();
    expect.insert(1);
    ASSERT_EQ(set, expect);

    notion = "ff";
    set = alioth::Clan::Parse(notion).GetChars();
    expect.clear();
    expect.insert(255);
    ASSERT_EQ(set, expect);

    notion = "80,ff";
    set = alioth::Clan::Parse(notion).GetChars();
    expect.clear();
    expect.insert(128);
    expect.insert(255);
    ASSERT_EQ(set, expect);
}

TEST(CHAR_SET, ParseRange) {
    std::string notion = "0-ff";
    auto set = alioth::Clan::Parse(notion).GetChars();
    std::set<unsigned char> expect;
    for (int i = 0; i < 256; i++) expect.insert(i);
    ASSERT_EQ(set, expect);

    notion = "0-80";
    set = alioth::Clan::Parse(notion).GetChars();
    expect.clear();
    for (int i = 0; i <= 128; i++) expect.insert(i);
    ASSERT_EQ(set, expect);

    notion = "12-34,56-78";
    set = alioth::Clan::Parse(notion).GetChars();
    expect.clear();
    for (int i = 0x12; i <= 0x34; i++) expect.insert(i);
    for (int i = 0x56; i <= 0x78; i++) expect.insert(i);
    ASSERT_EQ(set, expect);
}