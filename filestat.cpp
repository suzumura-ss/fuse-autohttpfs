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

#include <ctype.h>
#include "ext/time_iso8601.h"
#include "filestat.h"


// convert '[dl-][rwx]{3}' to mode_t.
static mode_t str_to_mode(const std::string str)
{
  mode_t mode = 0;

  switch(str[0]) {
  case 'd':
    mode = S_IFDIR;
    break;
  case 'l':
    mode = S_IFLNK;
    break;
  case '-':
  default:
    mode = S_IFREG;
    break;
  case '\0':
    break;
  }
  if(str.size()!=10) return mode;

  if(str[1]=='r') mode |= S_IRUSR;
  if(str[2]=='w') mode |= S_IWUSR;
  if(str[3]=='x') mode |= S_IXUSR;
  if(str[4]=='r') mode |= S_IRGRP;
  if(str[5]=='w') mode |= S_IWGRP;
  if(str[6]=='x') mode |= S_IXGRP;
  if(str[7]=='r') mode |= S_IROTH;
  if(str[8]=='w') mode |= S_IWOTH;
  if(str[9]=='x') mode |= S_IXOTH;

  return mode;
}


void FileStat::from_json(std::string& json)
{
  std::string::iterator j = json.begin();
  while(isspace(*j)) j++;
  if((*j)!='{') throw std::string("Hash is required.");

  std::string err;
  picojson::value root;
  picojson::parse<std::string::iterator>(root, json.begin(), json.end(), &err);
  if(!err.empty()) throw err;

  picojson::object obj = root.get<picojson::object>();
  picojson::object::iterator it = obj.begin();
  if(it==obj.end()) throw std::string("Empty hash.");

  std::pair<std::string, picojson::value> val = (*it);
  name = val.first;
  from_json(val.second);
}


void FileStat::from_json(picojson::value& val)
{
  mode = str_to_mode(val.get("mode").get<std::string>());
  size = (uint64_t)(val.get("size").get<double>());
  mtime = TimeIso8601(val.get("mtime").get<std::string>().c_str());
}
  

// vim: sw=2 sts=2 ts=4 expandtab :
