#ifndef __QQDATA_DEF_H__
#define __QQDATA_DEF_H__

#include <vector>
#include <mutex>

class CQQData
{
public:
	int totalLen;
	int copyedLen;
	int packLen;
	int packcopyedLen; //己缓存的当前包数据长度
	int packindex;
	std::string songid;
	std::string data;
	void Clear();
	int CopyData(char* pData, int len);
	int GetData(uint8_t* pData, int len);
};

enum{
	QQMusic_Type_None,
	QQMusic_Type_Song,
	QQMusic_Type_Dir,
	QQMusic_Type_Radio,
};

struct stQQMusicItem
{
	unsigned char type;
	std::string id;
	std::string name;
	std::string artist;
	std::string album;
};

struct stQQMediaInfo
{
	int rate;
	int channel;
	int bit;
	int length;
	std::string id;
	int errorcode;
	bool Valid()
	{
		return rate != 0 && channel != 0 && bit != 0 &&  \
				length != 0 && errorcode == 0;
	}
	void Clear()
	{
		rate = 0;
		channel = 0;
		bit = 0;
		length = 0;
		errorcode = 0;
	}
	bool operator != (const stQQMediaInfo &other)
	{
		return !( this->rate == other.rate && this->channel == other.channel && this->bit == other.bit);
	}
};

class CQQMusicList
{
public:
	std::string parentid;
	std::string parentname;
	int pageindex;
	int count;
	int errorcode;
	std::vector<stQQMusicItem> items;
	bool IsDownloadComplete();
};

#endif
