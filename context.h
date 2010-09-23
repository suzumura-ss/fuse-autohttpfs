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


class AutoHttpFsContext
{
public:
  AutoHttpFsContext(uint64_t seq, RemoteAttr& attr);
  virtual ~AutoHttpFsContext();
  inline uint64_t seq() const { return m_seq; };
  inline int get_attr(const char* path, UrlStat& stat) { return m_attr->get_attr(path, stat); };

private:
  uint64_t m_seq;
  RemoteAttr* m_attr;
};
typedef std::map<uint64_t, AutoHttpFsContext*> AutoHttpFsContextMap;


class AutoHttpFsContexts
{
public:
  AutoHttpFsContexts();
  virtual ~AutoHttpFsContexts();
  inline static AutoHttpFsContexts* ctxs() {
    fuse_context* fc = fuse_get_context();
    return (AutoHttpFsContexts*)(fc->private_data);
  };
  AutoHttpFsContext* alloc_context();
  void	release_context(AutoHttpFsContext* ctx);
  AutoHttpFsContext* find(uint64_t seq);
  inline int get_attr(const char* path, UrlStat& stat) { return m_attr.get_attr(path, stat); };

private:
  pthread_mutex_t m_lock;
  AutoHttpFsContextMap m_contexts;
  uint64_t	sequence;
  int64_t   active_fds;
  RemoteAttr m_attr;
};
#define	AUTOHTTPFSCONTEXTS	(*AutoHttpFsContexts::ctxs())


#endif // __INCLUDE_CONTEXT_H__
// vim: sw=2 sts=2 ts=4 expandtab :
