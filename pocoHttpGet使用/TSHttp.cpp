#include "TSHttp.h"

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG  "TSHttp"

#include "config.h"
#include <Poco/Net/HTTPCredentials.h>
#include "Poco/StreamCopier.h"
#include "Poco/NullStream.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include <sstream>

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;

CTSHttp* CTSHttp::m_pInstance = NULL;
#define MAX_HTTP_THREAD_SIZE 3
CTSHttp::CTSHttp()
{
	pthread_t m_tid;
	for (int i = 0; i < MAX_HTTP_THREAD_SIZE; i++)
	{
		pthread_create(&m_tid, NULL, RequestThread, (void*)this);
	}
}

CTSHttp::~CTSHttp()
{

}

CTSHttp* CTSHttp::GetInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new CTSHttp();
    }
    return( m_pInstance );
}

void* CTSHttp::RequestThread(void *arg)
{
	pthread_detach(pthread_self());
	CTSHttp* pthis = (CTSHttp*) arg;
	HttpRequestInfo info;
	while (true)
	{
		if (!pthis->GetRequestItem(info))
		{
			usleep(25*1000);
			continue;
		}
		//执行请求动作
		bool result = false;
		std::string content = "";
		try
		{
			URI uri(info.url);
			std::string path(uri.getPathAndQuery());
			if (path.empty()) path = "/";

			HTTPClientSession session(uri.getHost(), uri.getPort());
			HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
			HTTPResponse response;
			std::ostringstream m_os;
			session.sendRequest(request);
			std::istream& rs = session.receiveResponse(response);
			if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
			{
				StreamCopier::copyStream(rs,  m_os );
				result = true;
				content = m_os.str();
			}
			else
			{
				SVP_INFO(" receiveResponse Fail  ");
			}
		}
		catch (Exception& exc)
		{
			SVP_INFO( "%s",exc.displayText().c_str() );
		}
		//结果返回
		if (info.responseCB != NULL)
		{
			HttpResponseCB cbfunc = (HttpResponseCB) info.responseCB;
			cbfunc(result, content, info);
		}
	}
}

void CTSHttp::SendRequest(HttpRequestInfo& info)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	m_requestList.push_back(info);
}

bool CTSHttp::GetRequestItem(HttpRequestInfo& info)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	if (m_requestList.empty())
	{
		return false;
	}
	info = *m_requestList.begin();
	m_requestList.pop_front();
	return true;
}
