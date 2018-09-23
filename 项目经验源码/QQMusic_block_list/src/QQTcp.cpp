#include "QQTcp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include "SVPLog.h"

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG  "QQTcp"

#define LENGTH_OF_LISTEN_QUEUE 20
#define MAX_BUFFER_SIZE 80*1024

CQQTcp::CQQTcp(QQTcpReceiveDataCB cbReceiveData, QQTcpConnectStateCB cbConnectState, int bindPort)
: m_cbReceiveData(cbReceiveData)
, m_cbConnectState(cbConnectState)
, m_port(bindPort)
, m_bConnected(false)
{
	m_pBuffer = new char[MAX_BUFFER_SIZE];
	pthread_t m_tid;
	pthread_create(&m_tid, NULL, ThreadReceiveData, (void*)this);
}

CQQTcp::~CQQTcp()
{
	delete m_pBuffer;
	close(m_fd);
	close(m_socket);
	m_socket = -1;
}


void* CQQTcp::ThreadReceiveData(void *arg)
{
	pthread_detach(pthread_self());
	CQQTcp* pthis = (CQQTcp*)arg;
	pthis->_InitConnect();
	return 0;
}

void CQQTcp::_InitConnect()
{
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	socklen_t  sin_size = sizeof(struct sockaddr_in);
	memset (&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(m_port);
	if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		SVP_ERROR("Create Tcp Socket Failed: %s", strerror(errno));
		return;
	}
	int on = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (-1 == bind(m_socket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))
	{
		SVP_ERROR("Bind Tcp Socket Port %d, Failed: %s", m_port, strerror(errno));
		return;
	}
	if (-1 == listen(m_socket, LENGTH_OF_LISTEN_QUEUE))
	{
		SVP_ERROR("Listen Tcp Socket Port %d, Failed: %s", m_port, strerror(errno));
		return;
	}
	while(m_socket > 0)
	{
		if ((m_fd = accept (m_socket, (struct sockaddr *) &remote_addr, &sin_size)) < 0)
		{
			SVP_ERROR("Accept Tcp Socket Port %d, Failed: %s", m_port, strerror(errno));
			return;
		}
		std::string remote_ip = inet_ntoa(remote_addr.sin_addr);
		if (remote_ip == "0.0.0.0")
		{
			close(m_fd);
			continue;
		}
		SetConnectState(remote_ip, true);
		while (_ReceiveData());
		SVP_ERROR("Disconnect, Failed: %s", strerror(errno));
		SetConnectState(remote_ip, false);
		close(m_fd);
		m_fd = -1;
	}
}


void CQQTcp::SetConnectState(const std::string& ip, bool bConnected)
{
	m_bConnected = bConnected;
	if (m_cbConnectState != NULL)
	{
		m_cbConnectState(ip, bConnected);
	}
}

bool CQQTcp::_ReceiveData()
{
	memset(m_pBuffer, 0, MAX_BUFFER_SIZE);
	int len = read(m_fd, m_pBuffer, MAX_BUFFER_SIZE);
	if (len > 0 && m_cbReceiveData != NULL)
	{
		m_cbReceiveData(m_pBuffer, len);
	}
	return len > 0;
}

void CQQTcp::Disconnect()
{
	if (m_fd >= 0)
	{
		shutdown(m_fd, SHUT_RDWR);
	}
}

