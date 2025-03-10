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
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
//#include <cstdlib>
#include <bitset>
#include <iomanip>

#include <readline/readline.h>
#include <readline/history.h>

PMAC2Turbo::PMAC2Turbo ()
{
  // Default constructor
  fSocket = -1;

  // IfCounter
  fIfTotal = 0;
}



PMAC2Turbo::PMAC2Turbo (std::string const& IP, int const PORT)
{
  // Constructor
  // Arguments:
  //  IP - IP address as a string, e.g. "192.168.1.103"
  //  PORT - The port number to connect to as an int.  Typically 1025

  fSocket = -1;

  this->Connect(IP, PORT);

  // IfCounter
  fIfTotal = 0;
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



bool PMAC2Turbo::Check ()
{
  if (fSocket < 0) {
    std::cerr << "ERROR: Cannot create socket" << std::endl;
    l() && fL << "ERROR: Cannot create socket" << std::endl;
    return false;
  }

  return true;
}




void PMAC2Turbo::Connect (std::string const& IP, int const PORT)
{
  // Store IP and PORT
  fIP = IP;
  fPORT = PORT;

  struct addrinfo hints, *res;

  std::string port = std::to_string(PORT);

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = PF_INET;

  int err;
  if ((err = getaddrinfo(IP.c_str(), port.c_str(), &hints, &res)) != 0)
  {
    std::cerr << "ERROR: Cannot create socket" << std::endl;
    l() && fL << "ERROR: Cannot create socket" << std::endl;
    return;
  }

  // Create and connect the socket
  fSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fSocket < 0) {
    std::cerr << "ERROR: Cannot create socket" << std::endl;
    l() && fL << "ERROR: Cannot create socket" << std::endl;
  }

  // Connect socket or return an error
  if (connect(fSocket, res->ai_addr, res->ai_addrlen) == 0) {
  } else {
    std::cerr << "Error when connecting to server " << IP << " on port " << PORT << std::endl;
    l() && fL << "Error when connecting to server " << IP << " on port " << PORT << std::endl;
    return;
  }

  freeaddrinfo(res);

  // To establish correct communication protocol
  this->SendLine("I3=2");
  this->Flush();

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
  fL.open(OutFileName.c_str());
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

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  this->SendLine("$$$");
  sleep(2);
  this->Flush();

  return;
}




void PMAC2Turbo::FactoryReset()
{
  // Send $$$ to the controller, pause, and flush

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

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

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  std::cout << "saving" << std::endl;
  l() && fL << "saving" << std::endl;
  this->GetResponse("save");
  sleep(4);
  this->Flush();

  return;
}




#define NWORDS 16
const char *words[NWORDS] = {
".help ",
".quit ",
".download ",
".logging ",
".gather ",
".backup ",
".ivars ",
".pvars ",
".qvars ",
".mvars ",
".mdefs ",
".cat ",
".ip ",
".watch ",
".cleardefs",
".?"
};

// Generator function for word completion.
char *my_generator (const char *text, int state)
{

  static int list_index, len;
  const char *name;

  if (!state) {
    list_index = 0;
    len = strlen (text);
  }

  while (list_index < NWORDS) {
    name = words[list_index];

    list_index++;
    if (strncmp (name, text, len) == 0) {
      return strdup (name);
    }
  }

  // If no names matched, then return NULL.
  return ((char *) NULL);
}


// Custom completion function
static char **my_completion (const char *text, int start, int end)
{
  // This prevents appending space to the end of the matching word
  rl_completion_append_character = '\0';

  char **matches = (char **) NULL;
  if (start == 0) {
    matches = rl_completion_matches ((char *) text, &my_generator);
  }

  // else rl_bind_key ('\t', rl_abort);
  return matches;
}







