#include <stdlib.h>
#include <mcheck.h>

class MTrace
{
public:
  inline MTrace(const char* tag="malloc_trace.mlog") {
    setenv("MALLOC_TRACE", tag, true);
    mtrace();
  };
  inline ~MTrace() {
    muntrace();
    unsetenv("MALLOC_TRACE");
  };
};
