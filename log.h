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

#ifndef __INCLUDE_LOG_H__
#define __INCLUDE_LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>


class Log
{
public:
  typedef enum {
    VERBOSE = LOG_DEBUG+1,
    DEBUG = LOG_DEBUG,
    DEVELOP = DEBUG,
    INFO = LOG_INFO,
    NOTE = LOG_NOTICE,
    WARN = LOG_WARNING,
    ERR  = LOG_ERR,
    CRIT = LOG_CRIT,
    ALRT = LOG_ALERT,
  } LOGLEVEL;

  Log(const char* ident, int facility = LOG_LOCAL7, LOGLEVEL level = DEVELOP);
  virtual ~Log();
  inline void set_level(LOGLEVEL level) { m_level = level; };
  void vlog(int level, const char* fmt, va_list va) const __attribute__ ((__format__ (__printf__, 3, 0)));
  void operator() (int level, const char* fmt, ...) const __attribute__ ((__format__ (__printf__, 3, 4)));

private:
  LOGLEVEL  m_level;
};


#endif // __INCLUDE_LOG_H__
// vim: sw=2 sts=2 ts=4 expandtab :
