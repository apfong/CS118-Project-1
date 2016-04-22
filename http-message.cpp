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
#include <vector>

//==========================================//
//============HttpMessage===================//
//==========================================//

class HttpMessage {
public:
    HttpMessage();
    
    int getVersion();
    void setVersion(int version);
    
    map<string, string> getHeaders();
    void setHeader(string key, string value);
    
    string getPayload();
    void setPayload(string payload);
    
private:
    int m_version; //0 -> 1.0, 1 -> 1.1, 2 -> 2.0
    map<string, string> m_headers;
    string m_payload;
};


HttpMessage::HttpMessage() {
    m_version = 0;
    m_payload = "";
}

int HttpMessage::getVersion() {
    return m_version;
}

void HttpMessage::setVersion(int version) {
    m_version = version;
}

map<string, string> HttpMessage::getHeaders() {
    return m_headers;
}

void HttpMessage::setHeader(string key, string value) {
    m_headers.insert(pair<string, string>(key, value));
}

string HttpMessage::getPayload() {
    return m_payload;
}

void HttpMessage::setPayload(string payload) {
    m_payload = payload;
}


//==========================================//
//============HttpRequest===================//
//==========================================//

class HttpRequest : public HttpMessage {
public:
    HttpRequest();
    
    //CLIENT sends a request
    HttpRequest(string url);
    
    string getMethod();
    void setMethod(string method);
    
    string getUrl();
    void setUrl(string url);
    
    int getPort();
    void setPort(int port);
    
    string buildRequest();
    
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

HttpRequest::HttpRequest(string url) {
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
    while (url[i] != '/' && url[i] != ':') {
        host += url[i];
        i++;
    }
    
    //if a port number is specified...
    if (url[i] == ':') {
        i++;
        
        string portnum;
        
        while (url[i] != '/') {
            portnum += url[i];
            i++;
        }
        
        m_port = stoi(portnum);
        
    } else { //else default to 80
        m_port = 80;
    }
    
    //store /... into m_url
    while (i < url_size) {
        m_url += url[i];
        i++;
    }
    
    //default version is 1.0
    setVersion(0);
    setHeader("Host", host);
    setPayload("");
    
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

string HttpRequest::buildRequest() {
    string request = getMethod() + " " + getUrl() + " HTTP/";
    switch(getVersion()) {
        case 1:
            request += "1.1\r\n";
            break;
        case 2:
            request += "2.0\r\n";
            break;
        default:
            request += "1.0\r\n";
    }
    
    map<string, string>headers = getHeaders();
    map<string, string>::iterator it;
    for (it = headers.begin(); it != headers.end(); it++) {
        request += it->first + ": " + it->second + "\r\n";
    }
    request += "\r\n";
    request += getPayload();
    
    return request;
}



















void printHeaders(map<string, string> headers) {
    
    map<string, string>::iterator it = headers.begin();
    while (it != headers.end()) {
        cout << it->first << ": " << it->second << endl;
        it++;
    }
}



int main() {
    /*
     HttpRequest test;
     
     cout << test.getVersion() << endl;
     test.setVersion(5);
     cout << test.getVersion() << endl;
     test.setVersion(1);
     cout << test.getVersion() << endl;
     
     
     printHeaders(test.getHeaders());
     test.setHeader("Host", "hello");
     printHeaders(test.getHeaders());
     test.setHeader("Test", "bye");
     printHeaders(test.getHeaders());
     
     cout << test.getPayload() << endl;
     test.setPayload("TestPayload");
     cout << test.getPayload() << endl;
     
     cout << test.getMethod() << endl;
     test.setMethod("MethodTest");
     cout << test.getMethod() << endl;
     
     cout << test.getUrl() << endl;
     test.setUrl("TestUrl");
     cout << test.getUrl() << endl;
     
     cout << test.getPort() << endl;
     test.setPort(80);
     cout << test.getPort() << endl;
     */
    
    HttpRequest facebook("http://www.facebook.com/index.html");
    
    /*
     cout << "url: " << facebook.getUrl() << endl;
     cout << "method: " << facebook.getMethod() << endl;
     cout << "port: " << facebook.getPort() << endl;
     
     cout << "version: " << facebook.getVersion() << endl;
     cout << "payload: " << facebook.getPayload() << endl;
     
     printHeaders(facebook.getHeaders());
     */
    
    
    cout << facebook.buildRequest() << endl;
    
}

