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
#include "cache.h"
#include "context.h"
#include "remoteattr.h"
#include "curlaccessor.h"
#include "log.h"
#include "int64format.h"


// Globals.
static Log glog("autohttpfs", LOG_LOCAL7, Log::NOTE);


// AutoHttpFs class implements.
void AutoHttpFs::parse_args(const char*& help, int& argc, char* argv[])
{
  m_root = "/";
  int ll = Log::NOTE;

  for(int it=1; it<argc; it++) {
    parsearg_helper(m_root, "--root=", argc, argv+it, it);
    parsearg_helper(ll, "--loglevel=", argc, argv+it, it);
    if(strcmp("--help", argv[it])==0) {
      help = "autohttpfs options:\n" \
             "    --root=DIR      (default: / (root))\n" \
             "    --loglevel=N    syslog level (default: 5 (NOTE))\n";
    }
  }
  glog.set_level((Log::LOGLEVEL)ll);
}


void AutoHttpFs::parsearg_helper(std::string& opt, const char* key, int& argc, char** argv, int& it)
{
  size_t kl = strlen(key);
  if(strncmp(*argv, key, kl)==0) {
    opt = argv[0] + kl;
    parsearg_shift(argc, argv, it);
    glog(Log::DEBUG, "parse_args: '%s' => '%s'\n", key, opt.c_str());
  }
}


void AutoHttpFs::parsearg_helper(int& opt, const char* key, int& argc, char** argv, int& it)
{
  size_t kl = strlen(key);
  if(strncmp(*argv, key, kl)==0) {
    opt = atoi(argv[0] + kl);
    parsearg_shift(argc, argv, it);
    glog(Log::DEBUG, "parse_args: '%s' => %d\n", key, opt);
  }
}


void AutoHttpFs::parsearg_shift(int& argc, char** argv, int& it)
{
  memmove(argv, argv+1, sizeof(argv)*(argc-it));
  argc--;
  it--;
}


void AutoHttpFs::setup()
{
  m_errno = 0;
  memset(&m_root_stat, 0, sizeof(struct stat));
  if(::stat(m_root.c_str(), &m_root_stat)) m_errno = errno;
  m_root_stat.st_mode &= (~(S_IWUSR|S_IWGRP|S_IWOTH));
  m_root_stat.st_mode &= (~S_IFMT);
  m_reguler_stat = m_root_stat;
  m_root_stat.st_mode |= S_IFDIR;
  m_reguler_stat.st_mode &= (~(S_IXUSR|S_IXGRP|S_IXOTH));
  m_reguler_stat.st_mode |= S_IFREG|S_IWUSR;
}


// fuse::getattr
int AutoHttpFs::getattr(const char* path, struct stat *stbuf)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  AutoHttpFs* self = ctxs->fs();
  glog(Log::DEBUG, ">> %s(%s) ctxs=%p, this=%p\n", __FUNCTION__, path, ctxs, self);

  memset(stbuf, 0, sizeof(struct stat));
  if(self->errcode()!=0) return self->errcode();

  UrlStat us;
  int r = ctxs->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_dir()) {
    memcpy(stbuf, self->stat_d(), sizeof(*stbuf));
  } else {
    memcpy(stbuf, self->stat_r(), sizeof(*stbuf));
    stbuf->st_size = us.length;
  }
  return 0;
}


// fuse::opendir
int AutoHttpFs::opendir(const char* path, struct fuse_file_info *ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
  ffi->fh = ctx->seq();
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  UrlStat us;
  int r = ctx->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_dir()) return 0;
  if(us.is_reg()) return -ENOTDIR;
  return -ENOENT;
}


// fuse::readdir
int AutoHttpFs::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  if(ffi->fh==0) return -EINVAL;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  return 0;
}


// fuse::releasedir
int AutoHttpFs::releasedir(const char* path, struct fuse_file_info *ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  AUTOHTTPFSCONTEXTS.release_context(ctx);
  return 0;
}


// fuse::open
int AutoHttpFs::open(const char* path, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
  ffi->fh = ctx->seq();
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  UrlStat us;
  int r = ctx->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_reg()) return 0;
  if(us.is_dir()) return -EACCES;
  return -ENOENT;
}


// fuse::read
int AutoHttpFs::read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p, size=%"FINT64"d, offset=%"FINT64"d\n", \
                        __FUNCTION__, path, ffi, ffi->fh, ctx, (off_t)size, offset);

  if(ffi->fh==0) return -EINVAL;

  UrlStat us;
  int r = ctx->get_attr(glog, path, us);
  if(r!=0) return r;
  if(!us.is_reg()) return -EINVAL;
 
  if((uint64_t)offset>=us.length) return 0;
  if((uint64_t)offset+(uint64_t)size>=us.length) {
    size = us.length - offset;
  }
  if(size<0) return 0;

  CurlAccessor ca(path, false);
  r = ca.get(glog, buf, offset, size);
  if((r==200)||(r==206)) return size;
  return -ENOENT;
}


// fuse::release
int AutoHttpFs::release(const char* path, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::INFO, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  AUTOHTTPFSCONTEXTS.release_context(ctx);
  return 0;
}


// fuse::init
void* AutoHttpFs::init(struct fuse_conn_info* fci)
{
  fuse_context* fc = fuse_get_context();
  AutoHttpFs* self = (AutoHttpFs*)(fc->private_data);

  fci->async_read   = 1;
  fci->max_write    = 0;
  fci->max_readahead = 0x20000;

  AutoHttpFsContexts* ctxs = new AutoHttpFsContexts(self);
  glog(Log::NOTE, "Starting autohttpfs.\n");
  return (void*)ctxs;
}


// fuse::destroy
void AutoHttpFs::destroy(void* user_data)
{
  AutoHttpFsContexts* ctxs = (AutoHttpFsContexts*)user_data;
  glog(Log::NOTE, "Stopping autohttpfs.\n");
  delete ctxs;
}


// init fuse::fuse_operations
void AutoHttpFs::init_fuse_operations(fuse_operations& oper)
{
  memset(&oper, 0, sizeof(oper));
  oper.getattr    = getattr;
  oper.opendir    = opendir;
  oper.readdir    = readdir;
  oper.releasedir = releasedir;
  oper.open       = open;
  oper.read       = read;
  oper.release    = release;
  oper.init       = init;
  oper.destroy    = destroy;
}

// vim: sw=2 sts=2 ts=4 expandtab :
