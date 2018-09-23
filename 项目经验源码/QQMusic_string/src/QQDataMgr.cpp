#include "QQDataMgr.h"
#include "SVPLog.h"
#include "json/src/Utils.h"
#include "json/src/writer.h"
#include "SVPTime.h"
//#include "WebAppMgr.h"
#include "SVPPcmPlayer.h"
//#include <fcntl.h>

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG  "QQDataMgr"

#define WLAN_DEVICE_NAME "eth0" //“wlan0”, "eth0"
#define PCM_BUFFER_SIZE 4194304 //4*1024*1024
#define PCM_PACK_BUFFER 1024*1024

CQQDataMgr* CQQDataMgr::m_pInstance = NULL;
CQQDataMgr::CQQDataMgr()
: m_connectStatus(QQ_Disconnect)
, m_BroadPort(43921)
, m_LocalDataPort(43997)
, m_LocalCmdPort(43955)
, m_LocalResultPort(43959)
, m_bPlayStatus(true)
, m_playMode(QQ_RepeatAll)
{
	m_BroadIpAddr = "255.255.255.255";
	m_PCMData.Clear();
	m_PicData.Clear();
	m_LocalMacAddr = GetLocalMacAddr(WLAN_DEVICE_NAME);
	SVP_INFO("Local Mac Addr %s, IP %s", m_LocalMacAddr.c_str(), GetLocalIpV4Addr(WLAN_DEVICE_NAME).c_str());

	m_pTcp = new CQQTcp(CQQDataMgr::TcpReceiveDataFunc, CQQDataMgr::TcpConnectStateFunc, m_LocalDataPort);
	m_pUdpCmd = new CQQUdp(CQQDataMgr::UdpReceiveDataFunc, m_LocalCmdPort);
	m_pUdpResult = new CQQUdp(CQQDataMgr::UdpReceiveDataFunc, m_LocalResultPort);

	pthread_t m_tid;
	pthread_create(&m_tid, NULL, ThreadPlayPcmData, (void*)this);
}

CQQDataMgr::~CQQDataMgr()
{
	delete m_pTcp;
	delete m_pUdpCmd;
	delete m_pUdpResult;
}

CQQDataMgr* CQQDataMgr::GetInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new CQQDataMgr();
    }
    return( m_pInstance );
}

void CQQDataMgr::TcpReceiveDataFunc(char* pData, int len)
{
	CQQDataMgr::GetInstance()->_TcpReceiveData(pData, len);
}

void CQQDataMgr::UdpReceiveDataFunc(char* pData, int len, int port)
{
	if (CQQDataMgr::GetInstance()->m_LocalCmdPort == port)
	{
		CQQDataMgr::GetInstance()->_UdpReceiveCmdData(pData, len);
	}
	else
	{
		CQQDataMgr::GetInstance()->_UdpReceiveResultData(pData, len);
	}
}

void CQQDataMgr::TcpConnectStateFunc(const std::string& ip, bool bConnected)
{
	CQQDataMgr::GetInstance()->_TcpConnectState(ip, bConnected);
	SVP_INFO("TcpConnectStateFunc ip %s, connected %d", ip.c_str(), bConnected);
}

void CQQDataMgr::_TcpReceiveData(char* pData, int len)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPcmData);
	int picCopyLen = m_PicData.copyedLen;
	int pos = m_PCMData.CopyData(pData, len);
	if (pos == 0)
	{
		m_PicData.CopyData(pData, len);
	}
	if (len - pos > 0)
	{
		m_DataHead.append(pData + pos, len - pos);
		ParseDataHead();
		SendGetPCMData();
	}
	if (picCopyLen != m_PicData.copyedLen && m_PicData.totalLen == m_PicData.copyedLen)
	{
		//[notify] ID3图片下载完成
		NotifyStatusChange(QQ_Notify_PicUpdate);
	}
	//测试，将数据写入文件
//	static bool bFirst = true;
//	if (m_PicData.totalLen == m_PicData.data.length() && bFirst)
//	{
//		int fdFile = open("/work/b.bmp", O_RDWR|O_CREAT);
//		write(fdFile, m_PicData.data.c_str(), m_PicData.data.length());
//		close(fdFile);
//		bFirst = false;
//	}
}

