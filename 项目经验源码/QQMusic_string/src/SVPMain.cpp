#include "QQDataMgr.h"
#include "SVPLog.h"


int main(int argc, char* argv[])
{
	CQQDataMgr::GetInstance()->SetConnectStatus(CQQDataMgr::QQ_Connecting);
	while(CQQDataMgr::GetInstance()->GetConnectStatus() != CQQDataMgr::QQ_Connected)
	{
		usleep(500000);
	}
	sleep(5);
	int selectid = -1;
	bool bIsSong, bListExisted;
	std::string path;
	CQQDataMgr::GetInstance()->SelectListItem(selectid, bIsSong, bListExisted);
	while(true)
	{
		CQQMusicList layer;
		while(!CQQDataMgr::GetInstance()->GetMusicList(layer, path)
				&& CQQDataMgr::GetInstance()->GetConnectStatus() == CQQDataMgr::QQ_Connected)
			sleep(1);
		CQQDataMgr::GetInstance()->SetConnectStatus(CQQDataMgr::QQ_Connecting);

		CQQDataMgr::GetInstance()->GetMusicList(layer, path);
		SVP_INFO("path %s", path.c_str());
		std::vector<stQQMusicItem> items;
		items = layer.items;
		for (int i = 0; i < items.size(); i++)
		{
			SVP_INFO("id: %d, type: %d, id %s, name %s, artist %s, album %s",
					i, items[i].type, items[i].id.c_str(), items[i].name.c_str(),
					items[i].artist.c_str(), items[i].album.c_str());
		}
		printf("请选择序号, 如返回上一级： -1, 下一曲: -2, 上一曲： -3\n");
		scanf("%d", &selectid);
		if (selectid == -2)
		{
			CQQDataMgr::GetInstance()->Op(CQQDataMgr::QQ_OP_NEXT);
		}
		else if (selectid == -3)
		{
			CQQDataMgr::GetInstance()->Op(CQQDataMgr::QQ_OP_PREVIOUS);
		}
		else
		{
			CQQDataMgr::GetInstance()->SelectListItem(selectid, bIsSong, bListExisted);
		}
	}
	return 0;
}

