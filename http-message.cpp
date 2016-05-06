//
//  main.cpp
//  httpmessage
//
//  Created by Max Chern on 4/21/16.
//  Copyright © 2016 Max Chern. All rights reserved.
//
#include <iostream>
using namespace std;

#include <string>
#include <map>
#include <set>
#include <vector>

//==========================================//
//============HttpMessage===================//
//==========================================//

class HttpMessage {
public:
    HttpMessage();
    
    int getVersion();
    void setVersion(int version);
    
    bool isValid();
    void invalidate();
    
    map<string, string> getHeaders();
    void setHeader(string key, string value);
    
    //string getPayload();
    vector<char> getPayload();
    //void setPayload(string payload);
    void setPayload(vector<char> payload);
    
    
private:
    int m_version; //0 -> 1.0, 1 -> 1.1, 2 -> 2.0
    map<string, string> m_headers;
    //string m_payload;
    vector<char> m_payload;
    bool m_valid;
};


HttpMessage::HttpMessage() {
    m_version = 0;
    //m_payload = "";
    m_valid = true;
    
    
    
}

int HttpMessage::getVersion() {
    return m_version;
}

void HttpMessage::setVersion(int version) {
    m_version = version;
}

bool HttpMessage::isValid() {
    return m_valid;
}

void HttpMessage::invalidate() {
    m_valid = false;
}

map<string, string> HttpMessage::getHeaders() {
    return m_headers;
}

void HttpMessage::setHeader(string key, string value) {
    m_headers.insert(pair<string, string>(key, value));
}

//string HttpMessage::getPayload() {
vector<char> HttpMessage::getPayload() {
    return m_payload;
}

//void HttpMessage::setPayload(string payload) {
void HttpMessage::setPayload(vector<char> payload) {
    m_payload = payload;
}






//==========================================//
//============HttpRequest===================//
//==========================================//

class HttpRequest : public HttpMessage {
public:
    HttpRequest();
    
    //CLIENT sends a request
    void urlToObject(string url);
    
    //SERVER receives a request
    //void messageToObject(string message);
    void messageToObject(vector<char> message);
    
    string getMethod();
    void setMethod(string method);
    
    string getUrl();
    void setUrl(string url);
    
    int getPort();
    void setPort(int port);
    
    //string buildRequest();
    vector<char> buildRequest();
    
private:
    string m_method;
    string m_url;
    int m_port;
};



HttpRequest::HttpRequest() {
    m_method = "";
    m_url = "";
    m_port = 0;
}

void HttpRequest::urlToObject(string url) {
    m_method = "GET";
    
    //splitting up url into HOST/URL
    string host = "";
    int i = 0;
    int url_size = url.size();
    
    //iterate past http(s)://
    while (url[i] != '/') {
        i++;
    }
    i += 2;
    
    //store www... into host string
    while (i < url.size() && url[i] != '/' && url[i] != ':') {
        host += url[i];
        i++;
    }
    
    //either consumed whole URL, reached '/', or reached ':'
    
    //if consumed whole URL, then default port 80, default url '/', default version 1.0, default Headers
    //return
    if (i == url.size()) {
        m_port = 80;
        m_url = "/";
        setVersion(0);
        setHeader("Host", host);
        return;
    }
    
    //if a port number is specified...
    if (url[i] == ':') {
        i++;
        
        string portnum;
        
        while (i < url.size() && url[i] != '/') {
            portnum += url[i];
            i++;
        }
        
        m_port = stoi(portnum);
        
        //either consumed whole URL or reached '/'
        
        //if consumed whole URL, then default url '/', default version 1.0, default Headers
        //return
        if (i == url.size()) {
            m_url = "/";
            setVersion(0);
            setHeader("Host", host);
            return;
        }
        
    } else { //else default to 80
        m_port = 80;
    }
    
    
    //store /... into m_url
    while (i < url_size) {
        m_url += url[i];
        i++;
    }
    
    //std::size_t lastBackslash = url.find_last_of("/");
    //m_url = url.substr(lastBackslash);
    
    //default version is 1.0
    setVersion(0);
    setHeader("Host", host);
    //    setPayload("");
    
}

