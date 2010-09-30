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

#ifndef __INCLUDE_FILESTAT_H__
#define __INCLUDE_FILESTAT_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include "ext/picojson.h"


class FileStat
{
public:
  inline FileStat(mode_t m = S_IFDIR, uint64_t s = 0, time_t t = 0) {
    mode = m; size = s; mtime = t;
  };
  inline FileStat(std::string& json) { from_json(json); };
  inline FileStat(picojson::value& val) { from_json(val); };
  inline virtual ~FileStat() {};
  void from_json(std::string& json);
  void from_json(picojson::value& val);

public:
  std::string name;
  mode_t    mode;
  uint64_t  size;
  time_t    mtime;
};


#endif // __INCLUDE_FILESTAT_H__
// vim: sw=2 sts=2 ts=4 expandtab :
