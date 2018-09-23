#ifndef  SVP_NET_MGR_H_
#define  SVP_NET_MGR_H_

#include <string>
#include <pthread.h>
#include <stdint.h>
//#include "qmusicbase.h"


class NetMgrListener
{
public:
	typedef enum
	{
		Net_TCP_DISCONNECT = 0,
		Net_TCP_CONNECT,
		Net_UDP_EXIT,
	}NETMGR_STATE;
	virtual void OnCmdData(char *pdata ,int size) {(void)pdata;};
	virtual void OnResultData(char *pdata ,int size ) {(void)pdata;};
	virtual void OnPCMData(char *pdata ,int size ) {(void)pdata;};
	virtual void OnNetState(NETMGR_STATE state ) {(void)state;};
};

class NetMgr
{
	typedef void (*NetReciveData)(std::string &stream);
public:
	static NetMgr* GetInstance()
	{
		static NetMgr instance;
	        return &instance;
	}
//	friend class DataMgr;

	bool InitAndStart(NetMgrListener * listen,int DataPort,int CmdPort,int ResultPort);
	void ExitAllTread();
//	std::string GetLocalMacAddr(const std::string& devicename);
//	std::string GetLocalIpV4Addr(const std::string& devicename);

	void RegisterListener( NetMgrListener * listen )
	{
//		listener_= boost::shared_ptr<SVPNetMgrListener>(listen, boost::null_deleter());
		listener_ = listen;
	}

private:
	NetMgr();
	virtual ~NetMgr();
	NetMgr(const NetMgr &other) {};
	NetMgr &operator = (const NetMgr &other) {};

	static void* CmdListenThread(void *arg);
	static void* ResultListenThread(void *arg);
	static void* PCMListenThread(void *arg);

	int InitUdpToRec(int &socket,int &Port,int iscmd);
	int InitTCPToRec(int &socket,int &Port);

private:
	int m_InitError;
	int cmd_socket,result_socket,pmc_socket,connet_socket;

	int _Local_CMD_Port ;
	int _Local_Result_Port;
	int _Local_Data_Port;


//	boost::shared_ptr<SVPNetMgrListener>  listener_;
	NetMgrListener *listener_;
	NetMgrListener::NETMGR_STATE m_ConectState;
	char *m_TCPBuffer;
};

std::string GetLocalMacAddr(const std::string& devicename);
std::string GetLocalIpV4Addr(const std::string& devicename);
void UdpBroadcast(const std::string& _addr, int port, const std::string& data);

#endif