void PMAC2Turbo::Terminal ()
{
  // Simple terminal for PMAC2 using the readline library

  // Start with a flushed buffer
  this->Flush();

  rl_bind_key (CTRLK, rl_insert);
  //rl_bind_key (CTRLK, static_cast<PMAC2Turbo*>(this)->SendCTRLK);
  rl_bind_key (CTRLD, rl_insert);

  rl_attempted_completion_function = my_completion;


  bool Logging = false;
  std::string LogFileName = "";
  std::ofstream LogFile;


  char* buf;
  while ((buf = readline(">> ")) != NULL) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }

    l() && fL << ">> " << buf << std::endl;

    std::string bs = std::string(buf);
    size_t first_nws = bs.find_first_not_of(" \t\f\v\n\r");
    if (first_nws != 0 && first_nws != std::string::npos) {
      bs = std::string(bs.begin() + first_nws, bs.end());
    }

    if (bs == ".help") {
      std::cout << "Commands:" << std::endl;
      std::cout << "  .help                              - print help" << std::endl;
      std::cout << "  .quit                              - quit" << std::endl;
      std::cout << "  .download [file]                   - Download file to pmac" << std::endl;
      std::cout << "  .logging  [file]                   - logging (without [file] is to stop, with will log to file" << std::endl;
      std::cout << "  .gather   [file]                   - Upload gather buffer from pmac to file" << std::endl;
      std::cout << "  .backup   [file]                   - Upload backup CFG from pmac to file" << std::endl;
      std::cout << "  .ivars    [file] [start] [stop]    - dump I variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .pvars    [file] [start] [stop]    - dump P variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .qvars    [file] [start] [stop]    - dump Q variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .mvars    [file] [start] [stop]    - dump M variables to file (start stop optional integers)" << std::endl;
      std::cout << "  .mdefs    [file] [start] [stop]    - dump M variable definitions to file (start stop optional integers)" << std::endl;
      std::cout << "  .cat      [file] [start] [stop]    - print file from line start to stop" << std::endl;
      std::cout << "  .ip       [addr]                   - get ip, or set if [addr] is given" << std::endl;
      std::cout << "  .watch    [cmds]                   - watch vars " << std::endl;
      std::cout << "  .cleardefs                         - clear all #define in memory" << std::endl;
      std::cout << "  .?                                 - print motor status of current motor " << std::endl;
    } else if (bs == "$$$") {
      this->Reset();
    } else if (bs == "$$$***") {
      this->FactoryReset();
    } else if (bs == "save") {
      this->Save();
    } else if (bs.find(".ivars") == 0) {
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
        this->VariableDump("I", fn, 0x0, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".pvars") == 0) {
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
        this->VariableDump("P", fn, 0x0, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".qvars") == 0) {
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
        this->VariableDump("Q", fn, 0x0, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".mvars") == 0) {
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
        this->VariableDump("M", fn, 0x0, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".mdefs") == 0) {
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
        this->MVariableDefinitionDump(fn, 0x0, first, last);
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".cat") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      int first = 0;
      int last = 999999;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (ss.eof()) {
        first = 0;
      }
      ss >> first;
      if (ss.eof()) {
        last = 999999;
      }
      ss >> last;
      if (fn.size() > 0) {
        int iline = 0;
        std::ifstream fi(fn.c_str());
        if (!fi.is_open()) {
          std::cerr << "ERROR: cannot open file: " << fn << std::endl;
          l() && fL << "ERROR: cannot open file: " << fn << std::endl;
          continue;
        }

        for (std::string lfi; std::getline(fi, lfi); ++iline) {
          if (iline >= first && iline <= last) {
            std::cout << lfi << std::endl;
            l() && fL << lfi << std::endl;
          } else if (iline > last) {
            break;
          }
        }
      } else {
        std::cerr << "ERROR: no filename given" << std::endl;
      }
    } else if (bs.find(".quit") == 0) {
      if (l()) {
        this->StopLog();
      }
      free(buf);
      return;
    } else if (bs.find(".logging") == 0) {
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
    } else if (bs.find(".download") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->DownloadFile(fn);
      } else {
        std::cout << "Usage: .download [file]" << std::endl;
      }
    } else if (bs.find(".gather") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->ListGather(fn);
      } else {
        std::cout << "Usage: .gather [file]" << std::endl;
      }
    } else if (bs.find(".backup") == 0) {
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
    } else if (bs.find(".ip") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      fn = "";
      ss >> fn;
      if (fn.size() > 0) {
        this->IPAddress(fn);
      } else {
        this->IPAddress();
      }
    } else if (bs.find(".watch") == 0) {
      std::istringstream ss(bs);
      std::string fn;
      ss >> fn;
      std::vector<std::string> myvars;
      while(ss >> fn) {
        myvars.push_back(fn);
      }

      std::string watchline = "";
      for (size_t iwatch = 0; iwatch != myvars.size(); ++iwatch) {
        watchline += myvars[iwatch] + ' ';
      }
      std::ostringstream oss;

      while (true) {
        oss.str("");
        this->GetResponse(watchline, "", &oss, false);
        std::string myout = oss.str();

        size_t bsn = myout.find('\n');
        while (bsn != std::string::npos) {
          myout.replace(bsn, 1, "   ");
          bsn = myout.find('\n');
        }
        std::cout << myout << std::endl;
        sleep(1);
      }
    } else if (bs.find(".cleardefs") == 0) {
        this->ClearDefinePairs();
    } else if (bs.find(".?") == 0) {
      if (this->Check()) {
        std::string mstatus = this->GetResponseString("?");
        if (mstatus.size() == 12) {
          std::cout << mstatus.substr(0, 6) << " " << mstatus.substr(6, 6) << std::endl;


          const char* w = mstatus.c_str();

          std::cout << ((w[0]  & (1<<3))!=0) << " (bit 23) Motor Activated\n";
          std::cout << ((w[0]  & (1<<2))!=0) << " (bit 22) Negative End Limit Set\n";
          std::cout << ((w[0]  & (1<<1))!=0) << " (bit 21) Positive End Limit Set\n";
          std::cout << ((w[0]  & (1<<0))!=0) << " (bit 20) Extended Servo Algorithm Enabled\n";

          std::cout << ((w[1]  & (1<<3))!=0) << " (bit 19) Amplifier Enabled\n";
          std::cout << ((w[1]  & (1<<2))!=0) << " (bit 18) Open Loop Mode\n";
          std::cout << ((w[1]  & (1<<1))!=0) << " (bit 17) Move Timer Active\n";
          std::cout << ((w[1]  & (1<<0))!=0) << " (bit 16) Integration Mode\n";

          std::cout << ((w[2]  & (1<<3))!=0) << " (bit 15) Dwell in Progress\n";
          std::cout << ((w[2]  & (1<<2))!=0) << " (bit 14) Data Block Error\n";
          std::cout << ((w[2]  & (1<<1))!=0) << " (bit 13) Desired Velocity Zero\n";
          std::cout << ((w[2]  & (1<<0))!=0) << " (bit 12) Abort Deceleration\n";

          std::cout << ((w[3]  & (1<<3))!=0) << " (bit 11) Block Request\n";
          std::cout << ((w[3]  & (1<<2))!=0) << " (bit 10) Home Search in Progress\n";
          std::cout << ((w[3]  & (1<<1))!=0) << " (bit 09) User-Written Phase Enable\n";
          std::cout << ((w[3]  & (1<<0))!=0) << " (bit 08) User-Written Servo Enable\n";

          std::cout << ((w[4]  & (1<<3))!=0) << " (bit 07) Alternate Source/Destination\n";
          std::cout << ((w[4]  & (1<<2))!=0) << " (bit 06) Phased Motor\n";
          std::cout << ((w[4]  & (1<<1))!=0) << " (bit 05) Following Offset Mode\n";
          std::cout << ((w[4]  & (1<<0))!=0) << " (bit 04) Following Enabled\n";

          std::cout << ((w[5]  & (1<<3))!=0) << " (bit 03) Error Trigger\n";
          std::cout << ((w[5]  & (1<<2))!=0) << " (bit 02) Software Position Capture\n";
          std::cout << ((w[5]  & (1<<1))!=0) << " (bit 01) Integrator in Velocity Loop\n";
          std::cout << ((w[5]  & (1<<0))!=0) << " (bit 00) Alternate Command-Output Mode\n";

          std::cout << "\n";

          std::cout << ((w[6]  & (1<<3))!=0) << " (bit 23) (CS-1) # bit 3 (MSB)\n";
          std::cout << ((w[6]  & (1<<2))!=0) << " (bit 22) (CS-1) # bit 2\n";
          std::cout << ((w[6]  & (1<<1))!=0) << " (bit 21) (CS-1) # bit 1\n";
          std::cout << ((w[6]  & (1<<0))!=0) << " (bit 20) (CS-1) # bit 0\n";

          std::cout << ((w[7]  & (1<<3))!=0) << " (bit 19) Coordinate Definition # bit 3 (MSB)\n";
          std::cout << ((w[7]  & (1<<2))!=0) << " (bit 18) Coordinate Definition # bit 2\n";
          std::cout << ((w[7]  & (1<<1))!=0) << " (bit 17) Coordinate Definition # bit 1\n";
          std::cout << ((w[7]  & (1<<0))!=0) << " (bit 16) Coordinate Definition # bit 0\n";

          std::cout << ((w[8]  & (1<<3))!=0) << " (bit 15) Assigned to C.S\n";
          std::cout << ((w[8]  & (1<<2))!=0) << " (bit 14) (Reserved for future use)\n";
          std::cout << ((w[8]  & (1<<1))!=0) << " (bit 13) Foreground In-Position\n";
          std::cout << ((w[8]  & (1<<0))!=0) << " (bit 12) Stopped on Desired Position Limit\n";

          std::cout << ((w[9]  & (1<<3))!=0) << " (bit 11) Stopped on Position Limit\n";
          std::cout << ((w[9]  & (1<<2))!=0) << " (bit 10) Home Complete\n";
          std::cout << ((w[9]  & (1<<1))!=0) << " (bit 09) Phasing Search/Read Active\n";
          std::cout << ((w[9]  & (1<<0))!=0) << " (bit 08) Phasing Reference Error\n";

          std::cout << ((w[10] & (1<<3))!=0) << " (bit 07) Trigger Move\n";
          std::cout << ((w[10] & (1<<2))!=0) << " (bit 06) Integrated Fatal Following Error\n";
          std::cout << ((w[10] & (1<<1))!=0) << " (bit 05) I2T Amplifier Fault Error\n";
          std::cout << ((w[10] & (1<<0))!=0) << " (bit 04) Backlash Direction Flag\n";

          std::cout << ((w[11] & (1<<3))!=0) << " (bit 03) Amplifier Fault Error\n";
          std::cout << ((w[11] & (1<<2))!=0) << " (bit 02) Fatal Following Error\n";
          std::cout << ((w[11] & (1<<1))!=0) << " (bit 01) Warning Following Error\n";
          std::cout << ((w[11] & (1<<0))!=0) << " (bit 00) In Position\n";

        } else {
          std::cerr << "ERROR: return of ? not expected size: " << mstatus.size() << std::endl;
        }
      }
    } else {
      // Check if socket at least defined
      if (this->Check()) {
        this->Flush();
        this->GetResponse(buf);
      }
    }


    if (Logging && LogFileName != "" && !LogFile.is_open()) {
      LogFile.open(LogFileName.c_str());
      if (!LogFile.is_open()) {
        std::cerr << "ERROR: cannot open logfile: " << LogFileName << std::endl;
        Logging = false;
      }
    }


    // readline malloc's a new buffer every time.
    free(buf);
  }


  std::cout << "leaving terminal" << std::endl;
  return;
}


