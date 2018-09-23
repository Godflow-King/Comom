#ifndef __SVP_RELOCKMUTEX_H__
#define __SVP_RELOCKMUTEX_H__

#include "SVPType.h"
#include <pthread.h>

typedef enum
{
	MUTEX_NORMAL = 1,//PTHREAD_MUTEX_NORMAL
	MUTEX_ERRORCHECK,//PTHREAD_MUTEX_ERRORCHECK
	MUTEX_RECURSIVE,// PTHREAD_MUTEX_RECURSIVE
	MUTEX_DEFAULT,//PTHREAD_MUTEX_DEFAULT
} SELECTMUTEX_TYPE;

class SVPReLockMutex
{
public:
	SVPReLockMutex();
	SVPReLockMutex(SELECTMUTEX_TYPE type);
	~SVPReLockMutex();

	SVPBool Lock();
	SVPVoid Unlock();
	SVPBool TryLock();
	pthread_mutex_t * GetMutex();

private:
	pthread_mutex_t m_mutex;
};

#endif