void CQQDataMgr::ParseDataHead()
{
//	SVP_INFO("Head: %s", m_DataHead.c_str());
	int headBegin = m_DataHead.find("{\"PCMData\"");
	int headPICBegin = m_DataHead.find("{\"PICData\"");
	if (headBegin == std::string::npos ||
		(headPICBegin != std::string::npos && headPICBegin < headBegin))
		headBegin = headPICBegin;
	int headEnd = m_DataHead.find("\r\n", headBegin);
//	SVP_INFO("Head pos: %d, %d", headBegin, headEnd);
	//没有找到报文头
	if (headEnd == std::string::npos || headBegin == std::string::npos)
		return;
	std::string jsonStr = m_DataHead.substr(headBegin, headEnd - headBegin);
//	SVP_INFO("Head: %s", jsonStr.c_str());
	m_DataHead.erase(0, headEnd + 2);
	Json::Value root = CUtils::LoadJsonFromString(jsonStr);
	Json::Value item;
	CQQData* pQQData = NULL;
	if (!root["PCMData"].isNull())
	{
		pQQData = &m_PCMData;
		item = root["PCMData"];
	}
	else if (!root["PICData"].isNull())
	{
		pQQData = &m_PicData;
		item = root["PICData"];
	}
	std::string songid;
	CUtils::JsonToValue(&item["SongID"], songid);
	if (songid == pQQData->songid)
	{
		CUtils::JsonToValue(&item["PackageIndex"], pQQData->packindex);
		CUtils::JsonToValue(&item["Length"], pQQData->packLen);
		CUtils::JsonToValue(&item["TotalLength"], pQQData->totalLen);
		pQQData->packcopyedLen = 0;
		SVP_INFO("Head: packageindex %d, len %d, totalLen %d",
				pQQData->packindex, pQQData->packLen, pQQData->totalLen);
		//拷贝剩余数据
		int datalen = pQQData->CopyData((char*)m_DataHead.data(), m_DataHead.size());
		m_DataHead.erase(0, datalen);
	}
}

void CQQDataMgr::_TcpConnectState(const std::string& ip, bool bConnected)
{
	if (bConnected)
		SetConnectStatus(QQ_Connected);
	else
		SetConnectStatus(QQ_Disconnect);
}

void CQQDataMgr::_UdpReceiveCmdData(char* pData, int len)
{
	Json::Value root = CUtils::LoadJsonFromString(pData);
	std::string cmdID;
	CUtils::JsonToValue(&root["Request"], cmdID);
	if (cmdID == "CommInfos")
	{
		CUtils::JsonToValue(&root["Arguments"]["CommandPort"], m_RemoteCmdPort);
		CUtils::JsonToValue(&root["Arguments"]["ResultPort"], m_RemoteResultPort);
//		SVP_INFO("_UdpReceiveData CommandPort %d, ResultPort %d", m_RemoteCmdPort, m_RemoteResultPort);
	}
	else if (cmdID == "DeviceInfos")
	{
		SendDeviceInfos();
		SendGetMobileInfos();
	}
	else if (cmdID == "Disconnect")
	{
		SetConnectStatus(QQ_Disconnect);
	}
}

