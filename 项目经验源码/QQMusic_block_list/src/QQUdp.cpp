#include "QQUdp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include "SVPLog.h"

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG  "QQUdp"

#define MAX_BUFFER_SIZE 50*1024

CQQUdp::CQQUdp(QQUdpReceiveDataCB cbReceiveData, int bindPort)
: m_cbReceiveData(cbReceiveData)
, m_port(bindPort)
{
	m_pBuffer = new char[MAX_BUFFER_SIZE];
	pthread_t m_tid;
	pthread_create(&m_tid, NULL, ThreadReceiveData, (void*)this);
}

CQQUdp::~CQQUdp()
{
	delete m_pBuffer;
	close(m_socket);
	m_socket = -1;
}

void* CQQUdp::ThreadReceiveData(void *arg)
{
	pthread_detach(pthread_self());
	CQQUdp* pthis = (CQQUdp*)arg;
	pthis->_InitConnect();
	return 0;
}


void CQQUdp::_InitConnect()
{
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	memset(&local_addr, 0, sizeof (local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(m_port);
	if ((m_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		SVP_ERROR("Create Udp %d Socket Failed: %s", m_port,strerror(errno));
		return;
	}
	if (-1 == bind(m_socket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))
	{
		SVP_ERROR("Bind Udp Socket Port %d, Failed: %s", m_port, strerror(errno));
		return;
	}
	socklen_t sin_size = sizeof (struct sockaddr_in);
	while(true)
	{
		memset(m_pBuffer, 0, MAX_BUFFER_SIZE);
		int len = recvfrom(m_socket, m_pBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&remote_addr, &sin_size);
		if (len > 0 && m_cbReceiveData != NULL)
		{
			m_cbReceiveData(m_pBuffer, len, m_port);
		}
		SVP_INFO("recvfrom %s data %s", inet_ntoa(remote_addr.sin_addr), m_pBuffer);
	}
	close(m_socket);
}


void UdpBroadcast(const std::string& _addr, int port, const std::string& data)
{
	if (port == 0 || _addr.empty())
		return;
	int udpsock;
	struct sockaddr_in remote_addr;
	memset (&remote_addr, 0, sizeof (remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(_addr.data());
	remote_addr.sin_port = htons(port);
	if ((udpsock = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		SVP_ERROR("UdpBroadcast socket %s:%d, Failed: %s", _addr.data(), port, strerror(errno));
		return;
	}
	int on = 1;
	setsockopt(udpsock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	int len = sendto(udpsock, data.data(), data.length(), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr));
	if (len < 0)
	{
		SVP_ERROR("UdpBroadcast sendto %s:%d, Failed: %s", _addr.data(), port, strerror(errno));
	}
//	SVP_INFO("sendto %s data %s", _addr.data(), data.data());
	close(udpsock);
}


std::string GetLocalMacAddr(const std::string& devicename)
{
    struct ifreq ifr_mac;
    int mac_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(mac_sock == -1)
    {
    	SVP_ERROR("GetLocalMacAddr socket Failed: %s", strerror(errno));
        return "";
    }
    memset(&ifr_mac, 0, sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, devicename.c_str(), sizeof(ifr_mac.ifr_name)-1);
    if( (ioctl(mac_sock, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
    	SVP_ERROR("GetLocalMacAddr ioctl Failed: %s", strerror(errno));
        close(mac_sock);
        return "";
    }
    char buffer[30];
    sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
    close(mac_sock);
    return buffer;
}

std::string GetLocalIpV4Addr(const std::string& devicename)
{
    int if_count, i;
    struct ifconf ifc;
    struct ifreq ifr[10];
    char ipaddr[INET_ADDRSTRLEN] = {'\0'};
    memset(&ifc, 0, sizeof(struct ifconf));
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (char *)ifr;

    /* SIOCGIFCONF is IP specific. see netdevice(7) */
    ioctl(sfd, SIOCGIFCONF, (char *)&ifc);
    if_count = ifc.ifc_len / (sizeof(struct ifreq));
    for (i = 0; i < if_count; i++) {
    	if (devicename == ifr[i].ifr_name)
    	{
    		inet_ntop(AF_INET, &(((struct sockaddr_in *)&(ifr[i].ifr_addr))->sin_addr), ipaddr, INET_ADDRSTRLEN);
    	}
    }
    close(sfd);
    return ipaddr;
}
