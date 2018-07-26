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
#include <sstream>
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



void PMAC2Turbo::ReConnect ()
{
  // Reconnect the socket

  // Hang up first if connected, then connect
  this->Disconnect();
  this->Connect(fIP, fPORT);

  return;
}



void PMAC2Turbo::Connect (std::string const& IP, int const PORT)
{
  // Store IP and PORT
  fIP = IP;
  fPORT = PORT;

  // Create and connect the socket
  fSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (fSocket < 0) {
    std::cerr << "ERROR: Cannot create socket" << std::endl;
    l() && fL << "ERROR: Cannot create socket" << std::endl;
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
    l() && fL << "Error when connecting to server " << IP << " on port " << PORT << std::endl;
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




void PMAC2Turbo::StartLog (std::string const& OutFileName)
{

  std::cout << "Start logging to: " << OutFileName << std::endl;
  // Open log file for writing
  fL.open(OutFileName);
  if (!fL.is_open()) {
    std::cerr << "ERROR: cannot open logfile for writing: " << OutFileName << std::endl;
    return;
  }
  return;
}




void PMAC2Turbo::StopLog ()
{
  if (fL.is_open()) {
    fL.close();
  } else {
    std::cerr << "ERROR: logfile was already closed.  nothing done." << std::endl;
  }
  return;
}







void PMAC2Turbo::Reset()
{
  // Send $$$ to the controller, pause, and flush
  this->SendLine("$$$");
  sleep(2);
  this->Flush();

  return;
}




void PMAC2Turbo::FactoryReset()
{
  // Send $$$ to the controller, pause, and flush
  this->SendLine("$$$***");
  sleep(2);
  this->Flush();
  this->SendLine("I3=2");
  this->Flush();

  return;
}








void PMAC2Turbo::Save ()
{
  // Send $$$ to the controller, pause, and flush
  std::cout << "saving" << std::endl;
  l() && fL << "saving" << std::endl;
  this->SendLine("save");
  this->GetBuffer();
  sleep(4);
  this->Flush();

  return;
}








void PMAC2Turbo::Terminal ()
{
  // Simple terminal for PMAC2 using the readline library

  // Start with a flushed buffer
  this->Flush();

  rl_bind_key (CTRLK, rl_insert);
  //rl_bind_key (CTRLK, static_cast<PMAC2Turbo*>(this)->SendCTRLK);
  rl_bind_key (CTRLD, rl_insert);


  bool Logging = false;
  std::string LogFileName = "";
  std::ofstream LogFile;


  char* buf;
  while ((buf = readline(">> ")) != nullptr) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }

    l() && fL << ">> " << buf << std::endl;

    std::string bs = std::string(buf);
    size_t first_nws = bs.find_first_not_of(" \t\f\v\n\r");
    if (first_nws != 0 && first_nws != std::string::npos) {
      bs = std::string(bs.begin() + first_nws, bs.end());
    }

    if (bs == ".h") {
      std::cout << "Commands:" << std::endl;
      std::cout << "  .h                              - print help" << std::endl;
      std::cout << "  .q                              - quit" << std::endl;
      std::cout << "  .d     [file]                   - Download file to pmac" << std::endl;
      std::cout << "  .l     [file]                   - logging (without [file] is to stop, with will log to file" << std::endl;
      std::cout << "  .g     [file]                   - Upload gather buffer from pmac to file" << std::endl;
      std::cout << "  .b     [file]                   - Upload backup CFG from pmac to file" << std::endl;
      std::cout << "  .iv    [file] [start] [stop]    - dump I variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .pv    [file] [start] [stop]    - dump P variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .qv    [file] [start] [stop]    - dump Q variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .mv    [file] [start] [stop]    - dump M variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .mvdef [file] [start] [stop]    - dump M variable definitions to file (start stop optional integers)" << std::endl;
    } else if (bs == "$$$") {
      this->Reset();
    } else if (bs == "$$$***") {
      this->FactoryReset();
    } else if (bs == "save") {
      this->Save();
    } else if (bs.find(".iv") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      int first = 0;
      int last = 8191;
      ss >> fn;
      fn = "";
      ss >> fn;
      ss >> first;
      ss >> last;
      if (fn.size() > 0) {
        std::cout << "first: " << first << "  last: " << last << std::endl;
        l() && fL << "first: " << first << "  last: " << last << std::endl;
        this->VariableDump("I", fn, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".pv") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      int first = 0;
      int last = 8191;
      ss >> fn;
      fn = "";
      ss >> fn;
      ss >> first;
      ss >> last;
      if (fn.size() > 0) {
        std::cout << "first: " << first << "  last: " << last << std::endl;
        l() && fL << "first: " << first << "  last: " << last << std::endl;
        this->VariableDump("P", fn, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".qv") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      int first = 0;
      int last = 8191;
      ss >> fn;
      fn = "";
      ss >> fn;
      ss >> first;
      ss >> last;
      if (fn.size() > 0) {
        std::cout << "first: " << first << "  last: " << last << std::endl;
        l() && fL << "first: " << first << "  last: " << last << std::endl;
        this->VariableDump("Q", fn, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".mv") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      int first = 0;
      int last = 8191;
      ss >> fn;
      fn = "";
      ss >> fn;
      ss >> first;
      ss >> last;
      if (fn.size() > 0) {
        std::cout << "first: " << first << "  last: " << last << std::endl;
        l() && fL << "first: " << first << "  last: " << last << std::endl;
        if (bs.find(".mvdef") != std::string::npos) {
          this->MVariableDefinitionDump(fn, first, last);
        } else {
          this->VariableDump("M", fn, first, last);
        }
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".q") == 0) {
      if (l()) {
        this->StopLog();
      }
      free(buf);
      return;
    } else if (bs.find(".l") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->StartLog(fn);
      } else {
        this->StopLog();
      }
    } else if (bs.find(".d") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->DownloadFile(fn);
      } else {
        std::cout << "Usage: .d [file]" << std::endl;
      }
    } else if (bs.find(".g") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->ListGather(fn);
      } else {
        std::cout << "Usage: .g [file]" << std::endl;
      }
    } else if (bs.find(".b") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->MakeBackup(fn);
      } else {
        std::cout << "saving to backup.CFG" << std::endl;
        this->MakeBackup("backup.CFG");
      }
    } else {
      this->SendLine(buf);
      this->GetBuffer();
    }

    if (Logging && LogFileName != "" && !LogFile.is_open()) {
      LogFile.open(LogFileName);
      if (!LogFile.is_open()) {
        std::cerr << "ERROR: cannot open logfile: " << LogFileName << std::endl;
        Logging = false;
      }
    }


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
    l() && fL << std::bitset<8>(fData[0]).to_ulong() << "."
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
  // Returns : Number of errors
  //  -1    : Cannot open file
  //   0    : No errors
 
  std::cout << "Downloading file: " << InFileName << std::endl;
  l() && fL << "Downloading file: " << InFileName << std::endl;

  static int NFileDepth = 0;
  ++NFileDepth;

  // Simple safeguard against recursive inclusion
  if (NFileDepth >= 50) {
    std::cerr << "Error: Backing out due to file depth limit reached.  Possibly you have a recursion on #include <file>" << std::endl;
    l() && fL << "Error: Backing out due to file depth limit reached.  Possibly you have a recursion on #include <file>" << std::endl;
    return 1;
  }
  // Open file for reading
  std::ifstream fi(InFileName);
  if (!fi.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    l() && fL << "ERROR: cannot open file: " << InFileName << std::endl;
    return -1;
  }


  bool IgnoreInput = false;

  // Loop over each line in file
  int i = 0;
  for (std::string Line; std::getline(fi, Line); ++i) {

    size_t begin_ignore = Line.find("/*");
    size_t end_ignore = Line.find("*/");

    while (begin_ignore != std::string::npos && end_ignore != std::string::npos) {
      Line = std::string(Line.begin(), Line.begin() + begin_ignore) + std::string(Line.begin() + end_ignore + 2, Line.end());
      begin_ignore = Line.find("/*");
      end_ignore = Line.find("*/");
    }

    if (IgnoreInput && end_ignore != std::string::npos) {
      IgnoreInput = false;
      Line = std::string(Line.begin() + end_ignore + 2, Line.end());
    } else if (IgnoreInput) {
      continue;
    }

    if (!IgnoreInput && begin_ignore != std::string::npos) {
      IgnoreInput = true;
      Line = std::string(Line.begin(), Line.begin() + begin_ignore);
    }

    // Look for comment chars
    size_t comment_pos = Line.find(";");
    size_t first_comment_pos = comment_pos;
    comment_pos = Line.find("//");
    if (comment_pos < first_comment_pos) {
      first_comment_pos = comment_pos;
    }
    if (first_comment_pos != std::string::npos) {
      Line = std::string(Line.begin(), Line.begin() + first_comment_pos);
    }

    size_t include_pos = Line.find("#include ");
    if (include_pos != std::string::npos) {
      std::istringstream incstring(std::string(Line.begin() + include_pos + 9, Line.end()));
      std::string key;
      incstring >> key;
      if (key.size() < 3) {
        std::cerr << "Error in #include statement: " << Line << std::endl;
        l() && fL << "Error in #include statement: " << Line << std::endl;
        --NFileDepth;
        return 1;
      }
      std::string NewFileName = std::string(key.begin() + 1, key.end() -1);
      this->DownloadFile(NewFileName);
      Line = "";
    }


    size_t define_pos = Line.find("#define ");
    if (define_pos != std::string::npos) {
      std::istringstream defstring(std::string(Line.begin() + define_pos + 8, Line.end()));
      std::string key;
      defstring >> key;

      std::string value = std::string(Line.begin() + Line.find(key) + key.size() + 1, Line.end());
      if (Line.find_last_not_of(" \t\f\v\n\r") != std::string::npos) {
        value = std::string(value.begin(), value.begin() + value.find_last_not_of(" \t\f\v\n\r") + 1);
      }
      if (value.find_first_not_of(" \t\f\v\n\r") != std::string::npos) {
        value = std::string(value.begin() + value.find_first_not_of(" \t\f\v\n\r"), value.end());
      }
      this->AddDefinePair(key, value);

      Line = std::string(Line.begin(), Line.begin() + define_pos);
    }

    Line += '\0';

    Line = this->ReplaceDefines(Line);

    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);

    // Check for an error
    if (fData[3] == 0x80) {
      std::cerr << "Error at line " << i << " in file " << InFileName << std::endl;
      std::cerr << "  Input was: " << Line << std::endl;
    }

  }

  --NFileDepth;
  if (NFileDepth == 0) {
    // Remove dictionary
    this->ClearDefinePairs();
  }

  return 0;
}