void CQQDataMgr::_UdpReceiveResultData(char* pData, int len)
{
	Json::Value root = CUtils::LoadJsonFromString(pData);
	if (!root["MobileDeviceInfos"].isNull())
	{
		std::string macAddr;
		CUtils::JsonToValue(&root["MobileDeviceInfos"]["Brand"], m_RemoteBrand);
		CUtils::JsonToValue(&root["MobileDeviceInfos"]["Mac"], macAddr);
		if (macAddr != m_RemoteMacAddr)
		{
			m_RemoteMacAddr = macAddr;
			m_mtxList.lock();
			m_vPlayList.clear();
			m_mtxList.unlock();
		}
		if (m_vPlayList.empty())
		{
			m_mtxList.lock();
			m_vMusicLayers.clear();
			m_vPlayLayers.clear();
			m_stCurrMediaInfo.Clear();
			m_mtxList.unlock();
			bool bIsSong, bListExisted;
			SelectListItem(0, bIsSong, bListExisted);
		}
	}
	else if (!root["Items"].isNull())
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxList);
		if (m_vMusicLayers.empty())
			return;
		std::string parentid;
		int pageindex = 0;
		CQQMusicList* pLayer = &m_vMusicLayers.back();
		pLayer->querytime = 0;
		CUtils::JsonToValue(&root["Items"]["ParentID"], parentid);
		CUtils::JsonToValue(&root["Items"]["PageIndex"], pageindex);
		CUtils::JsonToValue(&root["Items"]["Error"], pLayer->errorcode);
		if (pLayer->errorcode != QQ_Error_OK)
		{
			pLayer->pageindex = 0;
			pLayer->count = pLayer->items.size();
			//[notify] 列表下载错误，错误类型有： 102, 110, 103
			NotifyStatusChange(QQ_Notify_ListError);
			return;
		}
		if (parentid != pLayer->parentid || pageindex == pLayer->pageindex)
			return;
		pLayer->pageindex = pageindex;
		CUtils::JsonToValue(&root["Items"]["Count"], pLayer->count);
		Json::Value jsonLists = root["Items"]["Lists"];
		int arraySize = CUtils::JsonArraySize(&jsonLists);
//		SVP_INFO("Items %s, %s, %d, %d, %d", parentid.c_str(), pLayer->parentid.c_str(),
//				pageindex, pLayer->pageindex, arraySize);
		for (int i = 0; i < arraySize; i++)
		{
			stQQMusicItem item;
			CUtils::JsonToValue(&jsonLists[i]["ID"], item.id);
			CUtils::JsonToValue(&jsonLists[i]["Name"], item.name);
			CUtils::JsonToValue(&jsonLists[i]["Artist"], item.artist);
			CUtils::JsonToValue(&jsonLists[i]["Album"], item.album);
			CUtils::JsonToValue(&jsonLists[i]["Type"], item.type);
			CUtils::JsonToValue(&jsonLists[i]["Duration"], item.duration);
			pLayer->items.push_back(item);
		}
		if (arraySize == 0)
			pLayer->count = pLayer->items.size();
		//[Notify] 列表更新完成
		if (pLayer->IsDownloadComplete())
			NotifyStatusChange(QQ_Notify_ListUpdate);
		else
			SendGetList();
	}
	else if (!root["MediaInfo"].isNull())
	{
		Json::Value jsonValue = root["MediaInfo"];
		CUtils::JsonToValue(&jsonValue["SongID"], m_stCurrMediaInfo.id);
		CUtils::JsonToValue(&jsonValue["PCMDataLength"], m_stCurrMediaInfo.length);
		CUtils::JsonToValue(&jsonValue["Rate"], m_stCurrMediaInfo.rate);
		CUtils::JsonToValue(&jsonValue["Bit"], m_stCurrMediaInfo.bit);
		CUtils::JsonToValue(&jsonValue["Channel"], m_stCurrMediaInfo.channel);
		CUtils::JsonToValue(&jsonValue["Error"], m_stCurrMediaInfo.errorcode);
		m_timerNextSong.stop();
		m_stCurrMediaInfo.querytime = 0;
		if (m_stCurrMediaInfo.Valid())
		{
			SendGetPicData();
			SendGetPCMData();
		}
		else  //无效歌曲: 105,109,106,111
		{
			NotifyStatusChange(QQ_Notify_InvalidSong);
		}
	}
}

void CQQDataMgr::OnHeartbeatTimer(Poco::Timer& timer)
{
	if (m_connectStatus == QQ_Connecting)
	{
		SendDiscover();
	}
	else if (m_connectStatus == QQ_Connected)
	{
		SendHeartbeat();
		SendGetPCMData();
		//丢包检测
		uint32_t currTickCount = SVPTime::getTickCount();
		uint32_t queryTime = 0;
		if (m_stCurrMediaInfo.retrycount >= 2)
		{
			m_stCurrMediaInfo.errorcode = QQ_Error_ReadDataFailed;
			NotifyStatusChange(QQ_Notify_InvalidSong);
		}
		else if (m_stCurrMediaInfo.querytime != 0 &&
				currTickCount - m_stCurrMediaInfo.querytime >= 4000)
			SendGetMediaInfo();
		m_mtxList.lock();
		if (!m_vMusicLayers.empty())
			queryTime = m_vMusicLayers.back().querytime;
		if (queryTime != 0 &&
				currTickCount - queryTime >= 4000)
			SendGetList();
		m_mtxList.unlock();
	}
}

