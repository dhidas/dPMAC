////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int dpreboot (std::string const& IP)
{
  PMAC2Turbo PMAC(IP, 1025);
  PMAC.Reset();

  return 0;
}


int main (int argc, char* argv[])
{
  std::cout << "Will: $$$***, SAVE, $$$, download file, SAVE, and $$$" << std::endl;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [IP]" << std::endl;
    return 1;
  }

  dpreboot(argv[1]);

  return 0;
}
