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

#include "log.h"


// Log class implements.
Log::Log(const char* ident, int facility, LOGLEVEL level)
{
  m_level = level;
  openlog(ident, LOG_NDELAY, facility);
}


Log::~Log()
{
  closelog();
}


void Log::vlog(int level, const char* fmt, va_list va) const
{
  if(level>m_level) return;
  vsyslog(level, fmt, va);
  vfprintf(stdout, fmt, va);
  fflush(stdout);
}


void Log::operator() (int level, const char* fmt, ...) const
{
  va_list va;
  va_start(va, fmt);
  vlog(level, fmt, va);
  va_end(va);
}

// vim: sw=2 sts=2 ts=4 expandtab :