void CQQDataMgr::SendDiscover()
{
	Json::Value item;
	item["DeviceIP"] = Json::Value(GetLocalIpV4Addr(WLAN_DEVICE_NAME));
	item["DeviceID"] = m_LocalMacAddr;
	item["DeviceName"] = "SGMW";
	item["DataPort"] = m_LocalDataPort;
	item["CommandPort"] = m_LocalCmdPort;
	item["ResultPort"] = m_LocalResultPort;
	item["DeviceType"] = 1;
	item["ConnectType"] = 1;
	item["DeviceBrand"] = "SGMW";
	Json::Value  root;
	Json::FastWriter jsonWriter;
	root["Discover"] = item;
	std::string jsonStr = jsonWriter.write(root)+"\r\n";
	UdpBroadcast(m_BroadIpAddr, m_BroadPort, jsonStr);
}

void CQQDataMgr::SendGetPCMData()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPcmData);
	if (m_PCMData.songid != m_stCurrMediaInfo.id)
	{
		m_PCMData.Clear();
		m_PCMData.songid = m_stCurrMediaInfo.id;
	}
	if (!m_stCurrMediaInfo.Valid() || m_PCMData.totalLen <= m_PCMData.copyedLen \
			|| m_PCMData.data.size() > PCM_BUFFER_SIZE )
		return;

	Json::Value item;
	item["SongID"] = m_PCMData.songid;
	item["PackageIndex"] = m_PCMData.packindex + 1;
	Json::Value  root;
	Json::FastWriter jsonWriter;
	root["Request"] = "PCMData";
	root["Arguments"] = item;
	std::string jsonStr = jsonWriter.write(root)+"\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void CQQDataMgr::SendGetPicData()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPcmData);
	if (m_PicData.songid != m_stCurrMediaInfo.id)
	{
		m_PicData.Clear();
		m_PicData.songid = m_stCurrMediaInfo.id;
	}
	if (!m_stCurrMediaInfo.Valid() || m_PicData.totalLen <= m_PicData.copyedLen)
		return;
	Json::Value item;
	item["SongID"] = m_PicData.songid;
	item["PackageIndex"] = m_PicData.packindex + 1;
	Json::Value  root;
	Json::FastWriter jsonWriter;
	root["Request"] = "PICData";
	root["Arguments"] = item;
	std::string jsonStr = jsonWriter.write(root)+"\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void CQQDataMgr::SendDeviceInfos()
{
	std::string jsonStr = std::string("{\"DeviceInfos\":")
	  + "{\"Brand\":\"SGMW\",\"Models\":\"SGMW\",\"Network\":1,\"OS\":\"linux\","
	  + "\"OSVer\":\"G5\",\"PCMBuf\":1048576,\"PICBuf\":1048576,\"Ver\":\"1.0\"}}\r\n";
    UdpBroadcast(m_BroadIpAddr, m_RemoteResultPort, jsonStr);
}

