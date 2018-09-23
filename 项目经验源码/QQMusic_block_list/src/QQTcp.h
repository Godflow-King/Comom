#ifndef __QQTCP_H__
#define __QQTCP_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

typedef void (*QQTcpReceiveDataCB)(char* pData, int len);
typedef void (*QQTcpConnectStateCB)(const std::string& ip, bool bConnected);

class CQQTcp
{
public:
	CQQTcp(QQTcpReceiveDataCB cbReceiveData, QQTcpConnectStateCB cbConnectState, int bindPort);
    ~CQQTcp();
    void Disconnect();
private:
    static void* ThreadReceiveData(void *arg);
    void _InitConnect();
    bool _ReceiveData();
    void SetConnectState(const std::string& ip, bool bConnected);
private:
    QQTcpReceiveDataCB m_cbReceiveData;
    QQTcpConnectStateCB m_cbConnectState;
    bool m_bConnected;
    int m_port;
    int m_socket;
    int m_fd;
    char* m_pBuffer;
};

#endif
