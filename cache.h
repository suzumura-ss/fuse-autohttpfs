/*
  Copyright 2010 Toshiyuki Terashita.

  fuse-autohttpfs is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __INCLUDE_CACHE_H__
#define __INCLUDE_CACHE_H__

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string>
#include <map>
#include <deque>
#include "log.h"

#ifndef CACHE_EXPIRES_SEC
# define CACHE_EXPIRES_SEC (180) // sec
#endif



class UrlStat
{
public:
  inline UrlStat(mode_t m = S_IFDIR, uint64_t l = 0, time_t e = 0) {
    mode = m;
    length = l;
    expire = e;
  };
  inline virtual ~UrlStat() {};
  inline bool is_valid() const { return (expire>=time(NULL))? true: false; };
  inline bool is_dir() const { return !!(mode & S_IFDIR); }
  inline bool is_reg() const { return !!(mode & S_IFREG); }
  inline void operator=(const UrlStat& s) { length = s.length; mode = s.mode; };

public:
  mode_t    mode;
  uint64_t  length;
  time_t    expire;
};


class Url: public std::string
{
public:
  inline Url(const char* u) { std::string::operator=(u); };
  inline bool operator==(const Url& y) const { return strcmp(c_str(), y.c_str())==0; };
  inline bool operator<(const Url& y) const {
    if(size()==y.size()) return (strcmp(c_str(), y.c_str())>0);
    return (size()>y.size());
  };
};


typedef std::map<Url, UrlStat> UrlStatBASE;
class UrlStatMap: public UrlStatBASE
{
public:
  bool insert(const char* path, const UrlStat& us);
  iterator find_with_expire(const char* path);
  void trim(size_t count);
  void dump(Log& logger);

private:
  std::deque<iterator> m_entries;
};


class UrlStatCache
{
public:
  inline UrlStatCache() { m_expire_sec = CACHE_EXPIRES_SEC; };
  inline virtual ~UrlStatCache() {
    try { m_stats.clear(); }
    catch(...){}
  };
  void init();
  void stop();
  void add(const char* path, mode_t mode, uint64_t length);
  inline void add(const char* path, const UrlStat& stat) {
    add(path, stat.mode, stat.length);
  };
  bool find(const char* path, UrlStat& stat);
  inline void set_expire(time_t sec) { m_expire_sec = sec; };
  inline uint64_t size() const { return m_stats.size(); }
  void trim();
  void dump(Log& logger);

private:
  pthread_mutex_t m_lock;
  UrlStatMap m_stats;
  size_t  m_max_entries;
  time_t  m_expire_sec;
  static void* cleaner(void*);
  static void alarm_handler(int);
  pthread_t m_cleaner;
  bool m_stop_cleaner;
};


#endif // __INCLUDE_CACHE_H__
// vim: sw=2 sts=2 ts=4 expandtab :