void CQQDataMgr::SendHeartbeat()
{
	std::string jsonStr = "{\"Request\":\"Heartbeat\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void CQQDataMgr::SendDisconnect()
{
	std::string jsonStr = "{\"Request\":\"Disconnect\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void CQQDataMgr::SendGetMobileInfos()
{
	std::string jsonStr = "{\"Request\":\"MobileDeviceInfos\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void CQQDataMgr::SendGetMediaInfo()
{
	if (m_stCurrMediaInfo.id.empty())
		return;
	m_stCurrMediaInfo.querytime = SVPTime::getTickCount();
	m_stCurrMediaInfo.retrycount++;
	std::string jsonStr = "{\"Request\":\"MediaInfo\",\"Arguments\":{\"SongID\":\"%s\"}}\r\n";
	char buffer[256];
	sprintf(buffer, jsonStr.c_str(), m_stCurrMediaInfo.id.c_str());
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, buffer);
}

void CQQDataMgr::SendGetList()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	if (m_vMusicLayers.empty())
		return;
	CQQMusicList* pLayer = &m_vMusicLayers.back();
	SVP_INFO("SendGetList() %d, pageindex %d, itemsize %d, count %d",
			pLayer->IsDownloadComplete(), pLayer->pageindex, pLayer->items.size(), pLayer->count);
	if (pLayer->IsDownloadComplete())
		return;
	pLayer->querytime = SVPTime::getTickCount();
	Json::Value item;
	item["ParentID"] = pLayer->parentid;
	item["PageIndex"] = pLayer->pageindex + 1;
	item["PagePerCount"] = 50;
	Json::Value  root;
	Json::FastWriter jsonWriter;
	root["Request"] = "Items";
	root["Arguments"] = item;
	std::string jsonStr = jsonWriter.write(root)+"\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
	SVP_INFO("SendGetList %s", jsonStr.c_str());
}

void CQQDataMgr::SetConnectStatus(int status)
{
	if ((status == m_connectStatus) ||
		(status == QQ_Connected && m_connectStatus != QQ_Connecting) ||
		(status == QQ_Connecting && m_connectStatus != QQ_Disconnect) )
		return;
	m_connectStatus = status;
	m_timerHeartbeat.stop();
	if (status == QQ_Disconnect)
	{
		SendDisconnect();
		m_mtxPcmData.lock();
		m_PCMData.Clear();
		m_PicData.Clear();
		m_mtxPcmData.unlock();
		m_pTcp->Disconnect();
	}
	else
	{
		m_timerHeartbeat.setStartInterval(0);
		m_timerHeartbeat.setPeriodicInterval(2500);
		m_timerHeartbeat.start(Poco::TimerCallback<CQQDataMgr>(*this, &CQQDataMgr::OnHeartbeatTimer));
	}
	//[Notify] 连接状态通知
	NotifyStatusChange(QQ_Notify_ConnectStatus);
}

int CQQDataMgr::GetConnectStatus()
{
	return m_connectStatus;
}

void CQQDataMgr::SelectListItem(int listid, bool& bIsSong, bool& bListExisted)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	int layersize = m_vMusicLayers.size();
	bIsSong = false;
	bListExisted = false;
	//返回上一级
	if (listid < 0 && layersize > 1)
	{
		m_vMusicLayers.pop_back();
		bListExisted = true;
		return;
	}
	//根目录
	CQQMusicList nextLayer;
	nextLayer.Clear();
	if (layersize == 0)
	{
		nextLayer.parentid = "-1";
		m_vMusicLayers.push_back(nextLayer);
		SendGetList();
		return;
	}
	CQQMusicList* pLayer = &m_vMusicLayers.back();
	//界面点击项序号超出范围
	if (listid >= pLayer->items.size())
	{
		bListExisted = true;
		return;
	}
	stQQMusicItem selectitem = pLayer->items[listid];
	if (selectitem.id == m_stCurrMediaInfo.id)
	{
		bIsSong = true;
		return;
	}
	if (selectitem.type == QQMusic_Type_Song)
	{
		bool bIsPlayingDir = !m_vPlayLayers.empty() && \
				m_vPlayLayers.back().parentid == m_vMusicLayers.back().parentid;
		if (!bIsPlayingDir)
		{
			m_vPlayLayers = m_vMusicLayers;
			SetPlaylistMode(m_playMode);
		}
		PlaySong(selectitem.id);
		bIsSong = true;
	}
	else
	{
		//如果播放列表中有缓存，取播放列表数据。(非必要，有问题可以删除for代码)
		for (int i = 0; i < m_vPlayLayers.size(); i++)
		{
			if (m_vPlayLayers[i].parentid == selectitem.id && !m_vPlayLayers[i].items.empty())
			{
				bListExisted = true;
				nextLayer = m_vPlayLayers[i];
				m_vMusicLayers.push_back(nextLayer);
				break;
			}
		}
		if (!bListExisted)
		{
			nextLayer.parentid = selectitem.id;
			nextLayer.parentname = selectitem.name;
			m_vMusicLayers.push_back(nextLayer);
			SendGetList();
		}
	}
}


