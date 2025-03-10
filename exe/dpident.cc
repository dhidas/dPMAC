////////////////////////////////////////////////////////////////////
//
// Stuart B. Wilkins <swilkins@bnl.gov>
//
// Created on : 10/03/2020 10:03:21 AM
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>

#include "PMAC2Turbo.h"


int dpident (std::string const& IP)
{

  PMAC2Turbo PMAC(IP, 1025);

  PMAC.GetResponse("IDNUMBER");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [IP]" << std::endl;
    return 1;
  }

  dpident(argv[1]);

  return 0;
}