void PMAC2Turbo::WriteBuffer (std::string const& Buffer)
{
  // Send a command line to PMAC
 
  this->Flush();

  bool const EndsWithNULL = Buffer.back() == '\0';

  //std::string::iterator itFrom = Buffer.begin();
  //std::string::iterator itTo = Buffer.end();

  /*
  if (Buffer.size() < ToPosition) {
    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);
    int check =  fData[3];
    std::cout << check << std::endl;
    if (fData[3] == 0x80) {
      std::cout << "HELLO ERROR" << std::endl;
    }
  }

  while (ToPosition < Buffer.size()) {
    for ( ; ToPosition > FromPosition; --ToPosition) {
      if (
    }
  }



  for (std::string Line; std::getline(fi, Line); ) {

    std::cout << Line << std::endl;
    Line += '\0';

    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);
    int check =  fData[3];
    std::cout << check << std::endl;
    if (fData[3] == 0x80) {
      std::cout << "HELLO ERROR" << std::endl;
    }
    PrintBits(fData[0]);
    PrintBits(fData[1]);
    PrintBits(fData[2]);
    PrintBits(fData[3]);
  }
  */

  return;
}




std::string PMAC2Turbo::GetResponseString (std::string const& Line)
{
  // Send a command line to PMAC
 
  this->Flush();

  // For the output commands
  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request     = VR_PMAC_GETRESPONSE;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(Line.size());

  strncpy((char*) &fEthCmd.bData[0], Line.c_str(),  Line.size());
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
  recv(fSocket, fData, 1400, 0);

  char ret[1400];
  for (int i = 0, j = 0; i != 1400; ++i) {
    if (fData[i] == ACK) {
      ret[j++] = '\0';
      break;
    } else if (fData[i] == CTRLM) {
      ret[j++] = ' ';
    } else {
      ret[j++] = fData[i];
    }
  }


  return std::string(ret);
}