bool PMAC2Turbo::WaitReady (float const toms, float const ppms)
{

  int const np = int(toms/ppms) + 1;

  // For the output commands
  fEthCmd.RequestType = VR_UPLOAD;
  fEthCmd.Request     = VR_PMAC_READREADY;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(2);

  for (int i = 0; i != np; ++i) {
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
    recv(fSocket, fData, 2, 0);
    if (fData[0] != 0) {
      return true;
    }
    usleep(ppms*1000);
  }
  std::cerr << "PMAC2Turbo::WaitReady failed to see ready" << std::endl;
  return false;
}


bool PMAC2Turbo::Flush ()
{
  // Flush the data buffer in PMAC

  // Check if socket at least defined
  if (!this->Check()) {
    return false;
  }

  // First flush any data in the buffer
  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request     = VR_PMAC_FLUSH;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = 0;
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  recv(fSocket, (char*) &fData, 1, 0);
  if (fData[0] != VR_DOWNLOAD && fData[0] != VR_PMAC_FLUSH) {
    std::cerr << "ERROR: flush failed" << std::endl;
    return false;
  }

  return true;
}





void PMAC2Turbo::IPAddress (std::string const& IP)
{
  // Get or set the IP address

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  sscanf(IP.c_str(), "%hu.%hu.%hu.%hu",
      (unsigned short*) &fEthCmd.bData[0],
      (unsigned short*) &fEthCmd.bData[1],
      (unsigned short*) &fEthCmd.bData[2],
      (unsigned short*) &fEthCmd.bData[3]
      );

  if (IP == "") {
    fEthCmd.RequestType = VR_UPLOAD;
  } else {
    fEthCmd.RequestType = VR_DOWNLOAD;
  }
  fEthCmd.Request     = VR_IPADDRESS;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(4);
  if (IP == "") {
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
  } else {
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + 4, 0);
  }
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
  // Send a ^K

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

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

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  this->Flush();
  usleep(50000);
  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request     = VR_PMAC_SENDLINE;
  fEthCmd.wValue      = 0;
  fEthCmd.wIndex      = 0;
  fEthCmd.wLength     = htons(Line.size());
  strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
  send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
  recv(fSocket, (char*) &fData, 1, 0);

  usleep(50000);
  return;
}




