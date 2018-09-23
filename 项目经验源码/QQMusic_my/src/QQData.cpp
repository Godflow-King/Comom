#include "QQData.h"
#include "config.h"

void CQQData::Clear()
{
	totalLen = 1;
	copyedLen = 0;
	packLen = 0;
	packcopyedLen = 0;
	packindex = -1;
	songid.clear();
	data.clear();
}

int CQQData::CopyData(char* pData, int len)
{
	int pos = 0;
	int uncopylen = packLen - packcopyedLen;
	uncopylen > len ? pos = len : pos = uncopylen;
	data.append(pData, pos);
	packcopyedLen += pos;
	copyedLen += pos;
//	SVP_INFO("CopyData datasize %d, pos %d, packlen %d, packcopyedlen %d, copyedLen %d, totallen %d",
//			data.size(), pos, packLen, packcopyedLen, copyedLen, totalLen);
	return pos;
}

int CQQData::GetData(uint8_t* pData, int len)
{
	if (data.size() < len)
		len = data.size();
	if (len > 0)
	{
		memcpy(pData, data.data(), len);
		data.erase(0, len);
	}
	return len;
}

bool CQQMusicList::IsDownloadComplete()
{
	if (items.size() < count || pageindex == -1)
	{
		return false;
	}
	return true;
}


