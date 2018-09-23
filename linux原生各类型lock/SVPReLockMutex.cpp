
#include "SVPReLockMutex.h"

/*
	note: PTHREAD_MUTEX_RECURSIVE  
*/

SVPReLockMutex::SVPReLockMutex() 
{
	pthread_mutex_init(&m_mutex, NULL);
}

SVPReLockMutex::SVPReLockMutex(SELECTMUTEX_TYPE type)
{
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
    switch(type)
    {
		case MUTEX_NORMAL:
			pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_NORMAL);
  			break;
	 	case MUTEX_ERRORCHECK:
			pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
			break;
		case MUTEX_RECURSIVE:
			pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
			break;
		case MUTEX_DEFAULT:
			pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_DEFAULT);
			break;
		default:
			pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_DEFAULT);
			break;
        }
	pthread_mutex_init(&m_mutex, &mattr);
	pthread_mutexattr_destroy(&mattr);
}

SVPReLockMutex::~SVPReLockMutex() 
{
	pthread_mutex_destroy(&m_mutex);
}

 pthread_mutex_t* SVPReLockMutex::GetMutex()
 {
    return &m_mutex;
 }

SVPBool SVPReLockMutex::Lock()
{
    if( pthread_mutex_lock(&m_mutex) == 0 )
        return SVP_TRUE;
    else
        return SVP_FALSE;
}

SVPBool SVPReLockMutex::TryLock()
{
    if( pthread_mutex_trylock(&m_mutex) == 0 )
        return SVP_TRUE;
    else
        return SVP_FALSE;
}

SVPVoid SVPReLockMutex::Unlock() 
{
	pthread_mutex_unlock(&m_mutex);
}