int PMAC2Turbo::DownloadFile (std::string const& InFileName)
{
  // Download a file to PMAC.
  // Returns : Number of errors
  //  -1    : Cannot open file
  //   0    : No errors

  // Check if socket at least defined
  if (!this->Check()) {
    return 1;
  }

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
  std::ifstream fi(InFileName.c_str());
  if (!fi.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    l() && fL << "ERROR: cannot open file: " << InFileName << std::endl;
    return -1;
  }

  // If in an #ifdef ignore, count if and ifn up and endif down
  size_t InIfIgnoreIfCount = 0;

  bool IgnoreCommentInput = false;
  int NErrors = 0;
  int NIfDef = 0;

  // Loop over each line in file
  int i = 1;
  for (std::string Line; std::getline(fi, Line); ++i) {

    // Look for line comment chars
    size_t comment_pos = Line.find(";");
    size_t first_comment_pos = comment_pos;
    comment_pos = Line.find("//");
    if (comment_pos < first_comment_pos) {
      first_comment_pos = comment_pos;
    }
    if (first_comment_pos != std::string::npos) {
      Line = std::string(Line.begin(), Line.begin() + first_comment_pos);
    }


    // For comment chars ignore
    size_t begin_ignore = Line.find("/*");
    size_t end_ignore = Line.find("*/");

    while (begin_ignore != std::string::npos && end_ignore != std::string::npos) {
      Line = std::string(Line.begin(), Line.begin() + begin_ignore) + std::string(Line.begin() + end_ignore + 2, Line.end());
      begin_ignore = Line.find("/*");
      end_ignore = Line.find("*/");
    }

    if (IgnoreCommentInput && end_ignore != std::string::npos) {
      IgnoreCommentInput = false;
      Line = std::string(Line.begin() + end_ignore + 2, Line.end());
    } else if (IgnoreCommentInput) {
      continue;
    }

    if (!IgnoreCommentInput && begin_ignore != std::string::npos) {
      IgnoreCommentInput = true;
      Line = std::string(Line.begin(), Line.begin() + begin_ignore);
    }





    // For ifdef, define.  No second statement is allowed on a line
    size_t const define_pos = Line.find("#define");
    size_t const undef_pos  = Line.find("#undef");
    size_t const ifdef_pos  = Line.find("#ifdef");
    size_t const ifndef_pos = Line.find("#ifndef");
    size_t const else_pos   = Line.find("#else");
    size_t const endif_pos  = Line.find("#endif");


    bool const InIfIgnore = (fDefineStatus.size() > 0 && fDefineStatus.back() == false);

    if (!InIfIgnore && define_pos != std::string::npos) {
      std::string Key, Value;
      this->ParseDefine(Line, Key, Value);
      if (Key.size() == 0) {
        std::cerr << "ERROR: #ifdef statement incomplete in: " << Line << std::endl;
        ++NErrors;
      }
      std::string const ReplacedValue = this->ReplaceDefines(Value);
      this->AddDefinePair(Key, ReplacedValue);
      Line = "";
    } else if (!InIfIgnore && undef_pos != std::string::npos) {
      std::string Key, Value;
      this->ParseDefine(Line, Key, Value);
      if (Key.size() == 0) {
        std::cerr << "ERROR: #ifdef statement incomplete in: " << Line << std::endl;
        ++NErrors;
      }
      this->RemoveDefine(Key);
      Line = "";
    } else if (ifdef_pos != std::string::npos) {
      if (InIfIgnore) {
        ++InIfIgnoreIfCount;
      } else {
        std::string Key, Value;
        this->ParseDefine(Line, Key, Value);
        if (Key.size() == 0) {
          std::cerr << "ERROR: #ifdef statement incomplete in: " << Line << std::endl;
          ++NErrors;
        }
        if (this->DefineKeyExists(Key)) {
          fDefineStatus.push_back(true);
        } else {
          fDefineStatus.push_back(false);
        }
        Line = "";
      }
    } else if (ifndef_pos != std::string::npos) {
      if (InIfIgnore) {
        ++InIfIgnoreIfCount;
      } else {
        std::string Key, Value;
        this->ParseDefine(Line, Key, Value);
        if (Key.size() == 0) {
          std::cerr << "ERROR: #ifdef statement incomplete in: " << Line << std::endl;
          ++NErrors;
        }
        if (!this->DefineKeyExists(Key)) {
          fDefineStatus.push_back(true);
        } else {
          fDefineStatus.push_back(false);
        }
        Line = "";
      }
    } else if (!InIfIgnore && else_pos != std::string::npos) {
      size_t const nstatus = fDefineStatus.size();
      if (nstatus == 0) {
        std::cerr << "ERROR: Syntaxt. #else detected before if" << std::endl;
        return 1;
      }
      fDefineStatus[nstatus-1] = !fDefineStatus[nstatus-1];
      Line = "";
    } else if (endif_pos != std::string::npos) {
      if (InIfIgnore && InIfIgnoreIfCount > 0) {
        --InIfIgnoreIfCount;
      } else {
        if (fDefineStatus.size() == 0) {
          std::cerr << "ERROR: Syntaxt. #endif detected before if" << std::endl;
          return 1;
        }
        fDefineStatus.pop_back();
        Line = "";
      }
    }

    if (fDefineStatus.size() > 0 && fDefineStatus.back() == false) {
      //std::cout << "skipping line: " << Line << std::endl;
      continue;
    }






    size_t include_pos = Line.find("#include");
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
      NErrors += this->DownloadFile(NewFileName);
      Line = "";
    }


    Line += '\0';
    //Line.erase(std::remove(Line.begin(), Line.end(), '\t'), Line.end());
    size_t tab_pos = Line.find('\t');
    while (tab_pos != std::string::npos) {
      Line.replace(tab_pos, 1, " ");
      tab_pos = Line.find('\t');
    }
    size_t ctrlm_pos = Line.find(CTRLM);
    while (ctrlm_pos != std::string::npos) {
      Line.replace(ctrlm_pos, 1, " ");
      ctrlm_pos = Line.find(CTRLM);
    }

    Line = this->ReplaceDefines(Line);

    if (Line.size() == 1) {
      continue;
    }

    //std::cout << "send: " << Line << std::endl;

    fEthCmd.RequestType = VR_DOWNLOAD;
    fEthCmd.Request     = VR_PMAC_WRITEBUFFER;
    fEthCmd.wValue      = 0;
    fEthCmd.wIndex      = 0;
    fEthCmd.wLength     = htons(Line.size());
    strncpy((char*) &fEthCmd.bData[0], Line.c_str(), Line.size());
    send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE + Line.size(), 0);
    recv(fSocket, (char*) &fData, 4, 0);

    //std::cout << "recv: " << std::hex << (unsigned int) fData[3] << std::dec << std::endl;

    // Check for an error
    if (fData[3] == 0x80 || fData[3] == 0xff) {
      std::cerr << "Error " << (unsigned short) fData[2] << " at line " << i << " in file " << InFileName << std::endl;
      l() && fL << "Error " << (unsigned short) fData[2] << " at line " << i << " in file " << InFileName << std::endl;
      std::cerr << "  Input was: " << Line << std::endl;
      l() && fL << "  Input was: " << Line << std::endl;
      ++NErrors;
    }

  }

  --NFileDepth;
  if (NFileDepth == 0) {
    // Remove dictionary
    //this->ClearDefinePairs();
    this->Flush();
  }

  return 0;
}






