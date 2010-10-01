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

#include <errno.h>
#include "remoteattr.h"
#include "curlaccessor.h"
#include "filestat.h"
#include "ext/time_iso8601.h"


// RemoteAttr class implements.
void RemoteAttr::store(UrlStat& stat, const char* path, mode_t mode, std::string x_filestat)
{
  if(!x_filestat.empty()) {
    try {
      FileStat fs(x_filestat);
      stat = UrlStat(fs.mode, fs.size, fs.mtime);
      m_cache.add(path, stat);
    }
    catch(std::string e) {
      m_cache.add(path, stat);
      throw e;
    }
  } else {
    m_cache.add(path, stat);
  }
}


int RemoteAttr::get_attr(Log& logger, const char* path, UrlStat& stat)
{
  // check 'root' directory.
  if(strcmp(path, "/")==0) {
    stat = UrlStat(S_IFDIR);
    return 0;
  }

  // check dot.path.
  if(strncmp(path, "/.", 2)==0) {
    return -ENOENT;
  }

  // check cache.
  if(m_cache.find(path, stat)) {
    TimeIso8601 t(stat.mtime);
    logger(Log::DEBUG, "   RemoteAttr::get_attr(%s:FIND): %05o %s\n", path, stat.mode, ((std::string)t).c_str());
    return 0;
  }

  // challenge "path/" to directory.
  {
    CurlAccessor ca(path);
    ca.add_header("Accept", "text/json");
    int res = ca.head(logger);
    if((res==200) || (res==403)) {
      // path should be directory.
      try{ store(stat, path, S_IFDIR, ca.x_filestat()); }
      catch(std::string e) {
        logger(Log::WARN, "   RemoteAttr::get_attr(%s:DIR): %s\n", path, e.c_str());
      }
      return 0;
    }
  }

  // challenge "path" to regular file.
  {
    CurlAccessor ca(path, false);
    ca.add_header("Accept", "text/json");
    int res = ca.head(logger);
    if(res==200) {
      // path is regular file.
      try{ store(stat, path, S_IFREG, ca.x_filestat()); }
      catch(std::string e) {
        logger(Log::WARN, "   RemoteAttr::get_attr(%s:REG): %s\n", path, e.c_str());
      }
      return 0;
    }
  }

  return -ENOENT;
}

// vim: sw=2 sts=2 ts=4 expandtab :
