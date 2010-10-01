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

#ifndef __INCLUDE_REMOREATTR_H__
#define __INCLUDE_REMOREATTR_H__

#include "cache.h"
#include "log.h"


class RemoteAttr
{
public:
  inline RemoteAttr() {
    m_cache.init();
  };
  inline virtual ~RemoteAttr() {
    try { m_cache.stop(); }
    catch(...){}
  };
  int get_attr(Log& logger, const char* path, UrlStat& stat);
  inline void remove_attr(Log& logger, const char* path) {
    m_cache.remove(path);
  };

private:
  UrlStatCache  m_cache;
  void store(UrlStat& stat, const char*path, mode_t mode, std::string x_filestat);
};


#endif // __INCLUDE_REMOREATTR_H__
// vim: sw=2 sts=2 ts=4 expandtab :
