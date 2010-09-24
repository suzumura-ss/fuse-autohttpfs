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
#include "context.h"
#include "int64format.h"


// AutoHttpFsContext class implements.
AutoHttpFsContext::AutoHttpFsContext(uint64_t seq, RemoteAttr& attr)
{
  m_seq = seq;
  m_attr = &attr;
}


AutoHttpFsContext::~AutoHttpFsContext()
{
}



// AutoHttpFsContexts class implements.
AutoHttpFsContexts::AutoHttpFsContexts(AutoHttpFs* fs)
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); 
  pthread_mutex_init(&m_lock, &attr);

  m_fs = fs;
  m_contexts.clear();
  sequence = 1;
}


AutoHttpFsContexts::~AutoHttpFsContexts()
{
}


AutoHttpFsContext* AutoHttpFsContexts::alloc_context()
{
  AutoHttpFsContext* ctx = NULL;

  pthread_mutex_lock(&m_lock);
  {
    ctx = new AutoHttpFsContext(sequence, m_attr);
    m_contexts.insert(AutoHttpFsContextMap::value_type(sequence, ctx));
    sequence++;
  }
  pthread_mutex_unlock(&m_lock);
  return ctx;
}


void AutoHttpFsContexts::release_context(AutoHttpFsContext* ctx)
{
  pthread_mutex_lock(&m_lock);
  {
    m_contexts.erase(ctx->seq());
  }
  pthread_mutex_unlock(&m_lock);

  delete ctx;
}


AutoHttpFsContext* AutoHttpFsContexts::find(uint64_t seq)
{
  AutoHttpFsContextMap::iterator it;
  pthread_mutex_lock(&m_lock);
  {
    it = m_contexts.find(seq);
  }
  pthread_mutex_unlock(&m_lock);
  if(it!=m_contexts.end()) return (*it).second;

  return NULL;
}

// vim: sw=2 sts=2 ts=4 expandtab :