std::string PMAC2Turbo::GetResponseString (std::string const& Line)
{
  // Send a command line to PMAC

  if (!this->Flush()) {
    return "";
  }

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


  std::string retstr = std::string(ret);
  char last = *retstr.rbegin();
  if (last == '\n' || last == '\r' || last == ' ') {
    retstr = retstr.substr(0, retstr.size() - 1);
  }

  return retstr;
}




void PMAC2Turbo::GetResponse (std::string const& Line, std::string const& OutFileName, std::ostream* so, bool const cout)
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

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }


  fEthCmd.RequestType = VR_DOWNLOAD;
  fEthCmd.Request = VR_PMAC_GETRESPONSE;
  fEthCmd.wValue = 0;
  fEthCmd.wIndex = 0;
  fEthCmd.wLength = htons(Line.size());
  strncpy((char *)&fEthCmd.bData[0], Line.c_str(), Line.size());
  send(fSocket,(char*)&fEthCmd,ETHERNETCMDSIZE + Line.size(), 0);
  usleep(200000);
  recv(fSocket, &fData,READ_SIZE,0);

  bool first = true;
  bool done = false;
  bool bell = false;
  while (!done) {

    for (int i = 0; i != READ_SIZE; ++i) {
      if (fData[i] == BELL || fData[i] == STX) {
        bell = true;
        continue;
      }
      if (fData[i] == CR && bell) {
        done = true;
      }
      if (fData[i] == ACK || fData[i] == LF) {
        done = true;
      }
      if (done) {
        break;
      } else {
        if (fData[i] == CR) {
          fData[i] = '\n';
        }
        cout && std::cout << fData[i];
        so   &&       *so << fData[i];
        fo   &&       *fo << fData[i];
        l()  &&        fL << fData[i];
      }
    }

    if (!done) {
      if (!this->WaitReady()) {
        return;
      }
      fEthCmd.RequestType = VR_UPLOAD;
      fEthCmd.Request     = VR_PMAC_GETBUFFER;
      fEthCmd.wValue      = 0;
      fEthCmd.wIndex      = 0;
      fEthCmd.wLength     = htons(READ_SIZE);
      send(fSocket, (char*) &fEthCmd, ETHERNETCMDSIZE, 0);
      recv(fSocket, (char*) &fData, READ_SIZE, 0);
    }
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
  if (!this->Check()) {
    return;
  }

  // First flush any data in the buffer, send the list gather command,
  // Read the buffer, and flush after
  this->Flush();
  this->GetResponse("LIST GATHER", OutFileName);
  this->Flush();

  return;
}







