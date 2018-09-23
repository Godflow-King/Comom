#ifndef  SVP_GY_FREE_THREAD_H_
#define  SVP_GY_FREE_THREAD_H_

#include <pthread.h>

//typedef  void* (*FREETHREADFUN)( void * );

class FreeThread
{
public:
	FreeThread();
	virtual ~FreeThread();

	int 	SetIsCancelEnable( bool enable );
	int 	SetCancelType( int  type );
	int SetIsDetachMode( bool isdetach );
	bool SetSchedPolicy( int policy );
	bool SetPriority( float dvi ); /* 0 ~ 1 */

//	int thStart( FREETHREADFUN  funptr ,void * pthreadparam);
	int thStart( void *(*start_routine) (void *) ,void * pthreadparam);
	void Termina();//别的线程可以强势终止该线程;
	void WaitQuit();

	bool IsRunning();

//public :
//	static void* threadInlineFun(void* pdata);
//	static FREETHREADFUN threadfunptr;

private:
	pthread_t		m_thread = (pthread_t) 0;
	pthread_attr_t 	m_attr;
	bool m_isDetach = false;
};

#endif
