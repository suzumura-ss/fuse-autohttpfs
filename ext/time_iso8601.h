/*
  Copyright 2010 Toshiyuki Terashita.

  class TimeIso8601 is free software: you can redistribute it and/or modify
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

#ifndef __INCLUDE_TIME_ISO8601_H__
#define __INCLUDE_TIME_ISO8601_H__

#include <features.h>
#include <time.h>
#include <string>


class TimeIso8601
{
public:
  inline TimeIso8601(time_t v) { self = v; };
  inline TimeIso8601(const char* _str) { operator=(_str); }

  inline operator time_t() { return self; };
  operator std::string() const;

  inline TimeIso8601& operator=(time_t v) {
    self = v;
    return *this;
  };
  TimeIso8601& operator=(const char* _str);
  inline TimeIso8601& operator=(const std::string& _str) {
    return operator=(_str.c_str());
  };

private:
  time_t  self;
};  


#endif // __INCLUDE_TIME_ISO8601_H__
// vim: sw=2 sts=2 ts=4 expandtab :