void PMAC2Turbo::VariableDump (std::string const& V, std::string const& OutFileName, std::ostream* os, int const First, int const Last)
{
  // Dump MVariable definitions to file or stream

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  // Chekc at least one exists
  if (os == 0x0 && OutFileName == "") {
    std::cerr << "ERROR: neither output stream nor file specified" << std::endl;
    return;
  }

  // Check if filename exists and use it for output if so
  std::ofstream* fo = 0x0;
  if (OutFileName != "") {
    fo = new std::ofstream(OutFileName.c_str());
    if (!fo->is_open()) {
      std::cerr << "ERROR: cannot open file" << std::endl;
      return;
    }
  }

  if (fo != 0x0) {
    os = (std::ostream*) fo;
  }

  char command[100];
  std::ostringstream oss;
  std::istringstream iss;
  std::string s;

  sprintf(command, "%s%i..%i", V.c_str(), First, Last);

  this->GetResponse(command, "", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    *os << V << i << "=" << s << std::endl;
  }

  if (fo != 0x0) {
    fo->close();
  }

  return;
}





void PMAC2Turbo::MVariableDefinitionDump (std::string const& OutFileName, std::ostream* os, int const First, int const Last)
{
  // Dump MVariable definitions to file or stream

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  // Chekc at least one exists
  if (os == 0x0 && OutFileName == "") {
    std::cerr << "ERROR: neither output stream nor file specified" << std::endl;
    return;
  }

  // Check if filename exists and use it for output if so
  std::ofstream* fo = 0x0;
  if (OutFileName != "") {
    fo = new std::ofstream(OutFileName.c_str());
    if (!fo->is_open()) {
      std::cerr << "ERROR: cannot open file" << std::endl;
      return;
    }
  }

  if (fo != 0x0) {
    os = (std::ostream*) fo;
  }

  char command[100];
  std::ostringstream oss;
  std::istringstream iss;
  std::string s;

  sprintf(command, "M%i..%i->", First, Last);

  this->GetResponse(command, "", &oss, false);
  iss.str(oss.str());
  oss.str("");
  for (int i = First; i <= Last; ++i) {
    iss >> s;
    *os << "M" << i << "->" << s << std::endl;
  }

  if (fo != 0x0) {
    fo->close();
  }

  return;
}





