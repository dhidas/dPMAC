////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Fri Jul 20 15:35:30 EDT 2018
//
// This is intended to be a PMAC interface for the PMAC2 TURBO type
// controllers from DeltaTau
//
////////////////////////////////////////////////////////////////////

#include "PMAC2Turbo.h"

#include <iostream>

PMAC2Turbo::PMAC2Turbo ()
{
  // Default constructor
}



PMAC2Turbo::PMAC2Turbo (std::string const& IP, int const PORT)
{
  // Constructor
  // Arguments:
  //  IP - IP address as a string, e.g. "192.168.1.103"
  //  PORT - The port number to connect to as an int.  Typically 1025

  //this->SetIPPort(IP, PORT)

  int sock = socket(PF_INET, SOCK_STREAM, 0);
}



PMAC2Turbo::~PMAC2Turbo ()
{
  // Destruction
}


