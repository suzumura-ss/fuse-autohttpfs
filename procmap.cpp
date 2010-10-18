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

#include <error.h>
#include "autohttpfs.h"
#include "context.h"
#include "proc.h"
#include "procmap.h"
#include "int64format.h"


// class AutoHttpFsProc implements.

// proc::getattr
int AutoHttpFsProc::getattr(Log& logger, const char* path, struct stat& stbuf)
{
  ProcMap::iterator it = m_procs.find(path);
  if(it==m_procs.end()) return -ENOENT;
  logger(Log::DEBUG, "--> AutoHttpFsProc::getattr(%s) => %s\n", path, (*it).second->name());

  stbuf.st_dev = 0;
  stbuf.st_ino = 0;
  stbuf.st_nlink = 1;
  stbuf.st_uid = geteuid();
  stbuf.st_gid = getegid();

  ProcAbstract* proc = NULL;
  int o = (*it).second->open(logger, proc);
  int r = (*it).second->getattr(logger, stbuf);
  if(o==0) proc->release(logger);
  return r;
}


// proc::opendir
int AutoHttpFsProc::opendir(Log& logger, const char* path, struct fuse_file_info& ffi)
{
  ProcMap::iterator it = m_procs.find(path);
  if(it==m_procs.end()) return -ENOENT;
  logger(Log::DEBUG, "--> AutoHttpFsProc::opendir(%s) => %s\n", path, (*it).second->name());

  ProcAbstract* proc = NULL;
  int r = (*it).second->opendir(logger, proc);
  if(r==0) {
    AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
    ctx->proc = proc;
    ffi.fh = ctx->seq();
    logger(Log::DEBUG, "   => fh=%"FINT64"d, ctx=%p, proc=%p\n", ffi.fh, ctx, proc);
  }
  return r;
}


// proc::truncate
int AutoHttpFsProc::truncate(Log& logger, const char* path, off_t size)
{
  ProcMap::iterator it = m_procs.find(path);
  if(it==m_procs.end()) return -ENOENT;
  logger(Log::DEBUG, "--> AutoHttpFsProc::truncate(%s) => %s\n", path, (*it).second->name());

  return (*it).second->truncate(logger, size);
}


// proc::open
int AutoHttpFsProc::open(Log& logger, const char* path, struct fuse_file_info& ffi)
{
  ProcMap::iterator it = m_procs.find(path);
  if(it==m_procs.end()) return -ENOENT;
  logger(Log::DEBUG, "--> AutoHttpFsProc::open(%s) => %s\n", path, (*it).second->name());

  ProcAbstract* proc = NULL;
  int r = (*it).second->open(logger, proc);
  if(r==0) {
    AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
    ctx->proc = proc;
    ffi.fh = ctx->seq();
    logger(Log::DEBUG, "   => fh=%"FINT64"d, ctx=%p, proc=%p\n", ffi.fh, ctx, proc);
  }
  return r;
}


// initialize proc/ entries.
void AutoHttpFsProc::init()
{
  Proc_Dir *root, *cache;
  mount(".proc", root = new Proc_Dir("/.proc"));
  mount("cache", cache = new Proc_Dir(*root, "/cache"), root);
  mount("enable", new Proc_CacheEnable(), cache);
  mount("entries", new Proc_CacheEntries(), cache);
  mount("max_entries", new Proc_CacheMaxEntries(), cache);
  mount("expire", new Proc_CacheExpire(), cache);
  mount("loglevel", new Proc_LogLevel(), cache);
}


// append entry.
void AutoHttpFsProc::mount(const char* name, ProcAbstract* proc, Proc_Dir* base)
{
  std::string path;
  if(base) {
    base->mount(name);
    path = base->path();
  }
  path += "/";
  path += name;
  std::pair<ProcMap::iterator, bool> r = m_procs.insert(std::make_pair(path, proc));
}

// vim: sw=2 sts=2 ts=4 expandtab :
