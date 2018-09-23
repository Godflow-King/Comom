#ifndef __QQUDP_H__
#define __QQUDP_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

typedef void (*QQUdpReceiveDataCB)(char* pData, int len, int port);

class CQQUdp
{
public:
	CQQUdp(QQUdpReceiveDataCB cbReceiveData, int bindPort);
    ~CQQUdp();
private:
    static void* ThreadReceiveData(void *arg);
    void _InitConnect();
private:
    QQUdpReceiveDataCB m_cbReceiveData;
    int m_port;
    int m_socket;
    char* m_pBuffer;
};

std::string GetLocalMacAddr(const std::string& devicename);
std::string GetLocalIpV4Addr(const std::string& devicename);
void UdpBroadcast(const std::string& _addr, int port, const std::string& data);

#endif
