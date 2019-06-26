////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Mon Jul 23 09:53:54 EDT 2018
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PMAC2Turbo.h"


int dprestore (std::string const& IP, std::string const& InFileName)
{
  PMAC2Turbo PMAC(IP, 1025);
  PMAC.FactoryReset();
  if (PMAC.DownloadFile(InFileName) == 0) {
    PMAC.SendLine("save");
    PMAC.Reset();
  } else {
    std::cerr << "ERRORs detected dowing download.  pmac SAVE not issued" << std::endl;
    return 1;
  }

  //PMAC.PrintDefinePairs();
  return 0;
}


int main (int argc, char* argv[])
{
  std::cout << "Will: $$$***, SAVE, $$$, download file, SAVE, and $$$" << std::endl;
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [IP] [InFile]" << std::endl;
    return 1;
  }

  dprestore(argv[1], argv[2]);

  return 0;
}
