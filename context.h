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

#ifndef __INCLUDE_CONTEXT_H__
#define __INCLUDE_CONTEXT_H__

#include <stdint.h>
#include <pthread.h>
#include <string>
#include <map>
#include <fuse/fuse.h>
#include "remoteattr.h"
#include "procmap.h"


class AutoHttpFs;
class AutoHttpFsContext
{
public:
  AutoHttpFsContext(uint64_t seq, RemoteAttr& attr);
  virtual ~AutoHttpFsContext();
  inline uint64_t seq() const { return m_seq; };
  inline int get_attr(Log& logger, const char* path, UrlStat& stat) {
    return m_attr->get_attr(logger, path, stat);
  };
  inline RemoteAttr& attr() { return *m_attr; };

public:
  ProcAbstract* proc;

private:
  uint64_t m_seq;
  RemoteAttr* m_attr;
};
typedef std::map<uint64_t, AutoHttpFsContext*> AutoHttpFsContextMap;


class AutoHttpFsContexts
{
public:
  AutoHttpFsContexts(AutoHttpFs* fs);
  virtual ~AutoHttpFsContexts();
  inline static AutoHttpFsContexts* ctxs() {
    fuse_context* fc = fuse_get_context();
    return (AutoHttpFsContexts*)(fc->private_data);
  };
  inline RemoteAttr& remote_attr() { return m_attr; };
  AutoHttpFsContext* alloc_context();
  void	release_context(AutoHttpFsContext* ctx);
  AutoHttpFsContext* find(uint64_t seq);
  inline int get_attr(Log& logger, const char* path, UrlStat& stat) {
    return m_attr.get_attr(logger, path, stat);
  };
  inline AutoHttpFs* fs() const { return m_fs; };

  inline int proc_getattr(Log& logger, const char* path, struct stat& stbuf) {
    return m_proc.getattr(logger, path, stbuf);
  };
  inline int proc_opendir(Log& logger, const char* path, struct fuse_file_info& ffi) {
    return m_proc.opendir(logger, path, ffi);
  };
  inline int proc_truncate(Log& logger, const char* path, off_t size) {
    return m_proc.truncate(logger, path, size);
  };
  inline int proc_open(Log& logger, const char* path, struct fuse_file_info& ffi) {
    return m_proc.open(logger, path, ffi);
  };

private:
  AutoHttpFs* m_fs;
  pthread_mutex_t m_lock;
  AutoHttpFsContextMap m_contexts;
  uint64_t	sequence;
  int64_t   active_fds;
  RemoteAttr m_attr;
  AutoHttpFsProc m_proc;
};
#define	AUTOHTTPFSCONTEXTS	(*AutoHttpFsContexts::ctxs())


#endif // __INCLUDE_CONTEXT_H__
// vim: sw=2 sts=2 ts=4 expandtab :
