#include <string>
#include <map>
#include <vector>

/*
  Request line
  Headers

  Body (only in some messages)
*/
class HttpMessage {
public:
  void decodeHeaderLine(Byteblob line);

  int getVersion() { return m_version; }

  string getHeader(string key) { return m_headers[key]; }
  void setHeader(string key, string value) {
    m_headers.insert(std::pair<string,string>(key, value));
  }

  ByteBlob getPayload() { return m_payload }
  void setPayLoad(ByteBlob blob) { m_payload = blob; }

  virtual vector<uint_8> encode(ByteBlob line) = 0;
  virtual HttpMessage decode(ByteBlob line) = 0;

private:
  int m_version;
  map<string, string> m_headers;
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
