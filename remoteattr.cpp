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


// RemoteAttr class implements.
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
    return 0;
  }

  // challenge "path/" to directory.
  {
    CurlAccessor ca(path);
    ca.add_header("Accept", "text/json");
    int res = ca.head(logger);
    if((res==200) || (res==403)) {
      // path should be directory.
      stat = UrlStat(S_IFDIR);
      m_cache.add(path, stat);
      return 0;
    }
  }

  // challenge "path" to regular file.
  {
    CurlAccessor ca(path, false);
    int res = ca.head(logger);
    if(res==200) {
      // path is regular file.
      stat = UrlStat(S_IFREG, ca.content_length());
      m_cache.add(path, stat);
      return 0;
    }
  }

  return -ENOENT;
}

// vim: sw=2 sts=2 ts=4 expandtab :
