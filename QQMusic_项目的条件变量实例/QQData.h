#ifndef __QQDATA_DEF_H__
#define __QQDATA_DEF_H__

#include <vector>
#include <mutex>
#include <pthread.h>

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
	CQQData();
	~CQQData();
	void Clear();
	int CopyData(char* pData, int len);
	int GetData(uint8_t * pData, int len);
//	FILE *WriterOSfp = NULL;
	std::recursive_mutex m_mtx;
	pthread_mutex_t writermutex;
	pthread_cond_t condvar;
	int readmaxlen;
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
	int duration;
};

struct stQQMediaInfo
{
	int rate;
	int channel;
	int bit;
	int length;
	std::string id;
	int errorcode;
	uint32_t querytime;
	int playpos;
	unsigned char retrycount;
	stQQMusicItem id3;
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
		querytime = 0;
		retrycount = 0;
		playpos = 0;
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
	uint32_t querytime;
	std::vector<stQQMusicItem> items;
	bool IsDownloadComplete();
	void Clear()
	{
		pageindex = -1;
		querytime = 0;
		items.clear();
	}
};

#endif
