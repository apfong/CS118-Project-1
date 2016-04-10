#include <string>
#include <map>
#include <vector>

/*
  Request line
  Headers

  Body (only in some messages)
*/

vector<uint_8> encode(string message){
  vector<uint_8>encmsg;
  for(int i =0; i< message.size(); i++){
    encmsg.pushback((uint_8)message[i]);
  }
  return encmsg;
}
string HttpMessage decode(vector<uint_8> encmsg){
  string decoded = "";
  for(int i = 0; i<encmsg.size(); i++){
    decoded += (char)encmsg[i];
  }
  return decoded;
}
  
class HttpMessage {
public:
  void decodeHeaderLine(ByteBlob line);

  int getVersion() { return m_version; }

  string getHeaders() { return m_headers; }
  void setHeader(string key, string value) {
    m_headers.insert(std::pair<string,string>(key, value));
  }

  vector<uint_8> getPayload() { return m_payload }
  void setPayLoad(vector<uint_8> payload) { m_payload = payload; }

  
private:
  int m_version;                  // 0,1,2 --> 1.0, 1.1, 2.0
  map<string, string> m_headers;  //client: host
                                  //server: date, server, content type, content length
  vector<uint_8> m_payload;
};

class HttpRequest : HttpMessage {
public:

  HttpRequest() {}

  // constructor for when the client sends a request
  //*** assuming format is http(s)://www. ...
  HttpRequest(string url) {

    m_method = "GET";
    
    //splitting up url into HOST/URL
    string host = "";
    int i = 0;
    int url_size = url.size();

    //skip http(s)://
    while (url[i] != '/') {
      i++;
    }
    i += 2;

    //store www... into host string
    while (url[i] != '/') {
      host += url[i];
    }

    //store /... into m_url
    while (i < url_size) {
      m_url += url[i];
    }

    //default version is 1.0
    m_version = 1.0;

    m_headers.insert(std::pair<string,string>("Host", host));

  }

  //constructor for when the server receives a request
  HttpRequest(vector<uint_8> request_message) {

    string http_msg = decode(request_message);

    int i = 0;
    int msg_size = request_message.size();

    //method
    while (http_msg[i] != ' ') {
      m_method += http_msg[i];
    }
    i++;

    //url
    while (http_msg[i] != ' ') {
      m_url += http_msg[i];
    }
    i++;

    //skip HTTP/, then get version number
    while (http_msg [i] != '/') {
      i++;
    }
    i++;
    if (http_msg[i] == '1') {
      if (http_msg[i+2] == '0') {
	m_version = 0;
      } else {
	m_version = 1;
      }
    } else {
      m_version = 2;
    }
    
    //skip to header lines
    while (http_msg[i] != '\n') {
      i++;
    }
    i++;

    //headers
    while (http_msg[i] != '\r') {

      string key;
      string value;

      while (http_msg[i] != ':') {
	key += http_msg[i];
      }
      i += 2;

      while (http_msg[i] != '\r') {
	value += http_msg[i];
      }
      i += 2;

      m_headers.insert(std::pair<string,string>(key, value));

    }
    i += 2;

    //payload (Optional message body) - read from request_message vector
    while (i < msg_size) {
      m_payload.pushback(request_message[i]);
    }

  }

  string getMethod() { return m_method; }
  void setMethod(string method) { m_method = method; }

  string getUrl() { return m_url; }
  void setUrl(string url) { m_url = url; }

  vector<uint_8> encode(){}
  void consume(){}

  string buildRequest(){
    string request = getMethod() + " " + getURL() + " HTTP/"+getVersion()+"\r\n";
    map<string,string>headers = getHeaders();
    map<string,string>::iterator it;
    for(it = headers.begin(); it != headers.end(); it++){
      request += it->first+": "+it->second+"\r\n";
    }
    request += "\r\n";
    return request;

  }


private:
  string m_method;
  string m_url;
};

class HttpResponse : HttpMessage {
public:
  
  HttpResponse() {}

  int getStatus() { return m_status; }
  void setStatus(int status) { m_status = status; }

  string getDescription() { return m_statusDescription; }
  void setDescription(string description) { 
    m_statusDescription = description;
  }


private:
  int m_status;
  string m_statusDescription; 
};
