#include "QQData.h"
#include "SVPLog.h"
#include <sys/time.h>


CQQData::CQQData()
{
	pthread_cond_init(&condvar,NULL);
	pthread_mutex_init(&writermutex,NULL);
}

CQQData::~CQQData()
{
    pthread_mutex_unlock( &writermutex);
	pthread_mutex_destroy( &writermutex);
	pthread_cond_destroy( &condvar );
}

void CQQData::Clear()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	totalLen = 1;
	copyedLen = 0;
	packLen = 0;
	packcopyedLen = 0;

	packindex = -1;
	readmaxlen = 1024*32;
	songid.clear();
	data.clear();
//	pthread_mutex_lock(&writermutex);
//	pthread_cond_signal( &condvar );
//	pthread_mutex_unlock( &writermutex );
}

int CQQData::CopyData(char* pData, int len)
{
	int pos = 0;
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	int uncopylen = packLen - packcopyedLen;
	uncopylen > len ? pos = len : pos = uncopylen;
	data.append(pData, pos);
	packcopyedLen += pos;
	copyedLen += pos;

	if( packcopyedLen > readmaxlen )
	{
			pthread_mutex_lock(&writermutex);
			pthread_cond_signal( &condvar );
			pthread_mutex_unlock( &writermutex );
	}

	return pos;
}

int CQQData::GetData(uint8_t * pData, int len)
{
//	if (data.size() < 2*1024*1024)
//	{
		pthread_mutex_lock(&writermutex);
		readmaxlen = len;
		struct timespec outtime;
		struct timeval now;
		gettimeofday(&now, NULL);
//		now.tv_sec += 1;
		now.tv_usec += 400000;
		outtime.tv_nsec = now.tv_usec;
		outtime.tv_sec = now.tv_sec;
		pthread_cond_timedwait( &condvar,&writermutex ,&outtime);
		pthread_mutex_unlock( &writermutex );
//	}

	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	if (data.size() < len)
		len = -1;
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


