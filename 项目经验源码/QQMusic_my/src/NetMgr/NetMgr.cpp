#include "NetMgr.h"
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define TCPBUTSIZE         		1024*1024
#define UDPBUTSIZE         		1024*50

NetMgr::NetMgr():listener_(NULL),m_InitError(0),m_TCPBuffer(NULL)
{
	cmd_socket = -1;
	result_socket = -1;
	pmc_socket = -1;
}

NetMgr::~NetMgr()
{
	ExitAllTread();
	SVP_INFO(" ~NetMgr ");
}

static int CheckError()
{
	extern int errno;
	char * mesg = strerror(errno);
	SVP_ERROR("CheckError :%s",mesg);
	return errno;
}

int NetMgr::InitUdpToRec(int &sok,int &Port,int iscmd )
{
	int rsize;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	socklen_t sin_size = sizeof (struct sockaddr_in);
	memset (&my_addr, 0, sizeof (my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(Port);
	char udpbuf[UDPBUTSIZE];
	if ((sok = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		SVP_ERROR("socket error");
		return 1;
	}
	int ReRetryCount = 0;
	while (bind (sok, (struct sockaddr *) &my_addr, sizeof (struct sockaddr)) < 0)
	{
		SVP_ERROR ("bind ERROR");
//		usleep(400000);
//		my_addr.sin_port = htons( ++Port );
//		if( ReRetryCount++ > 4)
//		{
			close(sok);
			return  1;
//		}
	}
	SVP_INFO ("UDP Port:%d waiting for a packet...\n",Port);
	while(true)
	{
		memset(udpbuf,0,UDPBUTSIZE);
		rsize = recvfrom(sok, udpbuf, UDPBUTSIZE, 0, (struct sockaddr*)&remote_addr, &sin_size);
//		if( rsize < 0 )
//		{
//			iError = CheckError();
//			close (sok);
//			sok = -1;
//			return -1;
//		}
		if( rsize <= 0 )
		{
			SVP_INFO( "UDP Port:%d is shutdown" ,Port);
			CheckError();
			break;
		}
		SVP_INFO ("UDP port:%d received packet from %s : %s",Port, inet_ntoa (remote_addr.sin_addr) ,udpbuf);
		if(iscmd)
			listener_->OnCmdData(udpbuf,rsize);
		else
			listener_->OnResultData(udpbuf,rsize);
	}
	listener_->OnNetState(NetMgrListener::Net_UDP_EXIT);
	close (sok);
	sok = -1;
	return 0;
}

int NetMgr::InitTCPToRec(int &newsocket,int &Port)
{
	int iError, rsize = 1;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	std::string Phone_IPaddr;
	socklen_t  sin_size = sizeof (struct sockaddr_in);
	memset (&my_addr, 0, sizeof (my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(Port);

	if ((pmc_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	  SVP_ERROR ("socket error ");
	  return 1;
	}
	int on = 1,ReRetryCount = 0;
	setsockopt( pmc_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	while(bind (pmc_socket, (struct sockaddr *) &my_addr, sizeof (struct sockaddr)) < 0)
	{
		SVP_ERROR ("bind error,start rebind");
//		my_addr.sin_port = htons(++Port);
//		usleep(300000);
//		if( ReRetryCount++ > 4 )
//		{
			close (pmc_socket);
			return 1;
//		}
	}

	iError = listen (pmc_socket, 20);
	if( iError < 0 )
	{
		SVP_ERROR("the TCP Port:%d listen return error",Port);
		close (pmc_socket);
		pmc_socket = -1;
		return 1;
	}
	SVP_INFO ("Car_TCP_Port:%d waiting connect...\n",Port );
	m_TCPBuffer = new char[TCPBUTSIZE];
	while(pmc_socket > 0)
	{
		newsocket = accept (pmc_socket, (struct sockaddr *) &remote_addr, &sin_size);//这里不支持一个端口，多个客户连接
		if (newsocket < 0)
		{
			SVP_ERROR("the TCP Port:%d accept return error",Port);
			m_ConectState = NetMgrListener::Net_TCP_CONNECT;
			listener_->OnNetState( m_ConectState );
			break;
		}
		Phone_IPaddr = inet_ntoa(remote_addr.sin_addr);
		SVP_INFO ("accept client %s\n", Phone_IPaddr.c_str());
		m_ConectState = NetMgrListener::Net_TCP_CONNECT;
		listener_->OnNetState( m_ConectState );
		while(rsize > 0)
		{
			rsize = recv (newsocket, m_TCPBuffer, TCPBUTSIZE, MSG_WAITALL);
			if( rsize > 0 )
			{
				SVP_INFO("TCP Recive %d Size ...........",rsize);
				listener_->OnPCMData(m_TCPBuffer,rsize);
			}
		}
		close(newsocket);
		m_ConectState = NetMgrListener::Net_TCP_DISCONNECT;
		listener_->OnNetState( m_ConectState );
		break;
	}
	delete m_TCPBuffer;
	m_TCPBuffer = NULL;
	close (newsocket);
	close (pmc_socket);
	newsocket = -1;
	pmc_socket = -1;
	ExitAllTread();/*TCP Down , All Thread Down*/
	return 0;
}

void* NetMgr::CmdListenThread(void *arg)
{
	pthread_detach(pthread_self());
	NetMgr* pthis = (NetMgr*)arg;
	pthis->m_InitError |= pthis->InitUdpToRec(pthis->cmd_socket,pthis->_Local_CMD_Port, 1);
	pthread_exit((void *)0);
	return NULL;
}

void* NetMgr::ResultListenThread(void *arg)
{
	pthread_detach(pthread_self());
	NetMgr* pthis = (NetMgr*)arg;
	pthis->m_InitError |= pthis->InitUdpToRec(pthis->result_socket,pthis->_Local_Result_Port, 0);
	pthread_exit((void *)0);
	return NULL;
}

void* NetMgr::PCMListenThread(void *arg)
{
	pthread_detach(pthread_self());
	NetMgr* pthis = (NetMgr*)arg;
	pthis->m_InitError |= pthis->InitTCPToRec(pthis->connet_socket,pthis->_Local_Data_Port);
	pthread_exit((void *)0);
	return NULL;
}

void NetMgr::ExitAllTread()
{
	if( cmd_socket != -1 )
		shutdown(cmd_socket , SHUT_RDWR );
	if( result_socket != -1 )
		shutdown(result_socket , SHUT_RDWR );
	if( connet_socket != -1 )
		shutdown(connet_socket , SHUT_RDWR );
	if( pmc_socket != -1 )
		shutdown(pmc_socket , SHUT_RDWR );
	close(cmd_socket);
	close(result_socket);
	close(connet_socket);
	close(pmc_socket);/* 即使线程退出，造成close两次，也就返回值报错，无关紧要 */
}

bool NetMgr::InitAndStart(NetMgrListener * listen,int DataPort,int CmdPort,int ResultPort)
{
	int iError;
	pthread_t tcptid,resulttid,cmdtid;
	_Local_CMD_Port  = CmdPort;
	_Local_Result_Port = ResultPort;
	_Local_Data_Port = DataPort;
	iError = pthread_create(&tcptid, NULL, CmdListenThread, (void*)this);
	iError |= pthread_create(&resulttid, NULL, ResultListenThread, (void*)this);
	iError |= pthread_create(&cmdtid, NULL, PCMListenThread, (void*)this);
	usleep(200000);
	iError |= m_InitError;
	if( iError )
	{
		SVP_INFO("thread create have one or more fail !");
		/*全盘退出*/
		ExitAllTread();
		return false;
	}
	RegisterListener(listen);
	return true;
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


void UdpBroadcast(const std::string& addr, int port, const std::string& data)
{
	int udp_fd;
	int rsize;
	struct sockaddr_in remote_addr;
	memset (&remote_addr, 0, sizeof (remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(addr.c_str());
//	if (remote_addr.sin_addr.s_addr == INADDR_NONE)
//	{
//		SVP_INFO("Incorrect ip address!");
//		return ;
//	}
	remote_addr.sin_port = htons(port);
	if ((udp_fd = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		SVP_ERROR ("socket error");
		return ;
	}
	int yes=1;
	setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));   //init so that can broadcast
	rsize =sendto(udp_fd, data.data(), data.length() , 0, (struct sockaddr *) &remote_addr,sizeof(struct sockaddr));
	if(rsize > 0)
	{
		SVP_INFO("Sendto %s : %d : %s",addr.c_str(),port,data.c_str());
	}
	else
	{
		SVP_INFO("Sendto Error %s : %d : %s",addr.c_str(),port,data.c_str());
		CheckError();
	}
	close (udp_fd);
}
