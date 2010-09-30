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

#include "time_iso8601.h"
#include <string.h>


// TZ GMT-offset by sec
class LocalTimeZoneOffset
{
public:
  LocalTimeZoneOffset() {
    time_t t = 0;
    struct tm r;
    memset(&r, 0, sizeof(r));
    localtime_r(&t, &r);
    offset = r.tm_gmtoff;
  };
  int operator()() const { return offset; };

private:
  int offset;
};
static LocalTimeZoneOffset gTzOffset;
    


// class TimeIso8601 implements.
TimeIso8601& TimeIso8601::operator=(const char* _str)
{
  if(_str) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    _str = strptime(_str, "%Y-%m-%dT%H:%M:%S%z", &t);
    if(!_str) throw "TimeIso8601#operator=(): parse string failed.";
    int ofs = t.tm_gmtoff - gTzOffset();
    time_t r = mktime(&t);
    self = r + ofs;
  } else {
    self = time(NULL);
  }
  return *this;
}


TimeIso8601::operator std::string() const
{
  struct tm t;
  memset(&t, 0, sizeof(t));
  localtime_r(&self, &t);

  char r[64];
  strftime(r, sizeof(r), "%Y-%m-%dT%H:%M:%S%z", &t);
  return std::string(r);
}

// vim: sw=2 sts=2 ts=4 expandtab :
