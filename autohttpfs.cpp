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
#include "dirent.h"
#include "log.h"
#include "int64format.h"


// Globals.
static Log glog("autohttpfs", LOG_LOCAL7, Log::NOTE);


// AutoHttpFs class implements.
void AutoHttpFs::parse_args(const char*& help, int& argc, char* argv[])
{
  m_root = "/";
  m_max_readahead = 0x20000;
  int ll = Log::NOTE;
  int mr = 0x20000;
  bool ro = true, ne = true;

  for(int it=1; it<argc; it++) {
    parsearg_helper(ro, "--readonly=", argc, argv+it, it);
    parsearg_helper(ne, "--noexec=", argc, argv+it, it);
    parsearg_helper(ll, "--loglevel=", argc, argv+it, it);
    parsearg_helper(m_root, "--root=", argc, argv+it, it);
    parsearg_helper(mr, "--max_readahead=", argc, argv+it, it);
    if(strcmp("--help", argv[it])==0) {
      help = "autohttpfs options:\n" \
             "    --readonly=SW       modify file permission.\n" \
             "                          'yes':readonly,       'no':writable   (default:yes)\n" \
             "    --noexec=SW         modify file permission.\n" \
             "                          'yes':non executable, 'no':executable (default:yes)\n" \
             "    --root=DIR          (default: / (root))\n" \
             "    --loglevel=N        syslog level (default: 5 (NOTE))\n" \
             "    --max_readahead     fuse_conn.info.max_readahead (default: 131072)\n";
    }
  }
  glog.loglevel((Log::LOGLEVEL)ll);
  m_file_readonly = ro;
  m_file_noexec = ne;
  m_max_readahead = mr;
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


void AutoHttpFs::parsearg_helper(bool& opt, const char* key, int& argc, char** argv, int& it)
{
  size_t kl = strlen(key);
  if(strncmp(*argv, key, kl)==0) {
    char* v = argv[0] + kl;
    if(strcasecmp(v, "YES")==0) {
      opt = true;
    } else if(strcasecmp(v, "NO")==0) {
      opt = false;
    } else {
      glog(Log::ERR, "Invalid options: '%s' for '%s'.\n", key, v);
    }
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
  m_root_stat.st_dev = 0;
  m_root_stat.st_ino = 0;
  m_root_stat.st_nlink = 1;
  m_root_stat.st_uid = geteuid();
  m_root_stat.st_gid = getegid();
  m_root_stat.st_mode &= (~(S_IWUSR|S_IWGRP|S_IWOTH));
  m_root_stat.st_mode &= (~S_IFMT);
  m_reguler_stat = m_root_stat;
  m_root_stat.st_mode |= S_IFDIR|S_IXUSR;
  m_reguler_stat.st_mode &= (~(S_IXUSR|S_IXGRP|S_IXOTH));
  m_reguler_stat.st_mode |= S_IFREG;
  if(!m_file_readonly) m_reguler_stat.st_mode |= S_IWUSR;
}


// fuse::getattr
int AutoHttpFs::getattr(const char* path, struct stat *stbuf)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  AutoHttpFs* self = ctxs->fs();
  glog(Log::DEBUG, ">> %s(%s) ctxs=%p, this=%p\n", __FUNCTION__, path, ctxs, self);

  memset(stbuf, 0, sizeof(struct stat));
  if(self->errcode()!=0) return self->errcode();

  // for proc/.
  if(ctxs->proc_getattr(glog, path, *stbuf)==0) return 0;

  // for normal files.
  UrlStat us;
  int r = ctxs->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_dir()) {
    memcpy(stbuf, self->stat_d(), sizeof(*stbuf));
  } else {
    memcpy(stbuf, self->stat_r(), sizeof(*stbuf));
  }
  if(us.mode & (~S_IFMT)) {
    stbuf->st_mode = us.mode & (~(S_IWUSR|S_IWGRP|S_IWOTH));
    if(!self->m_file_readonly) stbuf->st_mode |= S_IWUSR;
    if(self->m_file_noexec && us.is_reg()) stbuf->st_mode &= ~(S_IXUSR|S_IXGRP|S_IXOTH);
  }
  if(us.length!=0) {
    stbuf->st_size = us.length;
  }
  if(us.mtime!=0) {
    stbuf->st_atime = stbuf->st_ctime = stbuf->st_mtime = us.mtime;
  }
  return 0;
}


// fuse::opendir
int AutoHttpFs::opendir(const char* path, struct fuse_file_info *ffi)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  glog(Log::DEBUG, ">> %s(%s) ctxs=%p\n", __FUNCTION__, path, ctxs);

  // for proc/.
  if(ctxs->proc_opendir(glog, path, *ffi)==0) return 0;

  // for normal files.
  UrlStat us;
  int r = ctxs->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_dir()) {
    AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
    ffi->fh = ctx->seq();
    glog(Log::DEBUG, "   => fh=%"FINT64"d, ctx=%p\n", ffi->fh, ctx);
    return 0;
  }
  if(us.is_reg()) return -ENOTDIR;
  return -ENOENT;
}


