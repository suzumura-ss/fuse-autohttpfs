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

#include "curlaccessor.h"
#include "log.h"
#include "int64format.h"


// CurlSlist class implements.
CurlSlist::~CurlSlist()
{
  while(curl_slist* p=m_head) {
    m_head = p->next;
    delete[] p->data;
    delete p;
  }
}


void CurlSlist::add(const char* string)
{
  curl_slist* p = new curl_slist;
  p->next = m_head;
  p->data = new char[strlen(string)+1];
  strcpy(p->data, string);
  m_head = p;
  m_size++;
}



// CurlAccessor class implements.
CurlAccessor::CurlAccessor(const char* path, bool dir_access)
{
  m_url = (path[0]!='/')? path: path+1;
  if(dir_access) m_url.append("/");  

  m_user_agent = "autohttpfs/0.0.1 ";
  m_user_agent.append(curl_version());
  m_res_status = 0;
  m_buffer = NULL;
  m_buffer_size = 0;
  m_read_size = 0;
  m_content_length = (uint64_t)-1;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_URL, url());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, m_user_agent.c_str());
}


CurlAccessor::~CurlAccessor()
{
  curl_easy_cleanup(curl);
  curl = NULL;
}


void CurlAccessor::add_header(const char*key, const char* value)
{
  std::string h = key;
  h.append(": ");
  h.append(value);
  m_headers.add(h.c_str());
}


int CurlAccessor::head(Log& logger)
{
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headers.slist());
  m_curl_code = curl_easy_perform(curl);
  if(m_curl_code!=CURLE_OK) log_request_failed(logger, __FUNCTION__);
  return m_res_status;
}


int CurlAccessor::get(Log& logger, void* buf, uint64_t offset, uint64_t size)
{
  char range[256];
  snprintf(range, sizeof(range), "%"FINT64"u-%"FINT64"u", offset, offset+size-1);

  logger(Log::VERBOSE, "   [CurlAccessor::get(%s)] offset=%"FINT64"u, size=%"FINT64"u, Range: %s\n", m_url.c_str(), offset, size, range);

  m_buffer = buf;
  m_buffer_size = size;
  m_read_size = 0;
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headers.slist());
  curl_easy_setopt(curl, CURLOPT_RANGE, range);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  m_curl_code = curl_easy_perform(curl);
  if(m_curl_code==CURLE_PARTIAL_FILE) {
    logger(Log::WARN, "    [CurlAccessor::get(%s)] failed / size=%"FINT64"u, offset=%"FINT64"u, Range: %s\n", m_url.c_str(), size, offset, range);
  }
  if(m_curl_code!=CURLE_OK) log_request_failed(logger, __FUNCTION__);
  return m_res_status;
}


size_t CurlAccessor::copy(const void* ptr, uint64_t size)
{
  if(m_buffer==NULL) return 0;

  if(m_buffer_size<m_read_size) return 0;

  uint64_t remain = m_buffer_size - m_read_size;
  if(remain<size) size = remain;
  memcpy(((uint8_t*)m_buffer)+m_read_size, ptr, size);
  m_read_size += size;

  return size;
}


void CurlAccessor::log_request_failed(Log& logger, const char* func)
{
  std::string msg = m_user_agent;
  msg.append(" - rei"FINT64"uest failed / ");
  msg.append("URL: ");
  msg.append(m_url);
  msg.append(" code: ");
  msg.append(curl_easy_strerror(m_curl_code));
  logger(Log::WARN, "[%s] %s(%d)\n", func, msg.c_str(), m_curl_code);
  m_res_status = -m_curl_code;
}


size_t CurlAccessor::header_callback(const void* ptr, size_t size, size_t nmemb, void* _context)
{
  CurlAccessor* self = (CurlAccessor*)_context;
  const char* hdr = (const char*)ptr;

  std::string h = hdr;
  h.resize(size*nmemb-1);
  if(strncasecmp(hdr, "HTTP/1.", sizeof("HTTP/1.")-1)==0) {
    int mv, status;
    if(sscanf(hdr, "HTTP/1.%d %d ", &mv, &status)==2) {
      self->m_res_status = status;
    }
  } else
  if(strncasecmp(hdr, "Content-Length:", sizeof("Content-Length:")-1)==0) {
    self->m_content_length = strtoull(hdr+sizeof("Content-Length:")-1, NULL, 10);
  }
  return nmemb;
}


size_t CurlAccessor::write_callback(const void* ptr, size_t size, size_t nmemb, void* _context)
{ 
  CurlAccessor* self = (CurlAccessor*)_context;
  return self->copy(ptr, size*nmemb);
}

// vim: sw=2 sts=2 ts=4 expandtab :
