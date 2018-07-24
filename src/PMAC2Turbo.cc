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
#include <fstream>
#include <exception>
#include <string.h>
//#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <stdlib.h>
#include <unistd.h>
//#include <netdb.h>
//#include <cstdlib>
#include <bitset>

#include <readline/readline.h>
#include <readline/history.h>

void PrintBits (char c)
{
  std::cout << std::bitset<8>(c).to_ulong() << std::endl;
  return;
}

PMAC2Turbo::PMAC2Turbo ()
{
  // Default constructor
  fSocket = -1;
}



PMAC2Turbo::PMAC2Turbo (std::string const& IP, int const PORT)
{
  // Constructor
  // Arguments:
  //  IP - IP address as a string, e.g. "192.168.1.103"
  //  PORT - The port number to connect to as an int.  Typically 1025

  fSocket = -1;

  this->Connect(IP, PORT);
}



PMAC2Turbo::~PMAC2Turbo ()
{
  // Destruction
  this->Disconnect();
}



void PMAC2Turbo::Connect (std::string const& IP, int const PORT)
{
  // Create and connect the socket
  fSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (fSocket < 0) {
    std::cerr << "ERROR: Cannot create socket" << std::endl;
  }

  // Server socket address struct
  struct sockaddr_in server_addr;

  // Set server IP and port information
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP.c_str());
  server_addr.sin_port = htons(PORT);

  // Connect socket or return an error
  if (connect(fSocket,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
  } else {
    std::cerr << "Error when connecting to server " << IP << " on port " << PORT << std::endl;
    return;
  }

  return;
}



void PMAC2Turbo::Disconnect ()
{
  // Disconnect the socket if it looks like it exists
  if (fSocket >= 0) {
    close(fSocket);
  }

  return;
}




void PMAC2Turbo::Terminal ()
{
  // Simple terminal for PMAC2 using the readline library

  this->Flush();

  rl_bind_key (CTRLK, rl_insert);
  //rl_bind_key (CTRLK, static_cast<PMAC2Turbo*>(this)->SendCTRLK);
  rl_bind_key (CTRLD, rl_insert);
  char* buf;
  while ((buf = readline(">> ")) != nullptr) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }

    this->SendLine(buf);
    this->GetBuffer();

    // readline malloc's a new buffer every time.
    free(buf);
  }


  return;
}





void PMAC2Turbo::Flush ()
{
  // Flush the data buffer in PMAC

  // First flush any data in the buffer
  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request     = VR_PMAC_FLUSH;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = 0;
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  recv(fSocket, (char*) &fData, 1, 0);
  if (fData[0] != VR_DOWNLOAD) {
    std::cerr << "ERROR: flush failed" << std::endl;
  }

  return;
}





void PMAC2Turbo::IPAddress (std::string const& IP)
{
  // Get or set the IP address

  if (IP == "") {
    fEthCmd.RequestType = VR_UPLOAD;
  } else {
    fEthCmd.RequestType = VR_DOWNLOAD;
  }
  fEthCmd.Request     = VR_IPADDRESS;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(4);
  //strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
  //send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  recv(fSocket, (char*) &fData, 4, 0);

  if (IP == "") {
    std::cout << std::bitset<8>(fData[0]).to_ulong() << "."
              << std::bitset<8>(fData[1]).to_ulong() << "."
              << std::bitset<8>(fData[2]).to_ulong() << "."
              << std::bitset<8>(fData[3]).to_ulong() << std::endl;
  }

  return;
}




void PMAC2Turbo::SendCTRLK ()
{

  //fEthCmd.RequestType = VR_DOWNLOAD;
  //fEthCmd.Request     = VR_PMAC_SENDCTRLCHAR;
  //fEthCmd.wValue      = htons(CTRLK);
  //fEthCmd.wIndex      = 0;
  //fEthCmd.wLength     = 0;
  //send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);

  fEthCmd.RequestType = VR_UPLOAD;
  fEthCmd.Request     = VR_CTRL_RESPONSE;
  fEthCmd.wValue      = htons(CTRLK);
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(1);
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  recv(fSocket, fData, 1, 0);

  std::cout << "data back: " << fData << std::endl;
  return;
}