//void HttpRequest::messageToObject(string message) {
void HttpRequest::messageToObject(vector<char> message) {
    
    int i = 0;
    int msg_size = message.size();
    
    if (i >= message.size()) {
        invalidate();
        return;
    }
    
    //method
    while (message[i] != ' ') {
        m_method += message[i];
        i++;
        if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
            invalidate();
            return;
        }
    }
    while (message[i] == ' ') {
        i++;
        if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
            invalidate();
            return;
        }
    }

    string check_method = "";
    for (int k = 0; k < m_method.size(); k++) {
      check_method += tolower(m_method[k]);
    }
    if (check_method != "get") {
      invalidate();
      return;
    }
    
    //url
    while (message[i] != ' ') {
        m_url += message[i];
        i++;
        if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
            invalidate();
            return;
        }
    }
    while (message[i] == ' ') {
        i++;
        if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
            invalidate();
            return;
        }
    }
    
    //skip HTTP/, then get version number
    while (message[i] != '/') {
        i++;
        if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
            invalidate();
            return;
        }
    }
    i++;
    if (i >= message.size() || !isdigit(message[i])) {
        invalidate();
        return;
    }
    
    
    if (message[i] == '1') {
        if ((i+2) >= message.size() || !isdigit(message[i+2])) {
            invalidate();
            return;
        }
        if (message[i+2] == '0') {
            setVersion(0);
        } else if (message[i+2] == '1') {
            setVersion(1);
        } else {
	  invalidate();
	  return;
	}
    } else if (message[i] == '2') {
      if ((i+2) >= message.size() || !isdigit(message[i+2])) {
	invalidate();
	return;
      }
      if (message[i+2] == '0') {
        setVersion(2);
      } else {
	invalidate();
	return;
      }
    } else {
      invalidate();
      return;
    }
    
    //skip to header lines
    while (message[i] != '\n') {
        i++;
        if (i >= message.size()) {
            invalidate();
            return;
        }
    }
    i++;
    if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
        invalidate();
        return;
    }
    
    //headers
    while (message[i] != '\r') {
        
        string key;
        string value;
        
        while (message[i] != ':') {
            key += message[i];
            i++;
            if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
                invalidate();
                return;
            }
        }
        i += 2;
        
        while (message[i] == ' ') {
            i++;
            if (i >= message.size() || message[i] == '\r' || message[i] == '\n') {
                invalidate();
                return;
            }
        }
        
        while (message[i] != '\r') {
            value += message[i];
            i++;
            if (i >= message.size() || message[i] == '\n') {
                invalidate();
                return;
            }
        }
        i += 2;
        if (i >= message.size()) {
            invalidate();
            return;
        }
        
        setHeader(key, value);
        
    }
    i++;
    if (i >= message.size() || message[i] != '\n') {
        invalidate();
        return;
    }
    i++;
    
    //payload (Optional message body)
    //string payload = "";
    vector<char> payload;
    while (i < msg_size) {
        //payload += message[i];
        payload.push_back(message[i]);
        i++;
    }
    setPayload(payload);
}

string HttpRequest::getMethod() {
    return m_method;
}

void HttpRequest::setMethod(string method) {
    m_method = method;
}

string HttpRequest::getUrl() {
    return m_url;
}

void HttpRequest::setUrl(string url) {
    m_url = url;
}

int HttpRequest::getPort() {
    return m_port;
}

void HttpRequest::setPort(int port) {
    m_port = port;
}

//string HttpRequest::buildRequest() {
vector<char> HttpRequest::buildRequest() {
    string request = getMethod() + " " + getUrl() + " HTTP/";
    switch(getVersion()) {
        case 1:
            request += "1.1\r\n";
            break;
        case 2:
            request += "2.0\r\n";
            break;
        default:
            request += "1.\r\n";
    }
    
    map<string, string>headers = getHeaders();
    map<string, string>::iterator it;
    for (it = headers.begin(); it != headers.end(); it++) {
        request += it->first + ": " + it->second + "\r\n";
    }
    request += "\r\n";
    //   request += getPayload();
    
    vector<char> requestVec(request.begin(), request.end());
    vector<char> pl = getPayload();
    requestVec.insert(requestVec.end(), pl.begin(), pl.end());
    
    return requestVec;
}









//==========================================//
//============HttpResponse==================//
//==========================================//

class HttpResponse : public HttpMessage {
public:
    HttpResponse();
    
    //void responseToObject(string response);
    void responseToObject(vector<char> response);
    
    //string buildResponse();
    vector<char> buildResponse();
    
