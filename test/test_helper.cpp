#include "../log.h"

Log::Log(const char* ident, int facility, LOGLEVEL level) {}
Log::~Log() {}
void Log::vlog(int level, const char* fmt, va_list va) const {}
void Log::operator() (int level, const char* fmt, ...) const {}

Log glog("");
