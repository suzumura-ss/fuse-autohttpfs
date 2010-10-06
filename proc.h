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

#ifndef __INCLUDE_AUTOHTTPFSPROC_H__
#define __INCLUDE_AUTOHTTPFSPROC_H__

#ifndef FUSE_USE_VERSION
# define FUSE_USE_VERSION 26
#endif
#include <fuse.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string>
#include <vector>
#include "log.h"


// Base class of proc/ implements.
class ProcAbstract
{
public:
  inline ProcAbstract() {};
  inline ProcAbstract(const ProcAbstract* s) {};
  inline virtual ~ProcAbstract() {};
  inline virtual int getattr(Log& logger, struct stat& stbuf) { return -ENOSYS; };
  inline virtual int opendir(Log& logger, ProcAbstract*& self) { return -ENOSYS; };
  inline virtual int readdir(Log& logger, void *buf, fuse_fill_dir_t filler, off_t offset) { return -EINVAL; };
  inline virtual int releasedir(Log& logger) { return -EINVAL; };
  inline virtual int truncate(Log& logger, off_t size) { return -ENOSYS; };
  inline virtual int open(Log& logger, ProcAbstract*& self) { return -ENOSYS; };
  inline virtual int read(Log& logger, char* buf, size_t size, off_t offset) { return -EINVAL; };
  inline virtual int write(Log& logger, const char* buf, size_t size, off_t offset) { return -EINVAL; };
  inline virtual int flush(Log& logger, struct fuse_file_info* ffi) { return -EINVAL; };
  inline virtual int release(Log& logger) { return -EINVAL; };
  inline virtual const char* name() { return "ProcAbstract"; };
};


// Static directory for proc/.
class Proc_Dir: public ProcAbstract
{
public:
  inline Proc_Dir(const char* path): m_path(path) { };
  inline Proc_Dir(const Proc_Dir& root, const char* path) {
    m_path = root.path();
    m_path += path;
  };
  inline const std::string& path() const { return m_path; };
  inline void mount(const std::string& name) { m_dirs.push_back(name); };
  virtual int getattr(Log& logger, struct stat& stbuf);
  virtual int opendir(Log& logger, ProcAbstract*& self);
  virtual int readdir(Log& logger, void *buf, fuse_fill_dir_t filler, off_t offset);
  inline virtual int release(Log& logger) { return 0; };
  inline virtual const char* name() { return "Proc_Dir"; };

private:
  std::string m_path;
  std::vector<std::string> m_dirs;
};


// Base class with std::string. It works read-only.
class Proc_StringStream: public ProcAbstract
{
public:
  inline Proc_StringStream() {};
  virtual int getattr(Log& logger, struct stat& stbuf);
  virtual int read(Log& logger, char* buf, size_t size, off_t offset);
  inline virtual int release(Log& logger) { return 0; };
  inline virtual const char* name() { return "Proc_StringStream"; };

protected:
  std::string m_string;
};


// Base class with std::string. It works read/write.
class Proc_StringStreamIO: public Proc_StringStream
{
public:
  inline Proc_StringStreamIO(): m_wrote(0) { };
  virtual int getattr(Log& logger, struct stat& stbuf);
  inline virtual int truncate(Log& logger, off_t size) { m_string.resize(0); return 0; };
  virtual int write(Log& logger, const char* buf, size_t size, off_t offset);
  inline virtual int flush(Log& logger, struct fuse_file_info* ffi) {
    logger(Log::DEBUG, "Proc_StringStreamIO::flush: ffi.flush = %d\n", ffi->flush);
    return 0;
  }
  inline virtual const char* name() { return "Proc_StringStreamIO"; };

protected:
  uint64_t m_wrote;
};


// Cache enable/disable control.
class Proc_CacheEnable: public Proc_StringStreamIO
{
public:
  inline Proc_CacheEnable() {};
  virtual int open(Log& logger, ProcAbstract*& self);
  virtual int release(Log& logger);
  inline virtual const char* name() { return "Proc_CacheEnable"; };
};


// Return current cache entries.
class Proc_CacheEntries: public Proc_StringStream
{
public:
  inline Proc_CacheEntries() {};
  virtual int open(Log& logger, ProcAbstract*& self);
  inline virtual const char* name() { return "Proc_CacheEntries"; };
};


// Cache expire control.
class Proc_CacheExpire: public Proc_StringStreamIO
{
public:
  inline Proc_CacheExpire() {};
  virtual int open(Log& logger, ProcAbstract*& self);
  virtual int release(Log& logger);
  inline virtual const char* name() { return "Proc_CacheExpire"; };
};


// Return/Set max of cache entries.
class Proc_CacheMaxEntries: public Proc_StringStreamIO
{
public:
  inline Proc_CacheMaxEntries() {};
  virtual int open(Log& logger, ProcAbstract*& self);
  virtual int release(Log& logger);
  inline virtual const char* name() { return "Proc_CacheMaxEntries"; };
};


// Return/Set log level (syslog(3)).
class Proc_LogLevel: public Proc_StringStreamIO
{
public:
  inline Proc_LogLevel() {};
  virtual int open(Log& logger, ProcAbstract*& self);
  virtual int release(Log& logger);
  inline virtual const char* name() { return "Proc_LogLevel"; };
};


#endif // __INCLUDE_AUTOHTTPFSPROC_H__
// vim: sw=2 sts=2 ts=4 expandtab :