    string getStatus();
    void setStatus(string status);
    
    
private:
    string m_status;
};


HttpResponse::HttpResponse() {
    m_status = "";
}


string HttpResponse::getStatus() {
    return m_status;
}

void HttpResponse::setStatus(string status) {
    m_status = status;
}

//edit
//string HttpResponse::buildResponse() {
vector<char> HttpResponse::buildResponse() {
    string response = "HTTP/";
    switch(getVersion()) {
        case 1:
            response += "1.1 ";
            break;
        case 2:
            response += "2.0 ";
            break;
        default:
            response += "1.0 ";
    }
    response += m_status + "\r\n";
    m_status += "\r\n";
    
    map<string, string>headers = getHeaders();
    map<string, string>::iterator it;
    for (it = headers.begin(); it != headers.end(); it++) {
        response += it->first + ": " + it->second + "\r\n";
    }
    response += "\r\n";
    //response += getPayload();
    vector<char> responseVec(response.begin(), response.end());
    vector<char> pl = getPayload();
    responseVec.insert(responseVec.end(), pl.begin(), pl.end());
    
    return responseVec;
}

//void HttpResponse::responseToObject(string response) {
void HttpResponse::responseToObject(vector<char> response) {
    
    int i = 0;
    int response_size = response.size();
    
    //skip HTTP/, then get version number
    while (response[i] != '/') {
        i++;
    }
    i++;
    
    if (response[i] == '1') {
        if (response[i+2] == '0') {
            setVersion(0);
        } else {
            setVersion(1);
        }
    } else {
        setVersion(2);
    }
    
    //skip to status
    while (response[i] != ' ') {
        i++;
    }
    while (response[i] == ' ') {
        i++;
    }
    
    //status
    while (response[i] != '\r') {
        m_status += response[i];
        i++;
    }
    i += 2;
    
    
    //headers
    while (response[i] != '\r') {
        
        string key;
        string value;
        
        while (response[i] != ':') {
            key += response[i];
            i++;
        }
        i += 2;
        
        while (response[i] == ' ') {
            i++;
        }
        
        while (response[i] != '\r') {
            value += response[i];
            i++;
        }
        i += 2;
        
        setHeader(key, value);
        
    }
    i += 2;
    
    //payload (data)
    //string payload = "";
    vector<char> payload;
    while (i < response_size) {
        //payload += response[i];
        payload.push_back(response[i]);
        i++;
    }
    setPayload(payload);
}


















void printHeaders(map<string, string> headers) {
    
    map<string, string>::iterator it = headers.begin();
    while (it != headers.end()) {
        cout << it->first << ": " << it->second << endl;
        it++;
    }
}


/*
 
 int main() {
 HttpRequest test3;
 string s = "GET /index.html HTTP/1.1\r\nHost: www-net.cs.umass.edu\r\nUser-Agent: Firefox/3.6.10\r\nAccept: text/html,application/xhtml+xml\r\nAccept-Language: en-us,en;q=0.5\r\nAccept-Encoding: gzip,deflate\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7\r\nKeep-Alive: 115\r\nConnection: keep-alive\r\n\r\nhello";
 vector<char> test(s.begin(), s.end());
 test3.messageToObject(test);
 vector<char> testvector = test3.buildRequest();
 string s2(testvector.begin(), testvector.end());
 cout << s2 << endl;
 if (test3.isValid()) {
 cout << "It is valid." << endl;
 } else {
 cout << "It is invalid." << endl;
 }
 
 HttpResponse test4;
 string s3 = "HTTP/1.1 200 OK\r\nDate: Sun, 26 Sep 2010 20:09:20 GMT\r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified: Tue, 30 Oct 2007 17:00:02 GMT\r\nETag: \"17dc6-a5c-bf716880\"\r\nAccept-Ranges: bytes\r\nContent-Length: 2652\r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=ISO-8859-1\r\n\r\ndatadatadatadatadatalol";
 vector<char> testing(s3.begin(), s3.end());
 test4.responseToObject(testing);
 vector<char> testvector2 = test4.buildResponse();
 string s8(testvector2.begin(), testvector2.end());
 cout << s8 << endl;
 if (test4.isValid()) {
 cout << "It is valid." << endl;
 } else {
 cout << "It is invalid." << endl;
 }
 
 }
 
 
 */