void PMAC2Turbo::PLCDump (std::string const& OutFileName, std::ostream* os, int const First, int const Last)
{
  // Dump PLCs to file or stream

  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  // Chekc at least one exists
  if (os == 0x0 && OutFileName == "") {
    std::cerr << "ERROR: neither output stream nor file specified" << std::endl;
    return;
  }

  // Check if filename exists and use it for output if so
  std::ofstream* fo = 0x0;
  if (OutFileName != "") {
    fo = new std::ofstream(OutFileName.c_str());
    if (!fo->is_open()) {
      std::cerr << "ERROR: cannot open file" << std::endl;
      return;
    }
  }

  if (fo != 0x0) {
    os = (std::ostream*) fo;
  }

  char command[100];
  std::ostringstream oss;

  for (int i = First; i <= Last; ++i) {
    sprintf(command, "LIST PLC %i", i);

    this->GetResponse(command, "", &oss, false);
    std::string mystr = oss.str();
    oss.str("");
    if (mystr.size() > 0) {
      size_t pos_ret = mystr.find("RET");
      if (pos_ret != std::string::npos) {
        mystr.replace(pos_ret, 3, "CLOSE");
      }
      *os << "OPEN PLC " << i << " CLEAR" << std::endl;
      *os << mystr << std::endl;;
    }
  }

  if (fo != 0x0) {
    fo->close();
  }

  return;
}





void PMAC2Turbo::MakeBackup (std::string const& OutFileName)
{
  // Check if socket at least defined
  if (!this->Check()) {
    return;
  }

  // Open file for writing
  std::ofstream fo(OutFileName.c_str());
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



  std::cout << "Uploading I Variables" << std::endl;
  l() && fL << "Uploading I Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; I-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->VariableDump("I", "", &fo);



  std::cout << "Uploading P Variables" << std::endl;
  l() && fL << "Uploading P Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; P-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->VariableDump("P", "", &fo);



  std::cout << "Uploading Q Variables" << std::endl;
  l() && fL << "Uploading Q Variables" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; Q-Variables ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->VariableDump("Q", "", &fo);



  std::cout << "Uploading M Variable definitions" << std::endl;
  l() && fL << "Uploading M Variable definitions" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; M-Variable Definitions ;;" << std::endl;
  fo << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;" << std::endl << std::endl;

  this->MVariableDefinitionDump("", &fo);



  std::cout << "Uploading PLCs" << std::endl;
  l() && fL << "Uploading PLCs" << std::endl;

  fo << ";;;;;;;;;;" << std::endl;
  fo << ";; PLCs ;;" << std::endl;
  fo << ";;;;;;;;;;" << std::endl << std::endl;

  this->PLCDump("", &fo);



  std::cout << "Uploading COORDINATE SYSTEMS and kinematics" << std::endl;
  l() && fL << "Uploading COORDINATE SYSTEMS and kinematics" << std::endl;

  fo << ";;;;;;;;;;;;;;;;;;;;;;;;" << std::endl;
  fo << ";; COORDINATE SYSTEMS ;;" << std::endl;
  fo << ";;   and kinematics   ;;" << std::endl;
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
      this->GetResponse(command, "", &oss, false);
      fo << "#" << im << "->" << oss.str();
      oss.str("");
    }

    this->GetResponse("list forward", "", &oss, false);
    mystr = oss.str();
    oss.str("");
    pos_ret = mystr.find("RET");
    if (pos_ret != std::string::npos) {
      mystr.replace(pos_ret, 3, "CLOSE");
      fo << "OPEN FORWARD CLEAR\n" << mystr << std::endl;
    }

    this->GetResponse("list inverse", "", &oss, false);
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

    this->GetResponse(command, "", &oss, false);
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