bool CQQDataMgr::GetMusicList(CQQMusicList& layer, std::string& path)
{
	if (m_vMusicLayers.empty())
		return false;
	path = "";
	for (int i = 0; i < m_vMusicLayers.size(); i++)
		path += m_vMusicLayers[i].parentname + "/";
	layer = m_vMusicLayers.back();
	return true;
}

void* CQQDataMgr::ThreadPlayPcmData(void *arg)
{
	pthread_detach(pthread_self());
	CQQDataMgr* pthis = (CQQDataMgr*)arg;
	pthis->PlayPcmData();
	return 0;
}

void CQQDataMgr::PlayPcmData()
{
	uint8_t* buffer = new uint8_t[PCM_PACK_BUFFER];
	int len,pack_playpos,pack_playsize,sizepersecond = 44100*2;
	int last_playpos,playpos,byte_playpos;
	static std::string songid;
	stQQMediaInfo mediaInfo;
	bool ret;

	while(true)
	{
		m_mtxPcmData.lock();
		/* 换新数据包不做任何别的操作 */
		len = m_PCMData.GetData(buffer, PCM_PACK_BUFFER);
		m_mtxPcmData.unlock();
		if (len <= 0)
		{
			usleep(100000);
			continue;
		}

		if( mediaInfo != m_stCurrMediaInfo && m_stCurrMediaInfo.Valid())
		{
			mediaInfo = m_stCurrMediaInfo;
			sizepersecond = mediaInfo.rate*mediaInfo.channel*mediaInfo.bit/8;
			SVPSingleton<SVPPcmPlayer>::getInstance()->PcmFlush();
			SVPSingleton<SVPPcmPlayer>::getInstance()->PcmExit();
			SVP_INFO("SVPPcmPlayer PcmInit  channel:%d|rate:%d ", mediaInfo.channel,mediaInfo.rate);
			ret = SVPSingleton<SVPPcmPlayer>::getInstance()->PcmInit(mediaInfo.channel, mediaInfo.rate);
			if (!ret)
			{
				mediaInfo.Clear();//再次初始化
				continue;
			}
		}
		//播放
		pack_playpos = 0;
		int unplay = len - pack_playpos;
		while( unplay > 0 )
		{
			if (!m_bPlayStatus || !m_stCurrMediaInfo.Valid())
			{
				usleep(200000);
				continue;
			}
			/* 注意这两个条件的先后顺序不能调整 */
			if( songid != m_stCurrMediaInfo.id )
			{
				//新曲
				songid = m_stCurrMediaInfo.id;
				byte_playpos = 0;
				playpos = 0;
				break;
			}
			unplay > sizepersecond ? pack_playsize = sizepersecond : pack_playsize = unplay;
			ret = SVPSingleton<SVPPcmPlayer>::getInstance()->PlayPcmData(buffer + pack_playpos,pack_playsize );
			pack_playpos += pack_playsize;
			unplay = len - pack_playpos;
			byte_playpos += pack_playpos;/*播放的总大小*/

			playpos = byte_playpos/sizepersecond;
			if( last_playpos !=  playpos)
			{
				if (m_stCurrMediaInfo.playpos > m_stCurrMediaInfo.id3.duration || byte_playpos == m_PCMData.totalLen)
					m_stCurrMediaInfo.playpos = m_stCurrMediaInfo.id3.duration;
				if (playpos != m_stCurrMediaInfo.playpos)
				{
					m_stCurrMediaInfo.playpos++;
					NotifyStatusChange(QQ_Notify_PlayPosChanged);
				}
				last_playpos = playpos;
			}
		}

		if (len <= 0 && m_PCMData.totalLen == m_PCMData.copyedLen)  //播放结束，自动下一曲
		{
			if (m_playMode == QQ_RepeatSingle)
				PlaySong(m_stCurrMediaInfo.id);
			else
				Op(QQ_OP_NEXT);
			usleep(300000);
		}
	}
	delete buffer;
}

