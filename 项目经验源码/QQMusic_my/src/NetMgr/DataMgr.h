#ifndef  SVP_DATA_MGR_H_
#define  SVP_DATA_MGR_H_
#include "NetMgr/NetMgr.h"
#include <pthread.h>
#include "Poco/Timer.h"
#include "QQData.h"

class DataMgrListener
{
//public:
//	virtual void OnQplayStatus(int state ) {(void)state;};
//	virtual void OnQplayList(QQMusicItemsList &list ) {(void)list;};
//	virtual void OnQplayBitData( std::string &data, int length, int type ) {(void)data;(void)length; (void)type;};
};

class DataMgr: public NetMgrListener
{
public:
	typedef enum
	{
		QQ_Disconnect,
		QQ_Connecting,
		QQ_Connected,
	}QQCONNECT_STATE;
	typedef enum
	{
		QQ_Error_OK = 0,
		QQ_Error_LocalFailed = 100,
		QQ_Error_MobileFailed = 101,
		QQ_Error_ParentID_NoExist = 102,
		QQ_Error_Network_Unavailable = 103,
		QQ_Error_Unknow = 104,
		QQ_Error_SongID_NoExist = 105,
		QQ_Error_ReadDataFailed = 106,
		QQ_Error_InvalidParameter = 107,
		QQ_Error_CallError = 108,
		QQ_Error_NoCopyRight = 109,
		QQ_Error_NoLogin = 110,
		QQ_Error_PCM_RepeatRequest = 111,
	}QQ_ERROR;

	static DataMgr* GetInstance()
	{
		static DataMgr instance;
	        return &instance;
	}
	void SetConnectStatus(int status);
	int GetConnectStatus();
	void SelectListItem(int listid);
	bool GetMusicList(std::vector<stQQMusicItem>& items);
	void Exit();
	bool Init();

protected:
	virtual void OnCmdData(char *pdata ,int size) ;
	virtual void OnResultData(char *pdata ,int size );
	virtual void OnPCMData(char *pdata ,int size );
	virtual void OnNetState(NETMGR_STATE state );

//public:
//	int Phone_CommPort = 43958;
//	int Phone_ResuPort = 43958;
//
//	int Car_CMD_Port 	= 43955;
//	int Car_Result_Port = 43959;
//	int Car_TCP_Port  	= 43997;
//	std::string Phone_IPaddr; //qqmusic Phone IP
//	std::string Broad_IPaddr = "255.255.255.255"; //Broadcast IP

private:
	DataMgr();
	virtual ~DataMgr();
	DataMgr(const DataMgr &other) {};
	DataMgr &operator = (const DataMgr &other) {};

	void SendDiscover();
	void SendGetPCMData();
	void SendGetPicData();
	void SendDeviceInfos();
	void SendHeartbeat();
	void SendDisconnect();
	void SendGetMobileInfos();
	void SendGetMediaInfo();
	void SendGetList();

	void ParseDataHead();
	void OnHeartbeatTimer(Poco::Timer& timer);
	 static void* ThreadPlayPcmData(void *arg);

private:
	DataMgrListener * listen_;

    int m_connectStatus;
    CQQData m_PCMData;
    CQQData m_PicData;
    std::string m_DataHead;

//    int m_RecvLastRequestID;
	int m_LocalDataPort;
	int m_LocalCmdPort;
	int m_LocalResultPort;
    std::string m_LocalMacAddr;

    int m_BroadPort;
    int m_RemoteCmdPort;
    int m_RemoteResultPort;
    std::string m_RemoteMacAddr;
    std::string m_RemoteBrand;
    std::string m_BroadIpAddr;
    std::vector<CQQMusicList> m_vMusicLayers;
    std::vector<CQQMusicList> m_vPlayLayers;
    stQQMediaInfo m_stCurrMediaInfo;

	Poco::Timer   m_timerHeartbeat;
	std::recursive_mutex m_mtxPcmData;
	bool m_ispcmThreadRun;
};

#endif
