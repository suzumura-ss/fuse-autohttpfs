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

#ifndef __INCLUDE_CURLACCESSOR_H__
#define __INCLUDE_CURLACCESSOR_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <curl/curl.h>
#include "log.h"


class CurlSlist
{
public:
  inline CurlSlist() { m_head = NULL; m_size = 0; };
  virtual ~CurlSlist();
  void add(const char* string);
  inline curl_slist* slist() { return m_head; };
  inline size_t size() { return m_size; };

private:
  curl_slist* m_head;
  size_t  m_size;
};


class CurlAccessor
{
public:
  CurlAccessor(const char* url, bool dir_access = true);
  virtual ~CurlAccessor();
  void add_header(const char* key, const char* value);
  int head(Log& logger);
  int get(Log& logger, void* buf, uint64_t size, uint64_t offset);
  int get(Log& logger, std::string& body);
  inline const char* url() { return m_url.c_str(); };
  inline uint64_t content_length() { return m_content_length; };
  inline std::string content_type() { return m_content_type; };
  inline std::string x_filestat() { return m_x_filestat; };

private:
  CURL* curl;
  std::string m_user_agent;
  std::string m_url;
  CurlSlist m_headers;
  CURLcode  m_curl_code;
  int m_res_status;
  uint64_t m_content_length;
  std::string m_content_type;
  std::string m_x_filestat;
  void* m_buffer;
  uint64_t m_buffer_size;
  uint64_t m_read_size;
  std::string* m_body;
  size_t copy(const void* ptr, uint64_t size);
  void log_request_failed(Log& logger, const char* func);

private:
  static size_t header_callback(const void* ptr, size_t size, size_t nmemb, void* _context);
  static size_t write_callback(const void* ptr, size_t size, size_t nmemb, void* _context);
  static size_t write_callback_string(const void* ptr, size_t size, size_t nmemb, void* _context);
};
  

#endif // __INCLUDE_CURLACCESSOR_H__
// vim: sw=2 sts=2 ts=4 expandtab :
