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

#include <stdlib.h>
#include <string.h>
#include "proc.h"
#include "log.h"
#include "context.h"
#include "int64format.h"


static std::string uint64_to_str(uint64_t v)
{
  char t[64];
  snprintf(t, sizeof(t), "%"FINT64"u\n", v);
  return std::string(t);
}



// Proc_Dir class implements.
int Proc_Dir::getattr(Log& logger, struct stat& stbuf)
{
  stbuf.st_mode = S_IFDIR|S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
  stbuf.st_atime = stbuf.st_ctime = stbuf.st_mtime = time(NULL);
  return 0;
}


int Proc_Dir::opendir(Log& logger, ProcAbstract*& self)
{
  self = (ProcAbstract*)this;
  return 0;
}


int Proc_Dir::readdir(Log& logger, void *buf, fuse_fill_dir_t filler, off_t offset)
{
  std::vector<std::string>::iterator it;
  for(it = m_dirs.begin(); it!=m_dirs.end(); it++) {
    filler(buf, (*it).c_str(), NULL, 0);
  }
  return 0;
}



// Proc_StringStream class implements.
int Proc_StringStream::getattr(Log& logger, struct stat& stbuf)
{
  stbuf.st_mode = S_IFREG|S_IRUSR|S_IRGRP|S_IROTH;
  stbuf.st_atime = stbuf.st_ctime = stbuf.st_mtime = time(NULL);
  stbuf.st_size = m_string.size();
  return 0;
}


int Proc_StringStream::read(Log& logger, char* buf, size_t size, off_t offset)
{
  if(m_string.size()<size+offset) {
    size = (m_string.size()>offset)? m_string.size()-offset: 0;
  }
  if(size>0) memcpy(buf, m_string.c_str()+offset, size);
  return size;
}



// Proc_StringStreamIO class implements.
int Proc_StringStreamIO::getattr(Log& logger, struct stat& stbuf)
{
  Proc_StringStream::getattr(logger, stbuf);
  stbuf.st_mode |= S_IWUSR;
  return 0;
}


int Proc_StringStreamIO::write(Log& logger, const char* buf, size_t size, off_t offset)
{
  // Can append or replace only.
  if(offset==0) m_string.resize(0);
  if(m_string.size()==offset) {
    m_string.append(buf, size);
    m_wrote += size;
    return size;
  }
  return -EINVAL;
}




// Proc_CacheEnable class implements.
int Proc_CacheEnable::open(Log& logger, ProcAbstract*& self)
{
  m_string = uint64_to_str(AUTOHTTPFSCONTEXTS.remote_attr().cache().enabled());
  m_wrote = 0;
  self = this;
  return 0;
}


int Proc_CacheEnable::release(Log& logger)
{
  if(m_wrote) {
    int64_t mode = strtoll(m_string.c_str(), NULL, 10);
    AUTOHTTPFSCONTEXTS.remote_attr().cache().enabled(mode);
    logger(Log::NOTE, "Set cache::enable to %"FINT64"d\n", mode);
  }
  Proc_StringStream::release(logger);
  return 0;
}



// Proc_CacheMaxEntries class implements.
int Proc_CacheMaxEntries::open(Log& logger, ProcAbstract*& self)
{
  m_string = uint64_to_str(AUTOHTTPFSCONTEXTS.remote_attr().cache().max_entries());
  m_wrote = 0;
  self = this;
  return 0;
}


int Proc_CacheMaxEntries::release(Log& logger)
{
  if(m_wrote) {
    int64_t size = strtoll(m_string.c_str(), NULL, 10);
    AUTOHTTPFSCONTEXTS.remote_attr().cache().max_entries(size);
    logger(Log::NOTE, "Set cache::max_entries to %"FINT64"d\n", size);
  }
  Proc_StringStream::release(logger);
  return 0;
}



// Proc_CacheEntries class implements.
int Proc_CacheEntries::open(Log& logger, ProcAbstract*& self)
{
  m_string = uint64_to_str(AUTOHTTPFSCONTEXTS.remote_attr().cache().size());
  self = this;
  return 0;
}



// Proc_CacheExpire class implements.
int Proc_CacheExpire::open(Log& logger, ProcAbstract*& self)
{
  m_string = uint64_to_str(AUTOHTTPFSCONTEXTS.remote_attr().cache().expire());
  m_wrote = 0;
  self = this;
  return 0;
}


int Proc_CacheExpire::release(Log& logger)
{
  if(m_wrote) {
    int64_t sec = strtoll(m_string.c_str(), NULL, 10);
    AUTOHTTPFSCONTEXTS.remote_attr().cache().expire(sec);
    logger(Log::NOTE, "Set cache::expire to %"FINT64"d\n", sec);
  }
  Proc_StringStream::release(logger);
  return 0;
}



// Proc_LogLevel class implements.
int Proc_LogLevel::open(Log& logger, ProcAbstract*& self)
{
  m_string = uint64_to_str(logger.loglevel());
  m_wrote = 0;
  self = this;
  return 0;
}
int Proc_LogLevel::release(Log& logger)
{
  if(m_wrote) {
    int64_t lev = strtoll(m_string.c_str(), NULL, 10);
    logger.loglevel((Log::LOGLEVEL)lev);
    logger(Log::NOTE, "Set loglevel to %"FINT64"d\n", lev);
  }
  Proc_StringStream::release(logger);
  return 0;
}

// vim: sw=2 sts=2 ts=4 expandtab :
