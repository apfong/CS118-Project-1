#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
using namespace std;

#define MAXBYTES 1000 // most bytes we can receive at a time

int main(int argc, char* argv[])
{
	//AF_INET is current default IP
	string hostname = "localhost";
	int port = 4000;
	string filedir = ".";
	if(argc >= 2)
		hostname = argv[1];
	if(argc >= 3)
		port = atoi(argv[2]);
	if(argc >= 4)
		filedir = argv[3];
	if(argc > 4){
		cerr << "incorrect number of arguments";
		return 1;
	}
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *p;
  //struct sockaddr_storage their_addr;
  //socklen_t sin_size;


  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {   // how many connections the queue will hold onto
    perror("listen");
    return 3;
  }

  // read/write data from/into the connection
  bool isEnd = false;
  char buf[MAXBYTES] = {0};
  std::stringstream ss;

  while (!isEnd) {
    // accept a new connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSockfd == -1) {
      perror("accept");
      return 4;
    }

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;

    if (!fork()) {   // if in here, we're on child process
      close(sockfd); // closes listener for child
      while (true) {
        memset(buf, '\0', sizeof(buf));

        if (recv(clientSockfd, buf, 20, 0) == -1) {
          perror("recv");
          return 5;
        }

        /* TODO: implement HTTP transfer (recieving) here*/
        ss << buf << std::endl;
        std::cout << buf << std::endl;


        if (send(clientSockfd, buf, 20, 0) == -1) {
          perror("send");
          return 6;
        }

        if (ss.str() == "close\n") 
          break;

        ss.str("");
      }
      close(clientSockfd);
      exit(0);
    }
    else { // parent still listens for close
      if (recv(clientSockfd, buf, 20, 0) == -1) {
        perror("recv");
        return 5;
      }

      /* Listen for close server */
      ss << buf << std::endl;
      std::cout << buf << std::endl;
      
      if (send(clientSockfd, buf, 20, 0) == -1) {
        perror("send");
        return 6;
      }

      if (ss.str() == "close\n")
        return 0;

      ss.str("");
    }
    close(clientSockfd);
  }
  return 0;
}
