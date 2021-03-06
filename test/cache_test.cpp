#include <gtest/gtest.h>
#include "mtrace.hxx"
#include "../cache.h"
#include "../int64format.h"


TEST(UrlStatMap, Initialize)
{
  MTrace mt("UrlStatMap_Initialize.mlog");

  UrlStatMap usm;
  EXPECT_EQ(0U, usm.size());
}


TEST(UrlStatMap, InsertAndFind)
{
  MTrace mt("UrlStatMap_InsertAndFind.mlog");

  UrlStatMap usm;
  EXPECT_EQ(true, usm.insert("Hello", UrlStat(2, 100)));
  EXPECT_EQ(1U, usm.size());

  {
    UrlStatMap::iterator it = usm.find("Hello");
    EXPECT_TRUE(usm.end()!=it);
    EXPECT_EQ(2U, (*it).second.mode);
    EXPECT_EQ(100U, (*it).second.length);
  }
  {
    UrlStatMap::iterator it = usm.find("World");
    EXPECT_TRUE(usm.end()==it);
  }
}

TEST(UrlStatMap, Remove)
{
  MTrace mt("UrlStatMap_Remove.mlog");

  UrlStatMap usm;
  usm.insert("Hello", UrlStat(2, 100, 0, time(NULL)+1));

  usm.remove("world");
  EXPECT_TRUE(usm.end()!=usm.find_with_expire("Hello"));

  usm.remove("Hello");
  EXPECT_TRUE(usm.end()==usm.find_with_expire("Hello"));
}

TEST(UrlStatMap, Trim)
{
  MTrace mt("UrlStatMap_Trim.mlog");

  UrlStatMap usm;
  usm.insert("foo", UrlStat(1, 100));
  usm.insert("bar", UrlStat(2, 200));
  usm.insert("baz", UrlStat(3, 300));

  EXPECT_EQ(3U, usm.size());

  usm.trim(1);
  EXPECT_EQ(2U, usm.size());
  EXPECT_TRUE(usm.end()==usm.find("foo"));
  EXPECT_TRUE(usm.end()!=usm.find("bar"));
  EXPECT_TRUE(usm.end()!=usm.find("baz"));

  usm.trim(1);
  EXPECT_EQ(1U, usm.size());
  EXPECT_TRUE(usm.end()==usm.find("foo"));
  EXPECT_TRUE(usm.end()==usm.find("bar"));
  EXPECT_TRUE(usm.end()!=usm.find("baz"));

  usm.trim(1);
  EXPECT_EQ(0U, usm.size());
  EXPECT_TRUE(usm.end()==usm.find("foo"));
  EXPECT_TRUE(usm.end()==usm.find("bar"));
  EXPECT_TRUE(usm.end()==usm.find("baz"));
}



TEST(UrlStatCache, Initialize)
{
  MTrace("UrlStatCache_Initialize.mlog");

  UrlStatCache usc;
  usc.init();
  usc.stop();
}


TEST(UrlStatCache, InsertAndFind)
{
  MTrace("UrlStatCache_InsertAndFind.mlog");

  UrlStatCache usc;
  usc.init();

  usc.add("Hello", UrlStat(2, 100));
  {
    UrlStat us(0, 0);
    EXPECT_EQ(true, usc.find("Hello", us));
    EXPECT_EQ(2U, us.mode);
    EXPECT_EQ(100U, us.length);
  }
  {
    UrlStat us(0, 0);
    EXPECT_EQ(false,usc.find("World", us));
    EXPECT_EQ(0U, us.mode);
    EXPECT_EQ(0U, us.length);
  }

  usc.stop();
}

TEST(UrlStatCache, Remove)
{
  MTrace mt("UrlStatCache_Remove.mlog");

  UrlStatCache usc;
  usc.init();
  UrlStat us;

  usc.add("Hello", UrlStat(2, 100));

  usc.remove("World");
  EXPECT_EQ(true, usc.find("Hello", us));

  usc.remove("Hello");
  EXPECT_EQ(false, usc.find("Hello", us));

  usc.stop();
}

TEST(UrlStatCache, Expire)
{
  MTrace("UrlStatCache_Expire.mlog");

  UrlStatCache usc;
  usc.init();

  usc.add("Hello", UrlStat(2, 100));
  sleep(2);

  UrlStat us(0, 0);
  EXPECT_EQ(false, usc.find("Hello", us));

  usc.stop();
}


void* cache_access_proc(void* ctx)
{
  UrlStatCache* usc = (UrlStatCache*)ctx;

  for(unsigned ai=0; ai<10000; ai++) {
    char key[100];
    UrlStat us;
    snprintf(key, sizeof(key), "%d/%d", (int)pthread_self(), ai);
    usc->add(key, ai % 100, ai);
    if(usc->find(key, us)) {
      EXPECT_EQ(ai % 100, us.mode);
      EXPECT_EQ(ai, us.length);
    }
    if(ai&3) {
      snprintf(key, sizeof(key), "%d/%d", (int)pthread_self(), ai*4); // sometimes are invalid.
    }
    usc->remove(key);
    usleep(30);
  }

  return NULL;
}

TEST(UrlStatCache, Loop)
{
  MTrace("UrlStatCache_Loop.mlog");

  int threads = 50;

  UrlStatCache usc;
  usc.init();
  pthread_t* th = new pthread_t[threads];

  for(int ai=0; ai<threads; ai++) {
    pthread_create(&th[ai], NULL, cache_access_proc, &usc);
  }

  cache_access_proc(&usc);
  uint64_t n = usc.size();
  printf("UrlStatCache::size() = %"FINT64"u\n", n);

  for(int ai=0; ai<threads; ai++) {
    void* v;
    pthread_join(th[ai], &v);
  }

  delete[] th;
  usc.stop();
}



int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
