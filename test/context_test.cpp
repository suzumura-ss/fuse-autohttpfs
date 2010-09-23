#include <gtest/gtest.h>
#include "mtrace.hxx"
#include "../context.h"


TEST(UrlStatMap, Initialize)
{
  MTrace mt("UrlStatMap_Initialize.mlog");

  UrlStatMap usm;
  EXPECT_EQ(0, usm.size());
}

TEST(UrlStatMap, InsertAndFind)
{
  MTrace mt("UrlStatMap_InsertAndFind.mlog");

  UrlStatMap usm;
  {
    UrlStat us(2, 100);
    EXPECT_EQ(true, usm.insert("Hello", us));
  }
  EXPECT_EQ(1, usm.size());
  {
    UrlStatMap::iterator it = usm.find("Hello");
    EXPECT_TRUE(usm.end()!=it);
    EXPECT_EQ(2, (*it).second.mode);
    EXPECT_EQ(100, (*it).second.length);
  }
  {
    UrlStatMap::iterator it = usm.find("World");
    EXPECT_TRUE(usm.end()==it);
  }

  usm.trim(1);
  EXPECT_EQ(0, usm.size());
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
