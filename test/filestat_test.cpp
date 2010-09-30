#include <gtest/gtest.h>
#include "mtrace.hxx"
#include "../filestat.h"
#include "../int64format.h"


TEST(FileStat, Initialize)
{
  MTrace mt("FileStat_Initialize.mlog");

  FileStat fs;
  EXPECT_TRUE(fs.name.empty());
  EXPECT_EQ(S_IFDIR, fs.mode);
  EXPECT_EQ(0, fs.size);
  EXPECT_EQ(0, fs.mtime);
}

TEST(FileStat, from_json)
{
  MTrace mt("FileStat_from_json.mlog");

  std::string json = "";
  FileStat fs;
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "[]";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "{}";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "[\"name\"]";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "{\"name\":1}";
  try {
    fs.from_json(json);
  }
  catch(std::string e) {
    std::cout << e << std::endl;
  }
}


int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
