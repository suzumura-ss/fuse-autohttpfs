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
#include <sys/stat.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <map>


class UrlStat
{
public:
  inline UrlStat() { mode = 0; length = 0; };
  inline UrlStat(mode_t m, uint64_t l=0 ) { mode = m; length = l; };
  inline virtual ~UrlStat() {};
  inline bool is_dir() const { return !!(mode & S_IFDIR); }
  inline bool is_reg() const { return !!(mode & S_IFREG); }
  inline void operator=(const UrlStat& s) { length = s.length; mode = s.mode; };

public:
  mode_t    mode;
  uint64_t  length;
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
  void trim(size_t count);
  void dump();
};


class UrlStatCache
{
public:
  inline UrlStatCache() {};
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
  inline uint64_t size() const { return m_stats.size(); }
  void trim();
  void dump();

private:
  pthread_mutex_t m_lock;
  UrlStatMap m_stats;
  static void* cleaner(void*);
  static void alarm_handler(int);
  pthread_t m_cleaner;
  bool m_stop_cleaner;
};


#endif // __INCLUDE_CACHE_H__
// vim: sw=2 sts=2 ts=4 expandtab :
