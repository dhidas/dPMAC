#ifndef GUARD_PMAC2Turbo_h
#define GUARD_PMAC2Turbo_h

#include <string>
#include <sys/socket.h>

class PMAC2Turbo
{
  public:
    PMAC2Turbo ();
    PMAC2Turbo (std::string const& IP, int const PORT = 1025);
    ~PMAC2Turbo ();
};






#endif
