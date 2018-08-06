////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int dpterm (std::string const& IP = "", int const PORT = 1025)
{

  PMAC2Turbo PMAC(IP, PORT);
  PMAC.Terminal();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1 && argc != 2 && argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [IP] [PORT]" << std::endl;
    return 1;
  }

  if (argc == 1) {
    dpterm();
  } else if (argc == 2) {
    dpterm(argv[1]);
  } else {
    dpterm(argv[1], atoi(argv[2]));
  }

  return 0;
}
