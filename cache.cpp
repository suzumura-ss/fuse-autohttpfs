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

#include <signal.h>
#include "cache.h"
#include "log.h"
#include "globals.h"
#include "int64format.h"

#ifndef CACHE_MAX_ENTRIES
# define CACHE_MAX_ENTRIES (2000)
#endif


// UrlStatMap class implements.
bool UrlStatMap::insert(const char* path, const UrlStat& us)
{
  Url key(path);
  std::pair<UrlStatBASE::iterator, bool> r = UrlStatBASE::insert(std::make_pair(key, us));
  if(!r.second) {
    // already inserted. => update stat.
    (*r.first).second = us;
  }
  return r.second;
}


UrlStatMap::iterator UrlStatMap::find_with_expire(const char* path)
{
  iterator it = find(path);
  if(it==end()) return it;
  if((*it).second.is_valid()) return it;
  erase(it);
  return end();
}


void UrlStatMap::trim(size_t count)
{
  for(; count>0; count--) {
    iterator it = begin();
    erase(it);
  }
}


void UrlStatMap::dump(Log& logger)
{
  logger(Log::NOTE, "[UrlStatMap]\n");
  UrlStatMap::iterator it = begin();
  for(; it!=end(); it++) {
    logger(Log::NOTE, "  %8o/%10"FINT64"u - (%p) %s\n",
          (*it).second.mode, (*it).second.length, (*it).first.c_str(), (*it).first.c_str());
  }
  logger(Log::NOTE, "=== Total: %"FSIZET"u items.\n", size());
}



// UrlStatCache class implements.
void UrlStatCache::init()
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); 
  pthread_mutex_init(&m_lock, &attr);

  m_stop_cleaner = false;
  pthread_create(&m_cleaner, NULL, cleaner, (void*)this);
  
  glog(Log::INFO, "UrlStatCache::m_lock=%p\n", &m_lock);
}


void UrlStatCache::stop()
{
  void* ret;
  m_stop_cleaner = true;
  signal(SIGUSR2, alarm_handler);
  pthread_kill(m_cleaner, SIGUSR2);
  pthread_join(m_cleaner, &ret);
}


void UrlStatCache::add(const char* path, mode_t mode, uint64_t length)
{
  const char* fmt = "";

  pthread_mutex_lock(&m_lock);
  {
    UrlStat us(mode, length);
    bool r = m_stats.insert(path, us);
    if(r) {
      fmt = "@@ Append cache: path=%s, %o, %"FINT64"u\n";
    } else {
      fmt = "@@ Update cache: path=%s, %o, %"FINT64"u\n";
    }
  }
  pthread_mutex_unlock(&m_lock);
  glog(Log::DEBUG, fmt, path, mode, length);
}


bool UrlStatCache::find(const char* path, UrlStat& stat)
{
  bool result = false;

  pthread_mutex_lock(&m_lock);
  {
    UrlStatMap::iterator it = m_stats.find_with_expire(path);
    if(it!=m_stats.end()) {
      stat = (*it).second;
      result = true;
      glog(Log::DEBUG, "@@ Cache found(%s==%s)=>%o/%"FINT64"u\n", (*it).first.c_str(), path, stat.mode, stat.length);;
    }
  }
  pthread_mutex_unlock(&m_lock);
  if(result) add(path, stat.mode, stat.length);

  return result;
}


void UrlStatCache::trim()
{
  // [ToDo] 今はpathが長いものを優先的に削除している。
  // ディレクトリの途中パスもアクセス＆格納されるが、
  // これらは再利用されやすいであろうという推測。
  // これらを削除する前に UrlStat.expire が無効なものを削除すべき。

  while(m_stats.size()>CACHE_MAX_ENTRIES) {
    size_t sub = m_stats.size() - CACHE_MAX_ENTRIES;
    size_t delta = (sub<50)? 5: ((sub<200)? sub/10: 20);
    for(size_t b=0; b<sub; b+=delta) {
      pthread_mutex_lock(&m_lock);
      {
        m_stats.trim(delta);
      }
      pthread_mutex_unlock(&m_lock);
    }
  }
  glog(Log::NOTE, "UrlStatCache::trim() => %"FSIZET"u\n", m_stats.size());
}


void UrlStatCache::dump(Log& logger)
{
  logger(Log::INFO, "==== UrlStatCache dump ====\n");

  pthread_mutex_lock(&m_lock);
  {
    m_stats.dump(logger);
  }
  pthread_mutex_unlock(&m_lock);
}


void* UrlStatCache::cleaner(void* ctx)
{
  UrlStatCache* self = (UrlStatCache*)ctx;

  while(!self->m_stop_cleaner) {
    self->trim();
    sleep(5);
  }
  return NULL;
}


void UrlStatCache::alarm_handler(int sig)
{
  // Do nothing.
}

// vim: sw=2 sts=2 ts=4 expandtab :