int PMAC2Turbo::ParseDefine (std::string const& Line, std::string& Key, std::string& Value)
{
  // Parse a define statement.  Get Key and Value pair if available.
  // Returns 1 for error, 0 for success

  size_t const ifdef_pos  = Line.find("#ifdef");
  size_t const ifndef_pos = Line.find("#ifndef");
  size_t const define_pos = Line.find("#define");
  size_t const undef_pos  = Line.find("#undef");

  size_t pos = std::string::npos;
  size_t len = 0;
  if (define_pos != std::string::npos) {
    pos = define_pos;
    len = 8;
  } else if (undef_pos != std::string::npos) {
    pos = undef_pos;
    len = 7;
  } else if (ifdef_pos != std::string::npos) {
    pos = ifdef_pos;
    len = 7;
  } else if (ifndef_pos != std::string::npos) {
    pos = ifndef_pos;
    len = 8;
  }


  std::istringstream defstring(std::string(Line.begin() + pos + len, Line.end()));
  defstring >> Key;
  if (Key.size() == 0) {
    std::cerr << "ERROR: #ifdef statement incomplete in: " << Line << std::endl;
    return 1;
  }

  if (Line.begin() + Line.find(Key) + Key.size() + 1 >= Line.end()) {
    Value = "";
    return 0;
  }

  Value = std::string(Line.begin() + Line.find(Key) + Key.size() + 1, Line.end());
  if (Line.find_last_not_of(" \t\f\v\n\r") != std::string::npos) {
    Value = std::string(Value.begin(), Value.begin() + Value.find_last_not_of(" \t\f\v\n\r") + 1);
  }
  if (Value.find_first_not_of(" \t\f\v\n\r") != std::string::npos) {
    Value = std::string(Value.begin() + Value.find_first_not_of(" \t\f\v\n\r"), Value.end());
  }


  return 0;
}




int PMAC2Turbo::AddDefinePair (std::string const& Key, std::string const& Value)
{
  for (std::vector<std::pair<std::string, std::string> >::iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
    if (Key == it->first) {
      if (Value != it->second) {
        std::cerr << "WARNING: #define key already seen and now defined differently.: " << Key << " " << it->second << " -> " << Value << std::endl;
        l() && fL << "WARNING: #define key already seen and now defined differently.: " << Key << " " << it->second << " -> " << Value << std::endl;
        it->second = Value;
        return 0;
      }
      return 0;
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
  std::cout << "*** PrintDefinePairs BEGIN***" << std::endl;
  for (size_t i = 0; i != fDefinePairs.size(); ++i) {
    std::cout << fDefinePairs[i].first << "  " << fDefinePairs[i].second << std::endl;
    l() && fL << fDefinePairs[i].first << "  " << fDefinePairs[i].second << std::endl;
  }
  std::cout << "*** PrintDefinePairs END  ***" << std::endl;
  return;
}




bool PMAC2Turbo::DefineKeyExists (std::string const& Key) const
{
  for (std::vector< std::pair<std::string, std::string> >::const_iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
    if (it->first == Key) {
      return true;
    }
  }

  return false;
}




void PMAC2Turbo::RemoveDefine (std::string const& Key)
{
  for (std::vector< std::pair<std::string, std::string> >::iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
    if (it->first == Key) {
      fDefinePairs.erase(it);
      return;
    }
  }

  return;
}




std::string PMAC2Turbo::ReplaceDefines (std::string const& IN)
{
  std::string OUT = IN;

  // As long as you made a change last time, try again
  bool TryReplace = true;

  while (TryReplace) {
    TryReplace = false;
    size_t pos;
    for (std::vector<std::pair<std::string, std::string> >::iterator it = fDefinePairs.begin(); it != fDefinePairs.end(); ++it) {
      pos = OUT.find(it->first);
      while (pos != std::string::npos) {
        TryReplace = true;
        OUT = std::string(OUT.begin(), OUT.begin() + pos) + it->second + std::string(OUT.begin() + pos + it->first.size(), OUT.end());
        pos = OUT.find(it->first);
      }
    }
  }

  if (false && IN != OUT) {
    std::cout << "IN:  " << IN << std::endl;
    std::cout << "OUT: " << OUT << std::endl;
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
