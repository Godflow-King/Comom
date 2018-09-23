#include <pthread.h>
#include "NetMgr/DataMgr.h"
#include "config.h"
#include "json/src/Utils.h"
#include "json/src/reader.h"
#include "json/src/writer.h"
//#include "SVPSingleton.h"
#include "SVPPcmPlayer.h"

#define WLAN_DEVICE_NAME "eth0" //"wlan0"  "eth0"
#define PCM_BUFFER_SIZE 4194304 //4*1024*1024
#define PCM_PLAY_BUFFER 4096

DataMgr::DataMgr()
: m_connectStatus(QQ_Disconnect)
, m_BroadPort(43921)
,m_LocalDataPort(43997)
,m_LocalCmdPort(43955)
,m_LocalResultPort(43959)
{
	m_BroadIpAddr = "255.255.255.255";
	m_PCMData.Clear();
	m_PicData.Clear();
}
DataMgr::~DataMgr()
{
	NetMgr::GetInstance()->ExitAllTread();
	SVP_INFO("~DataMgr");
}
void DataMgr::Exit()
{
	m_ispcmThreadRun = false;
	NetMgr::GetInstance()->ExitAllTread();
}
bool DataMgr::Init()
{
	m_PCMData.Clear();
	m_PicData.Clear();
	m_stCurrMediaInfo.Clear();
	bool ret = NetMgr::GetInstance()->InitAndStart(this,m_LocalDataPort,m_LocalCmdPort,m_LocalResultPort);
	if(!ret)
		return false;
	m_LocalMacAddr = GetLocalMacAddr(WLAN_DEVICE_NAME);
	if( m_LocalMacAddr == "" )
		return false;
	pthread_t m_tid;
	pthread_create(&m_tid, NULL, ThreadPlayPcmData, (void*)this);
	return true;
}

void DataMgr::OnNetState(NETMGR_STATE state )
{
	switch(state)
	{
	case Net_TCP_CONNECT:
		SetConnectStatus(QQ_Connected);
		break;
	case Net_TCP_DISCONNECT:
		SetConnectStatus(QQ_Disconnect);
		m_ispcmThreadRun = false;
		break;
	case Net_UDP_EXIT:
		SVP_INFO("Net_UDP_EXIT............");
		break;
	}
}
void DataMgr::OnCmdData(char *pdata ,int size )
{
	static std::string lastStr;
	if (lastStr == pdata)
		return;
	lastStr = pdata;

	Json::Value root = CUtils::LoadJsonFromString(pdata);
	std::string cmdID;
	CUtils::JsonToValue(&root["Request"], cmdID);
	if (cmdID == "CommInfos")
	{
		CUtils::JsonToValue(&root["Arguments"]["CommandPort"], m_RemoteCmdPort);
		CUtils::JsonToValue(&root["Arguments"]["ResultPort"], m_RemoteResultPort);
		SVP_INFO("_UdpReceiveData CommandPort %d, ResultPort %d", m_RemoteCmdPort, m_RemoteResultPort);
		DataMgr::GetInstance()->SelectListItem(-1);
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
void DataMgr::OnResultData(char *pData ,int size )
{
	static std::string lastStr;
	if (lastStr == pData)
		return;
	lastStr = pData;
	Json::Value root = CUtils::LoadJsonFromString(pData);
	if (!root["MobileDeviceInfos"].isNull())
	{
		CUtils::JsonToValue(&root["MobileDeviceInfos"]["Brand"], m_RemoteBrand);
		CUtils::JsonToValue(&root["MobileDeviceInfos"]["Mac"], m_RemoteMacAddr);
	}
	else if (!root["Items"].isNull())
	{
		if (m_vMusicLayers.empty())
			return;
		std::string parentid;
		int pageindex = 0;
		CQQMusicList* pLayer = &m_vMusicLayers.back();
		CUtils::JsonToValue(&root["Items"]["ParentID"], parentid);
		CUtils::JsonToValue(&root["Items"]["PageIndex"], pageindex);
		CUtils::JsonToValue(&root["Items"]["Error"], pLayer->errorcode);
		if (pLayer->errorcode != QQ_Error_OK)
		{
			pLayer->pageindex = 0;
			pLayer->count = pLayer->items.size();
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
			pLayer->items.push_back(item);
		}
		if (arraySize == 0)
			pLayer->count = pLayer->items.size();
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
		if (m_stCurrMediaInfo.Valid())
		{
//			SendGetPicData();
			SendGetPCMData();
		}
	}
}
void DataMgr::OnPCMData(char *pdata ,int size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtxPcmData);
	int pos = m_PCMData.CopyData(pdata, size);
	if (pos == 0)
	{
		m_PicData.CopyData(pdata, size);
	}
	if (size - pos > 0)
	{
		m_DataHead.append(pdata + pos, size - pos);
		ParseDataHead();
	}
}
void DataMgr::ParseDataHead()
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

int DataMgr::GetConnectStatus()
{
	return m_connectStatus;
}
void DataMgr::SetConnectStatus(int status)
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
	}
	else
	{
		m_timerHeartbeat.setStartInterval(0);
		m_timerHeartbeat.setPeriodicInterval(2500);
		m_timerHeartbeat.start(Poco::TimerCallback<DataMgr>(*this, &DataMgr::OnHeartbeatTimer));
	}
}
void DataMgr::OnHeartbeatTimer(Poco::Timer& timer)
{
	if (m_connectStatus == QQ_Connecting)
	{
		SendDiscover();
	}
	else if (m_connectStatus == QQ_Connected)
	{
		SendHeartbeat();
		SendGetPCMData();
	}
}


