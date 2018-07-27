////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int dpip (std::string const& IP, std::string const& NEWIP)
{

  PMAC2Turbo PMAC(IP, 1025);
  PMAC.IPAddress(NEWIP);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [IP] [NEW IP]" << std::endl;
    return 1;
  }

  dpip(argv[1], argv[2]);

  return 0;
}
