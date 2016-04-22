#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
//#include "http-message.cpp"

#include <iostream>
#include <sstream>
using namespace std;

int
main(int argc, char* argv[])
{
  if(argc != 2){
    cerr << "please input a URL";
  }
  char * url = argv[1];
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // struct sockaddr_in addr;
  // addr.sin_family = AF_INET;
  // addr.sin_port = htons(40001);     // short, network byte order
  // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  // if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
  //   perror("bind");
  //   return 1;
  // }

  //HttpRequest request(url);
  const char * host = url;//request.getHeaders["Host"].c_str();
  const char * port = "80";//request.getPort();
  struct addrinfo hints;
  struct addrinfo* res;
  // prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  // gets a list of addresses stored in res
  int status = 0;
  if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    return 2;
  }
  //for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
    struct addrinfo* p = res;
    // convert address to IPv4 address
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

    // convert the IP to a string and print it:
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
    std::cout << "  " << ipstr << std::endl;
    // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;
  //}

  //create a struct sockaddr with the given port and ip address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(80);//request.getPort();
  //serverAddr.sin_addr.s_addr = (struct sockaddr_in*)res->ai_addr;
  //serverAddr.sin_port = htons(80);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(ipstr);
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));


  // connect to the server with struct serverAddr
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  //socket for the client
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }


  char ipstr2[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr2, sizeof(ipstr2));
  std::cout << "Set up a connection from: " << ipstr2 << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // send/receive data to/from connection
  bool isEnd = false;
  std::string input;
  char buf[1] = {0};
  std::stringstream ss;
  string test = "GET /classes/111_fall15/index.html HTTP/1.0\r\nHost: www.lasr.cs.ucla.edu\r\n\r\n";

  //while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    //std::cout << "send: ";
    //std::cin >> input;
    if (send(sockfd, test.c_str(), test.size(), 0) == -1) {
      perror("send");
      return 4;
    }

    int count = 0;
    while(count < 5000){
      if (recv(sockfd, buf, 1, 0) == -1) {
        perror("recv");
        return 5;
      }
      ss << buf;
      if(ss.str().find("\r\n\r\n") != -1){
        int content = 14097+1;
        int sum = 0;
        bool last = false;
        while(true){
            int received = recv(sockfd, buf, 1, 0);
            sum += received;
            //cout<<received<<endl;
            if (received == -1) {
                cout<<"error";
                perror("recv");
                return 5;
            }
          //if(received == 30){
            ss<<buf;
            //cout << buf<<endl;
          //}
          if(received == 0)
              break;
            //ss<<buf;
          
          //content -= 30;
        }
        cout <<"sum: "<<sum;
        break;
      }
      //char * bdy = strstr(buf,"\r\n\r\n");
    //   if(bdy != NULL){
    //       //cout<<"\n\n"<<&buf<<" "<<&bdy<<" "<<buf-bdy<<"\n"<<endl;
    //       char * content = new char[14097+1];
    //       //content[14097] = '\0';
    //       int received = recv(sockfd, content, 14097, 0);
    //       if (received == -1) {
    //         perror("recv");
    //         return 5;
    //     }
    // cout<<content;
    // delete content;
    //   break;
    // }
    count++;
    }
    ofstream file;
    file.open("index.html");
    file << ss.str();
    file.close();
    //cout<<ss.str();
    // if (recv(sockfd, buf, 100, 0) == -1) {
    //   perror("recv");
    //   return 5;
    // }
    // //ss << buf << std::endl;
    // //std::cout << "echo: ";
    // std::cout << buf << std::endl;

    //if (ss.str() == "close\n")
    //  break;

    //ss.str("");
  //}

  close(sockfd);

  return 0;
}
