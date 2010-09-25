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
#include "int64format.h"

#ifndef CACHE_MAX_ENTRIES
# define CACHE_MAX_ENTRIES (2000)
#endif


// UrlStatMap class implements.
bool UrlStatMap::insert(const char* path, const UrlStat& us)
{
  Url key(path);
  std::pair<UrlStatBASE::iterator, bool> r = UrlStatBASE::insert(std::make_pair(key, us));
  if(r.second) {
    // inserted.
    m_entries.push_back(r.first);
  } else {
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
  return end();
}


void UrlStatMap::trim(size_t count)
{
  for(; count>0; count--) {
    if(m_entries.size()==0) break;
    iterator it = m_entries.front();
    UrlStatBASE::erase(it);
    m_entries.pop_front();
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
  pthread_mutex_init(&m_lock, &attr);

  m_max_entries = CACHE_MAX_ENTRIES;
  m_stop_cleaner = false;
  signal(SIGUSR2, alarm_handler);
  pthread_create(&m_cleaner, NULL, cleaner, (void*)this);
}


void UrlStatCache::stop()
{
  void* ret;
  m_stop_cleaner = true;
  pthread_kill(m_cleaner, SIGUSR2);
  pthread_join(m_cleaner, &ret);
}


void UrlStatCache::add(const char* path, const UrlStat& stat)
{
  time_t expire = time(NULL) + m_expire_sec;
  bool over_capacity = false;

  pthread_mutex_lock(&m_lock);
  {
    UrlStat us = stat;
    us.expire = expire;
    if(m_stats.insert(path, us) && (m_stats.size()>m_max_entries)) {
      over_capacity = true;
    }
  }
  pthread_mutex_unlock(&m_lock);

  if(over_capacity) pthread_kill(m_cleaner, SIGUSR2);
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
    }
  }
  pthread_mutex_unlock(&m_lock);
  if(result) add(path, stat.mode, stat.length);

  return result;
}


void UrlStatCache::trim()
{
  while(m_stats.size()>m_max_entries) {
    size_t sub = m_stats.size() - m_max_entries;
    size_t delta = (sub<50)? 5: ((sub<200)? sub/10: 20);
    for(size_t b=0; b<sub; b+=delta) {
      pthread_mutex_lock(&m_lock);
      {
        m_stats.trim(delta);
      }
      pthread_mutex_unlock(&m_lock);
    }
  }
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
