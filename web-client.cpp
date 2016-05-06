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
#include <thread>
#include "http-message.cpp"

#include <iostream>
#include <sstream>
#include <iterator>
using namespace std;

void timertimeout(){
  this_thread::sleep_for(chrono::seconds(30));
  cerr<<"asynch timeout"<<endl;
  exit(1);
}

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

  HttpRequest request = HttpRequest();
  request.urlToObject(url);
  string hoststring = request.getHeaders()["Host"];
  string portstring = to_string(request.getPort());
  const char * host = hoststring.c_str();
  const char * port = portstring.c_str();

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


  //create a struct sockaddr with the given port and ip address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(request.getPort());//request.getPort();
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


  // send/receive data to/from connection
  //bool isEnd = false;
  std::string input;
  int bufSize = 1000;
  char buf[bufSize];
  std::stringstream ss;
  //string test = "GET /classes/111_fall15/index.html HTTP/1.0\r\nHost: www.lasr.cs.ucla.edu\r\n\r\n";
  //string test = request.buildRequest();
  vector<char> test = request.buildRequest();
  string s(test.begin(), test.end());

  //while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (send(sockfd, &test[0], test.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    int bytesRecv; // variable to hold how many bytes read each loop of recv
    size_t foundHeaderEnd;
    vector<char> msg;
    HttpResponse* response = new HttpResponse();
    int createdResponse = 0; // bool to tell if response obj was created yet
    int count = 0; // total bytes received
    int payloadLen = 0;

    thread timeout(timertimeout);
    timeout.detach();

    while ((bytesRecv = read(sockfd, buf, bufSize)) > 0) {
      //std::cout << buf;
      //std::cout << "BytesRecv: " << bytesRecv << endl;
      ss << buf;
      for (int i = 0; i < bytesRecv; i++) {
        msg.push_back(buf[i]);
        count++;
      }

      foundHeaderEnd = ss.str().find("\r\n\r\n");
      //std::cout << buf << "    " << to_string(foundHeaderEnd) << std::endl;
      if (foundHeaderEnd != string::npos) {
        if (createdResponse == 0) {
          response->responseToObject(msg);
          //vector<char> literal = response->buildResponse();
          //for(int i = 0; i < literal.size(); i++){
          //  cout<<literal[i];
         // }
          map<string,string> headers = response->getHeaders();
          map<string,string>::iterator it = headers.find("Content-Length");
          string reqHostname;
          if (it != headers.end()) {
            std::string::size_type sz;
            payloadLen = stoi(headers["Content-Length"]);
            //cout << "Content Length: " << payloadLen << endl;
          }
          createdResponse = 1;
        }
        if ((bytesRecv < bufSize) && (count == payloadLen)) {
          //std::cout << "BREAKING OUT\n";
          break;
        }
      }

      memset(buf, '\0', sizeof(buf));
    }


    //std::cout << endl << "finished recving\n\n";
    // End of while loop
    if (bytesRecv == -1) {
      perror("recv");
      return 5;
    }

    int headerLen = (count-payloadLen);
    string msgStr(msg.begin(), msg.begin()+headerLen);
    //cout << msgStr << endl;
    
    string strOut(msg.begin(), msg.end());
    //cout << strOut;


    // TODO: need to get the filename from url? or somewhere else
    string fileurl = request.getUrl();
    std::size_t lastBackslash = fileurl.find_last_of("/");
    string filename = fileurl.substr(lastBackslash);
    if (filename == "/")
      filename = "/index.html";
    filename.erase(0,1);

    string responseStatus = response->getStatus();
    std::size_t okStatus = responseStatus.find("200");

    // If status code = 200 Ok, open the file and write to it
    if (okStatus != std::string::npos) {
      //TODO: for some reason, its not writing to the file when doing:
      // ./web-client http://google.com/
      // it gets the response, just doesn't write to the file
      //ofstream file;
      //file.open(filename);
      ofstream file(filename, std::ios::out | std::ofstream::binary);
      std::copy(msg.begin()+headerLen, msg.end(), std::ostreambuf_iterator<char>(file));
      //file << strOut;
      //file.close();
    }
    else{
      cerr<<responseStatus<<endl;
    }
    /* Testing finding ok status code
    else {
      ofstream errf;
      errf.open("error");
      errf << "404 or 400";
      errf.close();
    }
    */

    free(response);

  close(sockfd);

  return 0;
}