void PMAC2Turbo::SendLine (std::string const& Line)
{
  // Send a command line to PMAC
  
  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request     = VR_PMAC_SENDLINE;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(Line.size());
  strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
  recv(fSocket, (char*) &fData, 1, 0);

  return;
}




int PMAC2Turbo::DownloadFile (std::string const& InFileName)
{
  // Download a file to PMAC.
  // Returns:
  //  -1    : Cannot open file
  //   1    : Success
  //  other : Write error
 
  // Open file for reading
  std::ifstream fi(InFileName);
  if (!fi.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    return -1;
  }

  // Loop over each line in file
  for (std::string Line; std::getline(fi, Line); ) {

    std::cout << Line << std::endl;

    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);

    PrintBits(fData[0]);
    PrintBits(fData[1]);
    PrintBits(fData[2]);
    PrintBits(fData[3]);
  }

  return 1;
}




void PMAC2Turbo::WriteBuffer (std::string const& Buffer)
{
  // Send a command line to PMAC
 
  std::ifstream fi(Buffer);
  if (!fi.is_open()) {
    std::cerr << "ERROR: cannot open file: " << Buffer << std::endl;
    return;
  }

  for (std::string Line; std::getline(fi, Line); ) {

    std::cout << Line << std::endl;

    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);
    PrintBits(fData[0]);
    PrintBits(fData[1]);
    PrintBits(fData[2]);
    PrintBits(fData[3]);
  }

  return;
}




void PMAC2Turbo::GetBuffer (std::string const& OutFileName)
{
  // File for writing if name given
  std::ofstream of;
  if (OutFileName != "") {
    of.open(OutFileName.c_str());
    if (!of.is_open()) {
      std::cerr << "ERROR: cannot open OutFileName: " << OutFileName << std::endl;
    }
  }

  // Write to file or std::cout
  std::ostream* out = of.is_open() ? &of : &std::cout ;

  // For the output commands
  fEthCmd.RequestType = VR_UPLOAD;
  fEthCmd.Request     = VR_PMAC_READREADY;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(2);

  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  recv(fSocket, fData, 2, 0);

  //int call = 0;
  while (fData[0] == 1) {

    fEthCmd.RequestType = VR_UPLOAD;
    fEthCmd.Request     = VR_PMAC_GETBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = 0;
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
    recv(fSocket, (char*) &fData, 1400, 0);

    int cr_at = -1;
    bool ack_found = false;
    for (int i = 0; i != 1400; ++i) {
      if (fData[i] == ACK) {
        ack_found = true;
        cr_at = i;
        break;
      }
      if (fData[i] == 0xD) {
        cr_at = i;
        if (i < 1400-1 && fData[i+1] == ACK) {
        }
        break;
      }
    }

    if (cr_at < 0) {
    } else {
      for (int j = 0; j < cr_at; ++j) {
        *out << fData[j];
      }
      if (!ack_found) {
        *out << std::endl;
      }
    }
    if (ack_found) {
      break;
    }

    fEthCmd.RequestType = VR_UPLOAD;
    fEthCmd.Request     = VR_PMAC_READREADY;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(2);
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
    recv(fSocket, fData, 2, 0);
  }

  return;
}





void PMAC2Turbo::ListGather (std::string const& OutFileName)
{
  // Check if socket at least defined
  if (fSocket < 0) {
    std::cerr << "ERROR: Trying to ListGather but socket not created" << std::endl;
    return;
  }

  // First flush any data in the buffer, send the list gather command,
  // Read the buffer, and flush after
  this->Flush();
  this->SendLine("LIST GATHER");
  this->GetBuffer(OutFileName);
  this->Flush();

  return;
}
