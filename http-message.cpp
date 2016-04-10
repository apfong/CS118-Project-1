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
  void decodeHeaderLine(Byteblob line);

  int getVersion() { return m_version; }

  string getHeaders() { return m_headers; }
  void setHeader(string key, string value) {
    m_headers.insert(std::pair<string,string>(key, value));
  }

  ByteBlob getPayload() { return m_payload }
  void setPayLoad(ByteBlob blob) { m_payload = blob; }

  

private:
  int m_version;
  map<string, string> m_headers;  //client: host
                                  //server: date, server, content type, content length
  ByteBlob m_payload;
};

class HttpRequest : HttpMessage {
public:
  HttpRequest() {}

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

  virtual vecotr<uint_8> encode(ByteBlob line) {}
  virtual HttpMessage decode(ByteBlob line) {}

private:
  string m_method;
  string m_url;
};

class HttpResponse : HttpMessage {
public:
  int getStatus() { return m_status; }
  void setStatus(int status) { m_status = status; }

  string getDescription() { return m_statusDescription; }
  void setDescription(string description) { 
    m_statusDescription = description;
  }

  virtual vecotr<uint_8> encode(ByteBlob line) {}
  virtual HttpMessage decode(ByteBlob line) {}

private:
  int m_status;
  string m_statusDescription; 
};
