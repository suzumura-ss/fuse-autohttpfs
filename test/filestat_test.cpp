#include <gtest/gtest.h>
#include "mtrace.hxx"
#include "../filestat.h"
#include "../ext/time_iso8601.h"
#include "../int64format.h"


TEST(FileStat, Initialize)
{
  MTrace mt("FileStat_Initialize.mlog");

  FileStat fs;
  EXPECT_TRUE(fs.name.empty());
  EXPECT_EQ((unsigned)S_IFDIR, fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);
}

TEST(FileStat, from_json_bad_stat)
{
  MTrace mt("FileStat_from_json_bad_stat.mlog");

  std::string json = "";
  FileStat fs;
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "[]";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "{}";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "[\"name\"]";
  EXPECT_THROW(fs.from_json(json), std::string);

  json = "{\"name\":[]}";
  EXPECT_NO_THROW(fs.from_json(json));

  json = "{\"name\":{}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name", fs.name);
  EXPECT_EQ((unsigned)S_IFDIR, fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);

  fs = FileStat();
  json = "{\"name\":1}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name", fs.name);
  EXPECT_EQ((unsigned)S_IFDIR, fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);

  fs = FileStat();
  json = "{\"name1\":{\"mode\":\"k\"}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name1", fs.name);
  EXPECT_EQ(0U, fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);

  fs = FileStat();
  json = "{\"name2\":{\"mtime\":\"000-00-00T00:00:00+0000\"}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name2", fs.name);
  EXPECT_EQ((unsigned)S_IFDIR, fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);
}

TEST(FileStat, from_json)
{
  MTrace mt("FileStat_from_json.mlog");

  std::string json = "";
  FileStat fs;

  json = "{\"name1\":{\"mode\":\"dr---w---x\"}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name1", fs.name);
  EXPECT_EQ((unsigned)(S_IFDIR|S_IRUSR|S_IWGRP|S_IXOTH), fs.mode);
  EXPECT_EQ(0U, fs.size);
  EXPECT_EQ(0, fs.mtime);

  json = "{\"name2\":{\"size\":100}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name2", fs.name);
  EXPECT_EQ((unsigned)(S_IFDIR|S_IRUSR|S_IWGRP|S_IXOTH), fs.mode);
  EXPECT_EQ(100U, fs.size);
  EXPECT_EQ(0, fs.mtime);

  json = "{\"name3\":{\"mtime\":\"2010-10-01T12:34:56+0900\"}}";
  EXPECT_NO_THROW(fs.from_json(json));
  EXPECT_EQ("name3", fs.name);
  EXPECT_EQ((unsigned)(S_IFDIR|S_IRUSR|S_IWGRP|S_IXOTH), fs.mode);
  EXPECT_EQ(100U, fs.size);
  TimeIso8601 t("2010-10-01T12:34:56+0900");
  EXPECT_EQ((time_t)t, fs.mtime);
}


int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