// fuse::readdir
int AutoHttpFs::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *ffi)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  AutoHttpFs* self = ctxs->fs();
  glog(Log::DEBUG, ">> %s(%s) fh=%"FINT64"d, ctx=%p, this=%p\n", __FUNCTION__, path, ffi->fh, ctx, self);

  if(ffi->fh==0) return -EINVAL;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  if(strcmp(path, "/")==0) return 0;

  // for proc/.
  if(ctx->proc!=NULL) return ctx->proc->readdir(glog, buf, filler, offset);

  // for normal files.
  CurlAccessor ca(path, true);
  ca.add_header("Accept", "text/json;hash");
  std::string json;
  int r = ca.get(glog, json);
  if(r!=200) return 0;
  if(ca.content_type().compare("text/json")!=0) return 0;

  Direntries de;
  try { de.from_json(json); }
  catch(std::string e) {
    glog(Log::INFO, "JSON parse failed in %s - %s\n", __FUNCTION__, e.c_str());
    std::string e;
    return 0;
  }
  for(Direntries::iterator it = de.begin(); it!=de.end(); it++) {
    struct stat st;
    if((*it).mode &  S_IFDIR) {
      memcpy(&st, self->stat_d(), sizeof(st));
    } else {
      memcpy(&st, self->stat_r(), sizeof(st));
      if((*it).mode & S_IFLNK) st.st_mode |= S_IFLNK;
      st.st_size = (*it).size;
    }
    filler(buf, (*it).name.c_str(), &st, 0);
  }

  return 0;
}


// fuse::releasedir
int AutoHttpFs::releasedir(const char* path, struct fuse_file_info *ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  // for proc/.
  if(ctx->proc) ctx->proc->releasedir(glog);

  // for normal files.
  AUTOHTTPFSCONTEXTS.release_context(ctx);
  return 0;
}


// fuse::truncate
int AutoHttpFs::truncate(const char* path, off_t size)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  glog(Log::DEBUG, ">> %s(%s) ctxs=%p\n", __FUNCTION__, path, ctxs);

  // for proc/.
  if(ctxs->proc_truncate(glog, path, size)==0) return 0;

  // for normal files.
  return -ENOSYS;
}


// fuse::open
int AutoHttpFs::open(const char* path, struct fuse_file_info* ffi)
{
  AutoHttpFsContexts* ctxs = &AUTOHTTPFSCONTEXTS;
  glog(Log::DEBUG, ">> %s(%s) ctxs=%p\n", __FUNCTION__, path, ctxs);

  // for proc/.
  if(ctxs->proc_open(glog, path, *ffi)==0) return 0;

  // for normal files.
  UrlStat us;
  int r = ctxs->get_attr(glog, path, us);
  if(r!=0) return r;

  if(us.is_reg()) {
    AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.alloc_context();
    ffi->fh = ctx->seq();
    glog(Log::DEBUG, "   => fh=%"FINT64"d, ctx=%p\n", ffi->fh, ctx);
    return 0;
  }
  if(us.is_dir()) return -EINVAL;
  return -ENOENT;
}


// fuse::read
int AutoHttpFs::read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p, size=%"FINT64"d, offset=%"FINT64"d\n", \
                        __FUNCTION__, path, ffi, ffi->fh, ctx, (off_t)size, offset);

  if(ffi->fh==0) return -EINVAL;

  // for proc/.
  if(ctx->proc) return ctx->proc->read(glog, buf, size, offset);

  // for normal files.
  UrlStat us;
  int r = ctx->get_attr(glog, path, us);
  if(r!=0) return r;
  if(!us.is_reg()) return -EINVAL;
 
  if((uint64_t)offset>=us.length) return 0;
  if((uint64_t)offset+(uint64_t)size>=us.length) {
    size = us.length - offset;
  }
  if(size<0) return 0;

  CurlAccessor ca(path);
  r = ca.get(glog, buf, offset, size);
  if((r==200)||(r==206)) return size;

  return -ENOENT;
}


// fuse::write
int AutoHttpFs::write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::DEBUG, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p, size=%"FINT64"d, offset=%"FINT64"d\n", \
                        __FUNCTION__, path, ffi, ffi->fh, ctx, (off_t)size, offset);

  if(ffi->fh==0) return -EINVAL;

  // for proc/.
  if(ctx->proc) return ctx->proc->write(glog, buf, size, offset);

  // for normal files.
  return -ENOSYS;
}


// fuse::flush
int AutoHttpFs::flush(const char* path, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::INFO, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  // for proc/.
  if(ctx->proc) ctx->proc->flush(glog, ffi);

  // for normal files.
  return -ENOSYS;
}


// fuse::release
int AutoHttpFs::release(const char* path, struct fuse_file_info* ffi)
{
  AutoHttpFsContext* ctx = AUTOHTTPFSCONTEXTS.find(ffi->fh);
  glog(Log::INFO, ">> %s(%s) ffi=%p, fh=%"FINT64"d, ctx=%p\n", __FUNCTION__, path, ffi, ffi->fh, ctx);

  // for proc/.
  if(ctx->proc) ctx->proc->release(glog);

  // for normal files.
  AUTOHTTPFSCONTEXTS.release_context(ctx);
  return 0;
}


// fuse::init
void* AutoHttpFs::init(struct fuse_conn_info* fci)
{
  fuse_context* fc = fuse_get_context();
  AutoHttpFs* self = (AutoHttpFs*)(fc->private_data);

  fci->async_read   = 1;
  fci->max_write    = 16;
  fci->max_readahead = self->m_max_readahead;

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
  oper.truncate   = truncate;
  oper.open       = open;
  oper.read       = read;
  oper.write      = write;
  oper.flush      = flush;
  oper.release    = release;
  oper.init       = init;
  oper.destroy    = destroy;
}

// vim: sw=2 sts=2 ts=4 expandtab :
