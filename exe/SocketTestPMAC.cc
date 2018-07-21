////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <dhidas@bnl.gov>
//
// Created on: Fri Jul 20 17:21:40 EDT 2018
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>




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
    unsigned char bData[1492];
} ETHERNETCMD,*PETHERNETCMD;
#pragma pack()



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

  int bufsize = 1024*1024;
  char buffer[bufsize];

  // Server socket address struct
  struct sockaddr_in server_addr;

  // Create the socket
  int client = socket(AF_INET, SOCK_STREAM, 0);
  if (client < 0) {
    std::cout << "Error establishing socket..." << std::endl;
    return client;
  }

  // Set server IP and port information
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IPADDRESS.c_str());
  server_addr.sin_port = htons(PORT);

  // Connect socket or return an error
  if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
    std::cout << "=> Connection to the server port number: " << PORT << std::endl;
  } else {
    std::cerr << "Error when connecting to server " << IPADDRESS << " on port " << PORT << std::endl;
    return -1;
  }

  // Send an HTML GET and read response
  std::string message = "GET / HTTP/1.1\r\nHOST: localhost\r\nUser-Agent: none\r\n\r\n";
  send(client, message.c_str(), message.size(), 0);
  recv(client, buffer, bufsize, 0);
  std::cout << buffer << std::endl;;

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