void DataMgr::SelectListItem(int listid)
{
	int layersize = m_vMusicLayers.size();
	//返回上一级
	if (listid < 0 && layersize > 1)
	{
		m_vMusicLayers.pop_back();
		return;
	}
	//根目录
	CQQMusicList nextLayer;
	nextLayer.pageindex = -1;
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
		return;
	stQQMusicItem selectitem = pLayer->items[listid];
	if (selectitem.type == QQMusic_Type_Song)
	{
		m_stCurrMediaInfo.Clear();
		m_stCurrMediaInfo.id = selectitem.id;
		SendGetMediaInfo();
//		m_vPlayLayers = m_vMusicLayers;
	}
	else
	{
		nextLayer.parentid = selectitem.id;
		nextLayer.parentname = selectitem.name;
		m_vMusicLayers.push_back(nextLayer);
		SendGetList();
	}
}
bool DataMgr::GetMusicList(std::vector<stQQMusicItem>& items)
{
	if (m_vMusicLayers.empty() || !m_vMusicLayers.back().IsDownloadComplete())
		return false;
	items = m_vMusicLayers.back().items;
	return true;
}





/* 各种send函数 */

void DataMgr::SendDiscover()
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

void DataMgr::SendGetPCMData()
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

//	SVP_INFO("==========SendGetPCMData  :  %d",m_PCMData.packindex + 1);
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

void DataMgr::SendGetPicData()
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

void DataMgr::SendDeviceInfos()
{
	std::string jsonStr = std::string("{\"DeviceInfos\":")
	  + "{\"Brand\":\"SGMW\",\"Models\":\"SGMW\",\"Network\":1,\"OS\":\"linux\","
	  + "\"OSVer\":\"G5\",\"PCMBuf\":1048576,\"PICBuf\":1048576,\"Ver\":\"1.0\"}}\r\n";
    UdpBroadcast(m_BroadIpAddr, m_RemoteResultPort, jsonStr);
}

void DataMgr::SendHeartbeat()
{
	std::string jsonStr = "{\"Request\":\"Heartbeat\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void DataMgr::SendDisconnect()
{
	std::string jsonStr = "{\"Request\":\"Disconnect\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void DataMgr::SendGetMobileInfos()
{
	std::string jsonStr = "{\"Request\":\"MobileDeviceInfos\"}\r\n";
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, jsonStr);
}

void DataMgr::SendGetMediaInfo()
{
	std::string jsonStr = "{\"Request\":\"MediaInfo\",\"Arguments\":{\"SongID\":\"%s\"}}\r\n";
	char buffer[256];
	sprintf(buffer, jsonStr.c_str(), m_stCurrMediaInfo.id.c_str());
	UdpBroadcast(m_BroadIpAddr, m_RemoteCmdPort, buffer);
}

void DataMgr::SendGetList()
{
	if (m_vMusicLayers.empty())
		return;
	CQQMusicList* pLayer = &m_vMusicLayers.back();
	SVP_INFO("SendGetList() %d, pageindex %d, itemsize %d, count %d",
			pLayer->IsDownloadComplete(), pLayer->pageindex, pLayer->items.size(), pLayer->count);
	if (pLayer->IsDownloadComplete())
		return;
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
}

void* DataMgr::ThreadPlayPcmData(void *arg)
{
	pthread_detach(pthread_self());
	DataMgr* pthis = (DataMgr*)arg;
	uint8_t buffer[PCM_PLAY_BUFFER] = {0};
	stQQMediaInfo mediaInfo;
	int len = 0,seq = 0;
	bool ret;
	pthis ->m_ispcmThreadRun = true;
	while(pthis->m_ispcmThreadRun)
	{
		pthis->m_mtxPcmData.lock();
		len = pthis->m_PCMData.GetData(buffer, PCM_PLAY_BUFFER);
		pthis->m_mtxPcmData.unlock();
		if (len > 0)
		{
			if( mediaInfo != pthis->m_stCurrMediaInfo )
			{
				mediaInfo = pthis->m_stCurrMediaInfo;
				SVP_INFO(" gyh----------------len : %d > 0 channel:%d rate:%d",len,mediaInfo.channel, mediaInfo.rate);
				SVPSingleton<SVPPcmPlayer>::getInstance()->PcmDrain();
				SVPSingleton<SVPPcmPlayer>::getInstance()->PcmExit();
				ret = SVPSingleton<SVPPcmPlayer>::getInstance()->PcmInit(mediaInfo.channel, mediaInfo.rate);
				if (!ret)
				{
					mediaInfo.bit = 0;
					SVP_ERROR("gyh-----------------PcmInit Error");
				}
			}

			//播放
			ret = SVPSingleton<SVPPcmPlayer>::getInstance()->PlayPcmData(buffer,len );
			if( !ret )
			{
				 SVP_INFO("gyh -------------------Player PlayPcmData Error ");
			}
		}
		else
		{   //数据缓冲中
			usleep(1000000);
			continue;
		}
	}
	SVPSingleton<SVPPcmPlayer>::getInstance()->PcmExit();
	pthread_exit((void *)0);
	return NULL;
}
