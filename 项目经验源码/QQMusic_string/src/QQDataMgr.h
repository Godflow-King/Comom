#ifndef __QQDataMgr_H__
#define __QQDataMgr_H__

#include <list>
#include <vector>
#include <mutex>
#include "QQTcp.h"
#include "QQUdp.h"
#include "QQData.h"
#include "Poco/Timer.h"

class CQQDataMgr
{
public:
	enum{
		QQ_Disconnect,
		QQ_Connecting,
		QQ_Connected,
	};
	enum{
		QQ_RepeatAll,
		QQ_RepeatSingle,
		QQ_RandomAll,
	};
	enum{
		QQ_OP_PLAY,
		QQ_OP_PAUSE,
		QQ_OP_NEXT,
		QQ_OP_PREVIOUS,
	};
	enum{
		QQ_Error_OK = 0,
		QQ_Error_LocalFailed = 100,
		QQ_Error_MobileFailed = 101,
		QQ_Error_ParentID_NoExist = 102,    //父节点不存在
		QQ_Error_Network_Unavailable = 103, //无网络信号
		QQ_Error_Unknow = 104,  //未知错误
		QQ_Error_SongID_NoExist = 105, //歌曲不存在, 播放下一曲
		QQ_Error_ReadDataFailed = 106, //数据读取错误, 播放下一曲
		QQ_Error_InvalidParameter = 107,
		QQ_Error_CallError = 108,
		QQ_Error_NoCopyRight = 109, //歌曲无版权, 播放下一曲
		QQ_Error_NoLogin = 110, //请登录QQ音乐
		QQ_Error_PCM_RepeatRequest = 111,
	};
	enum{
		QQ_Notify_ID3Update,
		QQ_Notify_ListError,
		QQ_Notify_ListUpdate,
		QQ_Notify_PicUpdate,
		QQ_Notify_InvalidSong,
		QQ_Notify_ConnectStatus,
		QQ_Notify_PlayPosChanged,
		QQ_Notify_PlayPauseUpdate,
	};
	static CQQDataMgr* GetInstance();

	void SelectListItem(int listid, bool& bIsSong, bool& bListExisted);
	void SetConnectStatus(int status);
	void Op(int opType);
	void SetPlaylistMode(unsigned char mode);
	bool GetPlayPauseStatus(){ return m_bPlayStatus;};

	int GetConnectStatus();
	void GetMediaInfo(stQQMediaInfo& info);
	bool GetId3Pic(std::string& data);
	bool GetMusicList(CQQMusicList& layer, std::string& path);
protected:
	CQQDataMgr();
    ~CQQDataMgr();
private:
    static void TcpReceiveDataFunc(char* pData, int len);
    static void TcpConnectStateFunc(const std::string& ip, bool bConnected);
    static void UdpReceiveDataFunc(char* pData, int len, int port);
    static void* ThreadPlayPcmData(void *arg);
    void PlayPcmData();
    void _TcpReceiveData(char* pData, int len);
    void _UdpReceiveCmdData(char* pData, int len);
    void _UdpReceiveResultData(char* pData, int len);
    void _TcpConnectState(const std::string& ip, bool bConnected);
    void ParseDataHead();

    void OnHeartbeatTimer(Poco::Timer& timer);
    void OnNextSongTimer(Poco::Timer& timer);
    void SendDiscover();
    void SendDeviceInfos();
    void SendHeartbeat();
    void SendGetMobileInfos();
    void SendGetList();
    void SendGetMediaInfo();
    void SendGetPCMData();
    void SendGetPicData();
    void SendDisconnect();
    int GetItemFromPlaylist(const std::string& songid);
    void PlaySong(const std::string& songid);
    void NotifyStatusChange(int id);
private:
    static CQQDataMgr* m_pInstance;
    CQQTcp* m_pTcp;
    CQQUdp* m_pUdpCmd;
    CQQUdp* m_pUdpResult;
    int m_connectStatus;
    CQQData m_PCMData;
    CQQData m_PicData;
    std::string m_DataHead;

    int m_LocalCmdPort;
    int m_LocalDataPort;
    int m_LocalResultPort;
    int m_RecvLastRequestID;
    std::string m_LocalMacAddr;

    int m_BroadPort;
    int m_RemoteCmdPort;
    int m_RemoteResultPort;
    unsigned char m_playMode;
    bool m_bPlayStatus;
    std::string m_RemoteMacAddr;
    std::string m_RemoteBrand;
    std::string m_BroadIpAddr;
    std::vector<CQQMusicList> m_vMusicLayers;
    std::vector<CQQMusicList> m_vPlayLayers;
    std::vector<stQQMusicItem> m_vPlayList;
    stQQMediaInfo m_stCurrMediaInfo;

    Poco::Timer   m_timerHeartbeat;
    Poco::Timer   m_timerNextSong;
    std::recursive_mutex m_mtxList;
    std::recursive_mutex m_mtxPcmData;
};

#endif
