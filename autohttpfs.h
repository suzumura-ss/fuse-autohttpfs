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

#ifndef __INCLUDE_AUTOHTTPFS_H__
#define __INCLUDE_AUTOHTTPFS_H__

#define FUSE_USE_VERSION 26
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <fuse.h>
#include <curl/curl.h>
#include <string>


class AutoHttpFs
{
public:
  inline AutoHttpFs() {};
  inline virtual ~AutoHttpFs() {};

  void setup();
  void parse_args(const char*& help, int& argc, char* argv[]);
  inline const struct stat* stat_d() { return &m_root_stat; };
  inline const struct stat* stat_r() { return &m_reguler_stat; };
  inline int errcode() { return m_errno; };

  static void init_fuse_operations(fuse_operations& oper); 

private:
  int m_errno;
  struct stat m_root_stat, m_reguler_stat;
  std::string m_root;
  bool      m_file_readonly;
  bool      m_file_noexec;
  uint64_t  m_max_readahead;
  static void parsearg_helper(std::string& opt, const char* key, int& argc, char** argv, int& it);
  static void parsearg_helper(int& opt, const char* key, int& argc, char** argv, int& it);
  static void parsearg_helper(bool& opt, const char* key, int& argc, char** argv, int& it);
  static void parsearg_shift(int& argc, char** argv, int& it);

private:
  static int getattr(const char* path, struct stat *stbuf);
  static int opendir(const char* path, struct fuse_file_info *ffi);
  static int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *ffi);
  static int releasedir(const char* path, struct fuse_file_info *ffi);
  static int truncate(const char* path, off_t size);
  static int open(const char* path, struct fuse_file_info* ffi);
  static int read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* ffi);
  static int write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* ffi);
  static int flush(const char* path, struct fuse_file_info* ffi);
  static int release(const char* path, struct fuse_file_info* ffi);
  static void* init(struct fuse_conn_info* fci);
  static void destroy(void* user_data);
};


#endif // __INCLUDE_AUTOHTTPFS_H__
// vim: sw=2 sts=2 ts=4 expandtab :