void CQQDataMgr::Op(int opType) {
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	switch (opType) {
	case QQ_OP_PLAY: {
		m_bPlayStatus = true;
		NotifyStatusChange(QQ_Notify_PlayPauseUpdate);
	}
		break;
	case QQ_OP_PAUSE: {
		m_bPlayStatus = false;
		NotifyStatusChange(QQ_Notify_PlayPauseUpdate);
	}
		break;
	case QQ_OP_NEXT:
	case QQ_OP_PREVIOUS: {
		if (m_vPlayList.empty())
			return;
		int idx = GetItemFromPlaylist(m_stCurrMediaInfo.id);
		if (idx == -1)
			return;
		else if (opType == QQ_OP_NEXT)
			idx++;
		else if (opType == QQ_OP_PREVIOUS)
			idx--;
		if (idx < 0)
			idx = m_vPlayList.size() - 1;
		else if (idx >= m_vPlayList.size())
			idx = 0;
		PlaySong(m_vPlayList[idx].id);
	}
		break;
	}
}

void CQQDataMgr::PlaySong(const std::string& songid)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	m_stCurrMediaInfo.Clear();
	m_stCurrMediaInfo.id = songid;
	stQQMusicItem item;
	int idx = GetItemFromPlaylist(songid);
	if (idx >= 0)
	{
		m_stCurrMediaInfo.id3 = m_vPlayList[idx];
	}
	SVP_INFO("%s, %s, %s", m_stCurrMediaInfo.id3.name.c_str(),
			m_stCurrMediaInfo.id3.artist.c_str(),
			m_stCurrMediaInfo.id3.album.c_str());
	SendGetMediaInfo();
	//[Notify] ID3更新
	NotifyStatusChange(QQ_Notify_ID3Update);
}

int CQQDataMgr::GetItemFromPlaylist(const std::string& songid)
{
	int idx = -1;
	for (int i = 0; i < m_vPlayList.size(); i++)
	{
		if (m_stCurrMediaInfo.id == m_vPlayList[i].id)
		{
			idx = i;
			break;
		}
	}
	SVP_INFO("GetItemFromPlaylist %d, index %d", m_vPlayList.size(), idx);
	return idx;
}

void CQQDataMgr::SetPlaylistMode(unsigned char mode)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxList);
	m_playMode = mode;
	if (m_vPlayLayers.empty())
		return;
	if (m_playMode == QQ_RepeatAll || m_playMode == QQ_RepeatSingle)
	{
		m_vPlayList = m_vPlayLayers.back().items;
	}
	else if (m_playMode == QQ_RandomAll)
	{
		m_vPlayList.clear();
		srand((unsigned int) time(NULL));
		std::vector<stQQMusicItem> vTempItems = m_vPlayLayers.back().items;
		while(!vTempItems.empty())
		{
			int idx = rand() % vTempItems.size();
			m_vPlayList.push_back(vTempItems[idx]);
			vTempItems.erase(vTempItems.begin() + idx);
		}
	}
}

void CQQDataMgr::GetMediaInfo(stQQMediaInfo& info)
{
	info = m_stCurrMediaInfo;
}

bool CQQDataMgr::GetId3Pic(std::string& data)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPcmData);
	data.clear();
	if (m_PicData.totalLen == m_PicData.copyedLen)
	{
		data.append(m_PicData.data.data(), m_PicData.data.size());
	}
	return data.size() > 0;
}

void CQQDataMgr::NotifyStatusChange(int id)
{
	if (id == QQ_Notify_InvalidSong)
	{
		//歌曲错误，停止重试，跳转至下一曲
		m_stCurrMediaInfo.querytime = 0;
		m_stCurrMediaInfo.retrycount = 0;
		m_timerNextSong.stop();
		m_timerNextSong.setStartInterval(3000);
		m_timerNextSong.start(Poco::TimerCallback<CQQDataMgr>(*this, &CQQDataMgr::OnNextSongTimer));
	}
	//通知逻辑层
//	CWebAppMgr::GetInstance()->PostEvent( WebAppInterface::WebAppMessage_QQMUSIC_DataUpdate_Notify,id);
}

void CQQDataMgr::OnNextSongTimer(Poco::Timer& timer)
{
	Op(QQ_OP_NEXT);
}
