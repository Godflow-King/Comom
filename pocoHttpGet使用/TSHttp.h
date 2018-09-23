#ifndef __TS_HTTP_H__
#define __TS_HTTP_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <list>
#include <mutex>

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

struct HttpRequestInfo
{
    std::string url;
    int iParam;
    std::string strParam;
    void* responseCB;
};

typedef void (*HttpResponseCB)(bool result, std::string& content, HttpRequestInfo& requestInfo);

class CTSHttp
{
public:
	static CTSHttp* GetInstance();
	void SendRequest(HttpRequestInfo& info);
protected:
	CTSHttp();
    ~CTSHttp();
private:
    static void* RequestThread(void *arg);
    bool GetRequestItem(HttpRequestInfo& info);
private:
    static CTSHttp* m_pInstance;
    std::list<HttpRequestInfo> m_requestList;
    std::recursive_mutex m_mtxList;
};

#endif