void PMAC2Turbo::GetBuffer (std::string const& OutFileName, std::ostream* so, bool const cout)
{
  // File for writing if name given
  if (so != 0x0 && !so->good()) {
    std::cerr << "ERROR: output stream is not good" << std::endl;
  }

  std::ofstream* fo = 0x0;
  if (OutFileName != "") {
    fo = new std::ofstream(OutFileName.c_str());
    if (!fo->is_open()) {
      std::cerr << "ERROR: cannot open OutFileName: " << OutFileName << std::endl;
    }
  }

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
        cout && std::cout << fData[j];
        so   &&       *so << fData[j];
        fo   &&       *fo << fData[j];
        l() && fL << fData[j];
      }
      if (!ack_found) {
        cout && std::cout << std::endl;
        so   &&       *so << std::endl;
        fo   &&       *fo << std::endl;
        l() && fL << std::endl;
      }

//      if (fo != 0x0) {
//        for (int j = 0; j < cr_at; ++j) {
//          *fo << fData[j];
//        }
//        if (!ack_found) {
//          *fo << std::endl;
//        }
//      }
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

  if (fo) {
    try {
      fo->close();
      delete fo;
    } catch (...) {
      std::cerr << "Warning: Closing and delete file object is in error state" << std::endl;
      // do nothing
    }
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







void PMAC2Turbo::VariableDump (std::string const& V, std::string const& OutFileName, int const First, int const Last)
{
  // Check if socket at least defined
  if (fSocket < 0) {
    std::cerr << "ERROR: Trying to VariableDump but socket not created" << std::endl;
    return;
  }

  // Open file for writing
  std::ofstream fo(OutFileName);
  if (!fo.is_open()) {
    std::cerr << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
    l() && fL << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
  }

  char command[20];
  std::string response = "";
  for (int i = First; i < Last; ++i) {
    sprintf(command, "%s%04i", V.c_str(), i);
    response = this->GetResponseString(command);
    std::cout << command << "=" << response << std::endl;
    l() && fL << command << "=" << response << std::endl;
    if (fo.is_open()) {
      fo << command << "=" << response << std::endl;
    }
  }

  fo.close();
  return;
}






void PMAC2Turbo::MVariableDefinitionDump (std::string const& OutFileName, int const First, int const Last)
{
  // Check if socket at least defined
  if (fSocket < 0) {
    std::cerr << "ERROR: Trying to MVariableDefinitionDump but socket not created" << std::endl;
    return;
  }

  // Open file for writing
  std::ofstream fo(OutFileName);
  if (!fo.is_open()) {
    std::cerr << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
    l() && fL << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
  }

  char command[20];
  std::string response = "";
  for (int i = First; i < Last; ++i) {
    sprintf(command, "M%04i->", i);
    response = this->GetResponseString(command);
    std::cout << command << response << std::endl;
    l() && fL << command << response << std::endl;
    if (fo.is_open()) {
      fo << command << "=" << response << std::endl;
    }
  }

  fo.close();
  return;
}






void PMAC2Turbo::MakeBackup (std::string const& OutFileName)
{
  // Check if socket at least defined
  if (fSocket < 0) {
    std::cerr << "ERROR: Trying to MakeBackup but socket not created" << std::endl;
    return;
  }

  // Open file for writing
  std::ofstream fo(OutFileName);
  if (!fo.is_open()) {
    std::cerr << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
    l() && fL << "ERROR: cannot open file for writing: " << OutFileName << std::endl;
  }


  // For collecting data from GetBuffer (faster than individual calls)
  std::ostringstream oss;
  std::istringstream iss;
  std::string s;


  int const First = 0;
  int const Last  = 8191;
  char command[100];


  /*

  std::cout << "Uploading I Variables" << std::endl;
  l() && fL << "Uploading I Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; I-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->SendLine("I0..8191");
  this->GetBuffer("", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    fo << "I" << i << "=" << s << std::endl;
  }



  std::cout << "Uploading P Variables" << std::endl;
  l() && fL << "Uploading P Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; P-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->SendLine("P0..8191");
  this->GetBuffer("", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    fo << "P" << i << "=" << s << std::endl;
  }



  std::cout << "Uploading Q Variables" << std::endl;
  l() && fL << "Uploading Q Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; Q-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->SendLine("Q0..8191");
  this->GetBuffer("", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    fo << "Q" << i << "=" << s << std::endl;
  }



  std::cout << "Uploading M Variable definitions" << std::endl;
  l() && fL << "Uploading M Variable definitions" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; M-Variable Definitions ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->SendLine("M0..8191->");
  this->GetBuffer("", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    fo << "M" << i << "->" << s << std::endl;
  }


  */

  std::cout << "Uploading PLCs" << std::endl;
  l() && fL << "Uploading PLCs" << std::endl;

  fo << ";;;;;;;;;;" << std::endl;
  fo << ";; PLCs ;;" << std::endl;
  fo << ";;;;;;;;;;" << std::endl << std::endl;
  for (int i = 0; i != 32; ++i) {
    sprintf(command, "LIST PLC %i", i);

    this->SendLine(command);
    this->GetBuffer("", &oss, false);
    std::string mystr = oss.str();
    oss.str("");
    if (mystr.size() > 0) {
      size_t pos_ret = mystr.find("RET");
      if (pos_ret != std::string::npos) {
        mystr.replace(pos_ret, 3, "CLOSE");
      }
      fo << "OPEN PLC " << i << " CLEAR" << std::endl;
      fo << mystr << std::endl;;
    }
  }



  std::cout << "Uploading COORDINATE SYSTEMS" << std::endl;
  l() && fL << "Uploading COORDINATE SYSTEMS" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; COORDINATE SYSTEMS ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  fo << "UNDEFINE ALL" << std::endl;
  for (int i = 1; i <= 16; ++i) {
    fo << ";; CS " << i << std::endl;
    sprintf(command, "&%i", i);
    fo << command << std::endl;
    this->SendLine(command);
    this->Flush();

    std::string mystr = "";
    size_t pos_ret = std::string::npos;

    for (int im = 1; im <= 32; ++im) {
      sprintf(command, "#%i->", im);
      this->SendLine(command);

      this->GetBuffer("", &oss, false);
      fo << "#" << im << "->" << oss.str();
      oss.str("");
    }

    this->SendLine("list forward");
    this->GetBuffer("", &oss, false);
    mystr = oss.str();
    oss.str("");
    pos_ret = mystr.find("RET");
    if (pos_ret != std::string::npos) {
      mystr.replace(pos_ret, 3, "CLOSE");
      fo << "OPEN FORWARD CLEAR\n" << mystr << std::endl;
    }

    this->SendLine("list inverse");
    this->GetBuffer("", &oss, false);
    mystr = oss.str();
    oss.str("");
    pos_ret = mystr.find("RET");
    if (pos_ret != std::string::npos) {
      mystr.replace(pos_ret, 3, "CLOSE");
      fo << "OPEN INVERSE CLEAR\n" << mystr << std::endl;
    }

    fo << std::endl;
  }



  std::cout << "Uploading Motion Programs" << std::endl;
  l() && fL << "Uploading Motion Programs" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; Motion Programs ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;;;;;" << std::endl << std::endl;
  // Technically this can go to 32767
  for (int i = 0; i <= 32; ++i) {
    sprintf(command, "LIST PROG %i", i);

    this->SendLine(command);
    this->GetBuffer("", &oss, false);
    std::string mystr = oss.str();
    oss.str("");
    if (mystr.size() > 0) {
      size_t pos_ret = mystr.find("RET");
      if (pos_ret != std::string::npos) {
        mystr.replace(pos_ret, 3, "CLOSE");
        fo << "OPEN PROG " << i << " CLEAR" << std::endl;
        fo << mystr << std::endl;;
      }
    }
  }



  fo.close();


  return;
}


















int PMAC2Turbo::AddDefinePair (std::string const& Key, std::string const& Value)
{
  for (std::vector<std::pair<std::string, std::string> >::iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
    if (Key == it->first) {
      std::cerr << "Error: #define key already seen.  Ignoring redefinition: " << Key << " " << Value << std::endl;
      l() && fL << "Error: #define key already seen.  Ignoring redefinition: " << Key << " " << Value << std::endl;
      return 1;
    }
  }
  fDefinePairs.push_back(std::make_pair(Key, Value));
  std::sort(fDefinePairs.begin(), fDefinePairs.end(), CompareDefinePair);
  return 0;
}




void PMAC2Turbo::ClearDefinePairs ()
{
  fDefinePairs.clear();
}



void PMAC2Turbo::PrintDefinePairs ()
{
  for (size_t i = 0; i != fDefinePairs.size(); ++i) {
  }
}




std::string PMAC2Turbo::ReplaceDefines (std::string const& IN)
{
  std::string OUT = IN;

  size_t pos;
  for (std::vector<std::pair<std::string, std::string> >::iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
    pos = OUT.find(it->first);
    while (pos != std::string::npos) {
      OUT = std::string(OUT.begin(), OUT.begin() + pos) + it->second + std::string(OUT.begin() + pos + it->first.size(), OUT.end());
      pos = OUT.find(it->first);
    }
  }

  return OUT;
}





void PMAC2Turbo::PrintBits (char c)
{
  std::bitset<8> b(c);
  std::cout << b << " " << b.to_ulong() << std::endl;
  l() && fL << b << " " << b.to_ulong() << std::endl;
  return;
}
