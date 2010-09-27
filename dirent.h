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

#ifndef __INCLUDE_DIRENT_H__
#define __INCLUDE_DIRENT_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext/picojson.h"


class Direntry
{
public:
  inline Direntry() { mode = 0; size = 0; };
  inline virtual ~Direntry() {};
  void set_mode(const std::string& str);

public:
  std::string name;
  mode_t  mode;
  uint64_t size;
};


class Direntries: public std::vector<Direntry>
{
public:
  inline Direntries() {};
  bool from_json(std::string& json, std::string& err);
};


#endif // __INCLUDE_DIRENT_H__
// vim: sw=2 sts=2 ts=4 expandtab :
