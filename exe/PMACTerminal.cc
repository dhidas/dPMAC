////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int PMACTerminal ()
{
  std::string const IP = "192.168.1.103";

  PMAC2Turbo PMAC(IP);

  PMAC.Terminal();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  PMACTerminal();

  return 0;
}
