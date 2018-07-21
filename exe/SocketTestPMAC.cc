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

int SocketTest (std::string const& IPADDRESS, int const PORT)
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
  //sprintf(buffer, "GET / HTTP/1.1\r\n\r\n");
  char* message = "GET / HTTP/1.1\r\nHOST: localhost\r\nUser-Agent: none\r\n\r\n";
  send(client, message, strlen(message), 0);
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

  SocketTest(argv[1], atoi(argv[2]));

  return 0;
}
