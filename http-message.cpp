#include <map>
#include <vector>

class HttpMessage {
public:
  virtual void decodeFirstLine(ByteBlob line) = 0;
  int getVersion();
  void setHeader(string key, string value);
  string getHeader(string key);
  void decodeHeaderLine(Byteblob line);
  void setPayLoad(ByteBlob blob);
  ByteBlob getPayload();

private:
  int m_version;
  map<string, string> m_headers;
  ByteBlob
};

class HttpRequest : HttpMessage {
public:
  HttpRequest() {}

  HttpMethod getMethod();
  string getUrl();

  void setMethod(HttpMethod method){}
  void setUrl(string url){}

  vector<uint_8> encode(){}
  void consume(){}

private:
  string m_method;
  string m_url;
};

class HttpResponse : HttpMessage {
public:
  virtual void decodeFirstLine(ByteBlob line);
  HttpStatus getStatus();
  void setStatus(HttpStatus status);
  string getDescription();
  void setDescription(string description);

private:
  HttpStatus m_status;
  string m_statusDescription;
};
