#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include "http-message.cpp"

#include <iostream>
#include <sstream>
using namespace std;

#define MAXBYTES 512 // most bytes we can receive at a time

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
  int bufSize = 1;

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
      ntohs(clientAddr.sin_port) << std::endl << std::endl;

    if (!fork()) {   // if in here, we're on child process
      char buf[MAXBYTES] = {0};
      std::stringstream ss;
      close(sockfd); // closes listener for new connections
      memset(buf, '\0', sizeof(buf));
      int bytesRecv;
      size_t foundHeaderEnd;

      while ((bytesRecv = read(clientSockfd, buf, bufSize)) > 0) {
        // TODO: Implement HTTP Timeout, say if server received part of
        // message, but hasnt received the rest of the message for a long time
        std::cout << buf;
        ss << buf;

        foundHeaderEnd = ss.str().find("\r\n\r\n");
        //std::cout << buf << "    " << to_string(foundHeaderEnd) << std::endl;
        if (foundHeaderEnd != string::npos)
          break;

        memset(buf, '\0', sizeof(buf));
      }

      std::cout << endl << "finished recving\n";
      // End of while loop
      if (bytesRecv == -1) {
        perror("recv");
        return 5;
      }

      // TODO: Create HttpMessage, if it fails, return a 400 bad request message
      // do error checking on header fields such as version, if bad version !1.0 | !1.1
      // then return 505 HTTP version not supported response
      // if method (GET/POST) is not GET, then return 501 Not implemented response
      HttpRequest* clientReq = new HttpRequest();
      // TODO: need a HttpRequest object constructor from the thing we recieved, not the url
      clientReq->setMethod("GET");
      clientReq->setUrl("/index.html");
      clientReq->setPort(4000);
      clientReq->setVersion(0);
      clientReq->setHeader("Host","localhost");

      map<string, string> headers = clientReq->getHeaders();
      map<string,string>::iterator it = headers.find("Host");
      string reqHostname;
      if (it != headers.end())
        reqHostname = headers["Host"];

      std::cout << "Method: " << clientReq->getMethod() << endl;
      std::cout << "Url: " << clientReq->getUrl() << endl;
      std::cout << "Port: " << clientReq->getPort() << endl;
      std::cout << "Version: " << clientReq->getVersion() << endl;
      std::cout << "Host: " << reqHostname << endl;

//  TODO: use hostname argument to check for valid headers
      // Preparing to open file
      std::stringstream filestream;
      std::string line;

      std::string resFilename = clientReq->getUrl();
      //ifstream resFile (resFilename);
      streampos size;
      char* memblock;


      // Dealing with root file request
      if (resFilename == "/") {
        // check if index exists and is unlocked and accessible
        ifstream infile("index.html");
        if (infile.good())
          resFilename = "index.html";
      }

      // Prepending starting directory to requested filename
      resFilename.insert(0, filedir);

      std::cout << "trying to get file from path: " << resFilename << endl;
      ifstream resFile (resFilename, ios::in|ios::binary|ios::ate);
      // Opening file
      if (resFile.is_open()) {
        /*
        while (getline(resFile, line)) {
          filestream << line << endl;
        }
        */
        size = resFile.tellg();
        memblock = new char [size];
        resFile.seekg (0, ios::beg);
        resFile.read (memblock, size);
        resFile.close();

        // Sending file back to client if it exists and was read correctly
        // send back 200 OK status code
        //if (send(clientSockfd, filestream.str().c_str(), filestream.str().size(), 0) == -1) {
        if (send(clientSockfd, memblock, size, 0) == -1) {
          perror("send");
          return 6;
        }
        else
          //std::cout << "sent: " << filestream.str().c_str() << endl;
          std::cout << "sent: " << memblock << endl;

        delete[] memblock;
      }
      // can't open file, return 404 here or other error
      else {
        size = 0;
        std::cout << "404 ERROR\n";
      }

      printf("closing connection to %d\n", clientSockfd);
      close(clientSockfd);
      exit(0);
    }
    else { // parent still listens for close
      close(clientSockfd);
    }
  }
  return 0;
}
