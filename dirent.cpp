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

#include <string.h>
#include "dirent.h"


// class Direntry implements.
void Direntry::set_mode(const std::string& str)
{
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
  if(str.size()!=11) return;

  if(str[1]=='r') mode |= S_IRUSR;
  if(str[2]=='w') mode |= S_IWUSR;
  if(str[3]=='x') mode |= S_IXUSR;
  if(str[4]=='r') mode |= S_IRGRP;
  if(str[5]=='w') mode |= S_IWGRP;
  if(str[6]=='x') mode |= S_IXGRP;
  if(str[7]=='r') mode |= S_IROTH;
  if(str[8]=='w') mode |= S_IWOTH;
  if(str[9]=='x') mode |= S_IXOTH;
}



// class Direntries implements.
bool Direntries::from_json(std::string& json, std::string& err)
{
  picojson::value root;
  picojson::parse<std::string::iterator>(root, json.begin(), json.end(), &err);
  if(!err.empty()) return false;

  clear();

  // try array: ["name", ... ]
  if(json[0]=='[') {
    picojson::array obj = root.get<picojson::array>();
    picojson::array::iterator it;
    for(it=obj.begin(); it!=obj.end(); it++) {
      Direntry item;
      item.name = (*it).get<std::string>();
      push_back(item);
    }
    if(size()>0) return true; 
  }

  // try hash: {"name":{"size":length,"mode":"modestr"}, ... }
  if(json[0]=='{') {
    picojson::object obj = root.get<picojson::object>();
    picojson::object::iterator it;
    for(it=obj.begin(); it!=obj.end(); it++) {
      std::pair<std::string, picojson::value> val = (*it);
      Direntry item;
      item.name = (*it).first;
      item.size = (uint64_t)((*it).second.get("size").get<double>());
      item.set_mode((*it).second.get("mode").get<std::string>().c_str());
      push_back(item);
    }
    if(size()>0) return true; 
  }

  return true;
}
