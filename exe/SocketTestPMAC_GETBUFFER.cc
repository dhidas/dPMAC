////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Fri Jul 20 17:21:40 EDT 2018
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <cstdlib>

#define ETHERNETCMDSIZE 8
#define ETHERNET_DATA_SIZE 1492

#define MAX_BUFFER_SIZE 2097152
#define INPUT_SIZE        (ETHERNET_DATA_SIZE+1)  /* +1 to allow space to add terminating ACK */
#define STX   '\2'
#define CTRLB '\2'
#define CTRLC '\3'
#define ACK   '\6'
#define CTRLF '\6'
#define BELL  '\7'
#define CTRLG '\7'
#define CTRLP '\16'
#define CTRLV '\22'
#define CTRLX '\24'

// Ethernet command structure.  The pragma is to foce the compiler to
// pack bytes without bufferring
#pragma pack(1)
typedef struct tagEthernetCmd
{
    unsigned char RequestType;
    unsigned char Request;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength; /* length of bData */
    unsigned char bData[ETHERNET_DATA_SIZE];
} ETHERNETCMD,*PETHERNETCMD;
#pragma pack()

#define ETHERNET_CMD_HEADER ( sizeof(ETHERNETCMD) - ETHERNET_DATA_SIZE )

// PMAC RequestType fields
#define VR_UPLOAD   0xC0
#define VR_DOWNLOAD 0x40

// PMAC Request fields
#define VR_PMAC_SENDLINE      0xB0
#define VR_PMAC_GETLINE       0xB1
#define VR_PMAC_FLUSH         0xB3
#define VR_PMAC_GETMEM        0xB4
#define VR_PMAC_SETMEM        0xB5
#define VR_PMAC_SETBIT        0xBA
#define VR_PMAC_SETBITS       0xBB
#define VR_PMAC_PORT          0xBE
#define VR_PMAC_GETRESPONSE   0xBF
#define VR_PMAC_READREADY     0xC2
#define VR_CTRL_RESPONSE      0xC4
#define VR_PMAC_GETBUFFER     0xC5
#define VR_PMAC_WRITEBUFFER   0xC6
#define VR_PMAC_WRITEERROR    0xC7
#define VR_FWDOWNLOAD         0xCB
#define VR_IPADDRESS          0xE0





int SocketTestPMAC (std::string const& IPADDRESS, int const PORT = 1025)
{


  // Server socket address struct
  struct sockaddr_in server_addr;

  // Create the socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  //int sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    std::cout << "Error establishing socket..." << std::endl;
    return sock;
  }

  // Set server IP and port information
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IPADDRESS.c_str());
  server_addr.sin_port = htons(PORT);

  // Connect socket or return an error
  if (connect(sock,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
    std::cout << "=> Connection to the server port number: " << PORT << std::endl;
  } else {
    std::cerr << "Error when connecting to server " << IPADDRESS << " on port " << PORT << std::endl;
    return -1;
  }

  ETHERNETCMD EthCmd;
  EthCmd.RequestType = VR_DOWNLOAD;
  EthCmd.Request     = VR_PMAC_FLUSH;
  EthCmd.wValue      = 0;
  EthCmd.wIndex      = 0;
  EthCmd.wLength     = 0;
  //EthCmd.bData
  //int rc, iTimeout;
  send(sock, (char*) &EthCmd, ETHERNETCMDSIZE, 0);

  char data[1401];
  recv(sock, (char*) &data, 1, 0);

  std::cout << data  << std::endl;
  if (data[0] == VR_DOWNLOAD) {
    std::cout << "flush ok" << std::endl;
  }


  char* outstr = "i1..100";
  EthCmd.RequestType = VR_DOWNLOAD;
  EthCmd.Request     = VR_PMAC_SENDLINE;
  EthCmd.wValue      = 0;
  EthCmd.wIndex      = 0;
  EthCmd.wLength     = htons(strlen(outstr));
  strncpy((char*) &EthCmd.bData[0], outstr, strlen(outstr));
  std::cout << "sending: " << EthCmd.bData << std::endl;
  send(sock, (char*) &EthCmd, ETHERNETCMDSIZE + strlen(outstr), 0);
  recv(sock, (char*) &data, 1, 0);




  EthCmd.RequestType = VR_UPLOAD;
  EthCmd.Request     = VR_PMAC_READREADY;
  EthCmd.wValue      = 0;
  EthCmd.wIndex      = 0;
  EthCmd.wLength     = htons(2);
  //char data[2];
  send(sock, (char*) &EthCmd, ETHERNETCMDSIZE, 0);
  recv(sock, data, 2, 0);

  int call = 0;
  while (data[0] == 1) {
    //std::cout << "call " << ++call << std::endl;

    EthCmd.RequestType = VR_UPLOAD;
    EthCmd.Request     = VR_PMAC_GETBUFFER;
    EthCmd.wValue      = 0;
    EthCmd.wIndex      = 0;
    EthCmd.wLength     = 0;
    send(sock, (char*) &EthCmd, ETHERNETCMDSIZE, 0);
    recv(sock, (char*) &data, 1400, 0);

    for (int i = 0; i != 1400; ++i) {
      if (data[i] == ACK) {
        //std::cout << "ACK found at i: " << i << std::endl;
      }
      if (data[i] == 0xD) {
        //std::cout << "CR found at i: " << i << std::endl;
      }
    }
    std::cout << data << std::endl;

    EthCmd.RequestType = VR_UPLOAD;
    EthCmd.Request     = VR_PMAC_READREADY;
    EthCmd.wValue      = 0;
    EthCmd.wIndex      = 0;
    EthCmd.wLength     = htons(2);
    //char data[2];
    send(sock, (char*) &EthCmd, ETHERNETCMDSIZE, 0);
    recv(sock, data, 2, 0);
  }

  std::cout << "done reading data" << std::endl;




  //int j = 0;
  //char dataword[4001];

  //bool ackfound = false;
  //while (!ackfound) {
  //for (int i = 0; i != 1400; ++i) {
  //  if (data[i] == ACK) {
  //    ackfound = true;
  //    break;
  //  }
  //  if (data[i] == 0x0d || i == 1400-1) {
  //    dataword[j] = '\0';
  //    j = 0;
  //    std::cout << "data: " << dataword << std::endl;
  //  } else if (data[i] != BELL) {
  //    dataword[j++] = data[i];
  //  } else {
  //    std::cout << "ERROR BELL" << std::endl;
  //    break;
  //  }
  //}



  close(sock);
  return 0;
}




int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " HOST PORT" << std::endl;
    return 1;
  }

  SocketTestPMAC(argv[1], atoi(argv[2]));

  return 0;
}
