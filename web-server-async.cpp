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
#include <iterator>
#include <thread>

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

  fd_set readFds;
  fd_set errFds;
  fd_set watchFds;
  FD_ZERO(&readFds);
  FD_ZERO(&errFds);
  FD_ZERO(&watchFds);

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int maxSockfd = sockfd;

  // put the socket in the socket set
  FD_SET(sockfd, &watchFds);

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


  // initialize timer (2s)
  struct timeval tv;
  vector<thread> threads;

  // listen for and accept a new connection
  while (true) {
    // setup watcher
    int nReadyFds = 0;
    readFds = watchFds;
    errFds = watchFds;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if ((nReadyFds = select(maxSockfd + 1, &readFds, NULL, &errFds, &tv)) == -1) {
      perror("select");
      return 4;
    }

    if (nReadyFds == 0) {
      cout << "no data is received for 3 seconds!" << endl;
    }
    else {
      for (int fd = 0; fd <= maxSockfd; fd++) {
        // get one socket for reading
        if (FD_ISSET(fd, &readFds)) {
          if (fd == sockfd) { // this is the listen socket
            struct sockaddr_in clientAddr;
            socklen_t clientAddrSize = sizeof(clientAddr);
            int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSockfd == -1) {
              perror("accept");
              return 4;
            }

            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(clientAddr.sin_family, &(clientAddr).sin_addr, ipstr, sizeof(ipstr));
            std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            std::cout << "Accept a connection from: " << ipstr << ":" <<
            ntohs((clientAddr).sin_port) << std::endl << std::endl;

            // update maxSockfd
            if (maxSockfd < clientSockfd)
              maxSockfd = clientSockfd;

            // add the socket into the socket set
            FD_SET(clientSockfd, &watchFds);
          }
          else { // this is the normal socket

            // Size of recv/send buffer
            int bufSize = 1;

            char buf[MAXBYTES] = {0};
            std::stringstream ss;
            memset(buf, '\0', sizeof(buf));
            int bytesRecv;
            int count = 0; // total bytes received
            size_t foundHeaderEnd;
            vector<char> msg;

            std::cout << "//////////////////////////////////////////////////////////////////////////////\n";
            std::cout << "\nMessage received:\n\n";
            while ((bytesRecv = read(fd, buf, bufSize)) > 0) {
              // TODO: Implement HTTP Timeout, say if server received part of
              // message, but hasnt received the rest of the message for a long time
              std::cout << buf;
              ss << buf;
              msg.push_back(*buf);
              count += bytesRecv;


              foundHeaderEnd = ss.str().find("\r\n\r\n");
              if (foundHeaderEnd != string::npos)
                break;

              memset(buf, '\0', sizeof(buf));
            }

            std::cout << endl << "finished recving " << count << " bytes\n\n";
            // End of while loop
            if (bytesRecv == -1) {
              perror("recv");
              return 5;
            }

            // TODO:
            //       error checking on header fields such as version, if bad version !1.0 | !1.1
            //           return 505 HTTP version not supported response
            //       if method (GET/POST) is not GET, then return 501 Not implemented response

            // Creating Request object from received message
            HttpRequest* clientReq = new HttpRequest();
            //clientReq->messageToObject(ss.str());
            std::string tempss = ss.str();
            vector<char> reqVec(tempss.begin(), tempss.end());
            clientReq->messageToObject(msg); // VS reqVec here

            // Finding the hostname
            map<string, string> headers = clientReq->getHeaders();
            map<string,string>::iterator it = headers.find("Host");
            string reqHostname;
            if (it != headers.end())
              reqHostname = headers["Host"];

            std::cout << "//////////////////////////////////////////////////////////////////////////////\n";
            std::cout << "\nRequest Header: \n\n";
            std::cout << "Method: " << clientReq->getMethod() << endl;
            std::cout << "Url: " << clientReq->getUrl() << endl;
            std::cout << "Port: " << clientReq->getPort() << endl;
            std::cout << "Version: " << clientReq->getVersion() << endl;
            std::cout << "Host: " << reqHostname << endl;

          //  TODO: use hostname argument to check if the request is to the right server.
          //  should we be doing this?
            //if (reqHostname != td->tHostname){
            if (!clientReq->isValid()){
              HttpResponse* responseObj = new HttpResponse();
              responseObj->setStatus("400 Bad Request");
              vector<char> responseBlob = responseObj->buildResponse();

              // Sending 404 Response object
              if (send(fd, &responseBlob[0], responseBlob.size(), 0) == -1) {
                perror("send");
                return 6;
              }
              else {
                string s (responseBlob.begin(), responseBlob.end());
                std::cout << "sent: \n" << s << endl;
              }
              delete responseObj;
              printf("closing connection to %d\n", fd);
              close(fd);
              //return 0;
            }

            // Preparing to open file
            std::stringstream filestream;
            std::string line;

            std::string resFilename = clientReq->getUrl();
            streampos size;
            char* memblock;

            // Dealing with root file request
            if (resFilename == "/") {
              resFilename += "index.html";
            }

            // Prepending starting directory to requested filename
            resFilename.insert(0, filedir);

            std::cout << "trying to get file from path: " << resFilename << endl;
            ifstream resFile (resFilename, ios::in|ios::binary);
            if (resFile.good()) {
              std::cout << "//////////////////////////////////////////////////////////////////////////////\n";
              std::cout << "\nOpened file:\n\n";
              resFile.open(resFilename);

              cout << "GOT HERE\n";
              std::ifstream resFile(resFilename, std::ios::binary);
              std::vector<char> payload((std::istreambuf_iterator<char>(resFile)),
                                         std::istreambuf_iterator<char>());
              cout << "1231243532521\n";
              string payloadStr(payload.begin(), payload.end());
              cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~`\n";
              int payloadSize = payload.size();

              cout << "ALKSDFJLKASDJFLAKSDFJ\n";
              // Sending file back to client if it exists and was read correctly
              // send back 200 OK status code
              HttpResponse* responseObj = new HttpResponse();
              responseObj->setStatus("200 Ok");
              responseObj->setHeader("Content-Length:",to_string(payloadSize));
              responseObj->setPayload(payload);
              vector<char> responseBlob = responseObj->buildResponse();
              cout << "SOMEWherE IN THE MIDDLE\n";

              // Sending response object
              if (send(fd, &responseBlob[0], responseBlob.size(), 0) == -1) {
                perror("send");
                return 6;
              }
              else {
                string s (responseBlob.begin(), responseBlob.end());
                std::cout << "sent: " << s << endl;
              }
              cout << "GOT to the end \n";

              resFile.close();
              delete clientReq;
              delete responseObj;
              delete[] memblock;
            }
            // can't open file, return 404 here or other error
            else {
              size = 0;
              std::cout << "404 ERROR\n";
              HttpResponse* responseObj = new HttpResponse();
              responseObj->setStatus("404 Not Found");
              vector<char> responseBlob = responseObj->buildResponse();

              // Sending 404 Response object
              if (send(fd, &responseBlob[0], responseBlob.size(), 0) == -1) {
                perror("send");
                return 6;
              }
              else {
                string s (responseBlob.begin(), responseBlob.end());
                std::cout << "sent: " << s << endl;
              }

              delete responseObj;
            }

            printf("closing connection to %d\n", fd);
            close(fd);
            FD_CLR(fd, &watchFds);
          }
        }
      }
    }
  }
  return 0;
}
