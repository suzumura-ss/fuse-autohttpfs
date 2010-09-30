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


// class Direntries implements.
void Direntries::from_json(std::string& json)
{
  std::string err;
  picojson::value root;
  picojson::parse<std::string::iterator>(root, json.begin(), json.end(), &err);
  if(!err.empty()) throw err;

  clear();

  // try array: ["name", ... ]
  if(json[0]=='[') {
    picojson::array obj = root.get<picojson::array>();
    picojson::array::iterator it;
    for(it=obj.begin(); it!=obj.end(); it++) {
      FileStat stat(S_IFDIR);
      stat.name = (*it);
      push_back(stat);
    }
    if(size()==0) throw std::string("Empty array."); 
    return; 
  }

  // try hash: {"name":{"size":length,"mode":"modestr"}, ... }
  if(json[0]=='{') {
    picojson::object obj = root.get<picojson::object>();
    picojson::object::iterator it;
    for(it=obj.begin(); it!=obj.end(); it++) {
      std::pair<std::string, picojson::value> val = (*it);
      FileStat stat;
      stat.name = val.first;
      stat.from_json(val.second);
      push_back(stat);
    }
    if(size()==0) throw std::string("Empty hash.");
    return;
  }

  throw std::string("Unknown json type.");
}

// vim: sw=2 sts=2 ts=4 expandtab :
