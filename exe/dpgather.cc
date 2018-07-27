////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int dpbackup (std::string const& IP, std::string const& OutFileName)
{

  PMAC2Turbo PMAC(IP, 1025);
  PMAC.ListGather(OutFileName);

  return 0;
}


int main (int argc, char* argv[])
{
  std::cout << "Will: download the gather buffer to specified file" << std::endl;
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [IP] [OutFile]" << std::endl;
    return 1;
  }

  dpbackup(argv[1], argv[2]);

  return 0;
}
