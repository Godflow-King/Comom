/*
 * SVPMain.cpp
 *
 *  Created on: 2018年1月31日
 *      Author: touch
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <memory>
#include "NetMgr/DataMgr.h"
#include "SVPPcmPlayer.h"
#include "config.h"
#include <pulse/simple.h>

int ConnectDevice()
{
	bool ret = DataMgr::GetInstance()->Init();
	if( !ret )
	{
		SVP_INFO(" DataMgr init Error ");
		return -1;
	}
	DataMgr::GetInstance() ->SetConnectStatus(DataMgr::QQ_Connecting);
	while(DataMgr::GetInstance()->GetConnectStatus() != DataMgr::QQ_Connected)
	{
		usleep(500000);
	}
	return 0;
}

int main( int argc, char *argv[] )
{
	if( ConnectDevice() )
		return -1;
	int selectid = -1;
	while(true)
	{
		std::vector<stQQMusicItem> items;
		while( ! DataMgr::GetInstance()->GetMusicList(items) )
		{
			SVP_INFO("Get List Waiting");
			usleep(1000000);
		}
		for (int i = 0; i < items.size(); i++)
		{
			SVP_INFO("id: %d, type: %d, id %s, name %s, artist %s, album %s",
					i, items[i].type, items[i].id.c_str(), items[i].name.c_str(),
					items[i].artist.c_str(), items[i].album.c_str());
		}
		printf("请选择序号, 如返回上一级： -1\n");
		scanf("%d", &selectid);
		if( -2 == selectid )
		{
			sleep(2);
			DataMgr::GetInstance()->Exit();
			if( ConnectDevice() )
				return -1;
			continue;
		}
		DataMgr::GetInstance()->SelectListItem(selectid);
	}
	return 0;
}



//#define PCM_PLAYE_SIZE   256*1024
//
//int main( int argc, char *argv[] )
//{
//	uint8_t buffer[PCM_PLAYE_SIZE];
//	FILE *WriterOSfp;
//    if( (WriterOSfp = fopen("test.wav", "rb")) == NULL)
//    {
//        SVP_INFO("gyh--------------- can't open source file" );
//        return -1;
//    }
//
//    if (!SVPSingleton<SVPPcmPlayer>::getInstance()->PcmInit(2, 44100))
//    {
//    	SVP_INFO("PcmInit Error");
//    	return -1;
//    }
//
//    uint8_t  *p_start ;
//    uint8_t  *p_pos = p_start;
//    while(true)
//    {
//    	int wsize = fread(  buffer, 1 , PCM_PLAYE_SIZE ,  WriterOSfp);
//    	if( wsize > 0 )
//    	{
//    		bool ret = SVPSingleton<SVPPcmPlayer>::getInstance()->PlayPcmData(buffer,wsize );
//			if(!ret)
//				usleep(100*1000);
//    	}
////    	usleep(100*1000);
//    }
//
//	return 0;
//}



