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

#include "autohttpfs.h"
#include "context.h"
#include "log.h"
#include "globals.h"
#include <mcheck.h>


// Globals.
AutoHttpFs gConfig;
Log glog("autohttpfs", LOG_LOCAL7, Log::DEVELOP);



// main loop.
int main(int argc, char* argv[])
{
  static fuse_operations oper;
  AutoHttpFs::init_fuse_operations(oper);

  gConfig.parse_args(argc, argv);
  gConfig.setup();
  glog.set_level(Log::NOTE);

  return fuse_main(argc, argv, &oper, NULL);
}

// vim: sw=2 sts=2 ts=4 expandtab :
