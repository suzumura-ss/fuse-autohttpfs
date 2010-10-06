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

#ifndef __INCLUDE_AUTOHTTPFSPROCMAP_H__
#define __INCLUDE_AUTOHTTPFSPROCMAP_H__

#include <string>
#include <map>
#include "proc.h"
#include "log.h"


typedef std::map<std::string, ProcAbstract*> ProcMap;

class AutoHttpFsProc
{
public:
  inline AutoHttpFsProc() {};
  inline virtual ~AutoHttpFsProc() {};

  int getattr(Log& logger, const char* path, struct stat& stbuf);
  int opendir(Log& logger, const char* path, struct fuse_file_info& ffi);
  int truncate(Log& logger, const char* path, off_t size);
  int open(Log& logger, const char* path, struct fuse_file_info& ffi);
  void init();

private:
  ProcMap m_procs;
  void mount(const char* name, ProcAbstract* proc, Proc_Dir* base = NULL);
};


#endif // __INCLUDE_AUTOHTTPFSPROCMAP_H__
// vim: sw=2 sts=2 ts=4 expandtab :
