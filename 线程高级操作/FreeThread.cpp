#include "common/FreeThread.h"
#include <stdio.h>
FreeThread::FreeThread(  )
{
	pthread_attr_init(&m_attr);
	pthread_attr_setinheritsched(&m_attr, PTHREAD_INHERIT_SCHED);/*默认继承父类的所有属性*/
}
FreeThread:: ~FreeThread(  )
{
	Termina();/* 即使该线程已经停止，也就再发一次信号，大不了pthread_cancel返回非0值 */
	pthread_attr_destroy(&m_attr);
}

/*
 * 1.对于JOINABLE未分离的线程，pthread_join会堵塞，直到线程消亡,该函数返回后,线程回收资源
 * 2.对于DETACHED已经分离的线程，pthread_join会立刻返回，无论该线程是否消亡，靠pthread_exit((void *)0);回收资源
 */
void FreeThread::WaitQuit()
{
	if( m_thread == 0 )
		return;
	if( m_isDetach == false)/*未分离的线程*/
	{
		pthread_join(m_thread, NULL );
		m_thread = (pthread_t)0;
	}
//	pthread_join(m_thread, NULL );/*对于分离的线程，调用了，也会立刻返回，无法控制线程的状态*/

		/*void *tret = NULL;
		*pthread_join(m_thread, &tret );
		*据说tret = 返回值，他是一个void*
		*然而大多数情况不关心 返回值
		* pthread_join 返回后，再调用pthread_join，立即返回*/
}

bool FreeThread::IsRunning()
{
	if( m_thread != 0 )
		return true;
	else
		return false;
}

/*
 *函数例子：*/
//static void errorfun( void *pdata )
//{
//	SVP_INFO( " error handle " );
//}
//static void* mainThreadFun(void* pdata)
//{
//	int *pIn32data = ( int * )pdata;
//	pthread_cleanup_push(errorfun, NULL);
//
//	int a = 4 ,b = 0;
//	while(true)
//	{
//		sleep( 2 );
//		SVP_INFO( "heartbeat ...%d",*pIn32data );
//		b = 5 / 0;
//		pthread_testcancel();/* 设置取消点 */
//	}
//	pthread_exit((void *)0);//必要的
//	 pthread_cleanup_pop(0);
//	return NULL;
//}

int FreeThread::thStart( void *(*start_routine) (void *) ,void * pthreadparam)
{
	int ret = 0;
//	threadfunptr = funptr;
	ret = pthread_create(&m_thread, &m_attr, start_routine, pthreadparam);
	return ret;
}

void FreeThread::Termina()
{
	/* 线程未开启 ,或者已经停止*/
	if( m_thread == 0 )
		return;
	pthread_cancel( m_thread );/* 判断返回值的意义不大 */
	/* 缺省线程打开取消功能，收到信号后继续运行至下一个取消点再退出 */
	if( m_isDetach )
		m_thread = (pthread_t)0;
	else
		WaitQuit();/*等退出回收资源*/
}

int FreeThread::SetIsCancelEnable( bool enable )
{
	int ret;
	if( enable )
	{
		ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);/* 缺省:打开 ,NULL 不关心原来的状态*/
	}
	else
	{
		ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	}
	return ret;
}

/*
 * PTHREAD_CANCEL_DEFERRED     缺省:线程收到信号后继续运行至下一个取消点再退出
 * PTHREAD_CANCEL_ASYCHRONOUS    立即执行取消动作（退出）
 *
 *
 * 	pthreads标准指定了几个取消点，其中包括：
 * 	(1)通过pthread_testcancel调用以编程方式建立线程取消点。
 *		(2)线程等待pthread_cond_wait或pthread_cond_timewait()中的特定条件。
 *		(3)被sigwait(2)阻塞的函数
 *		(4)一些标准的库调用。通常，这些调用包括线程可基于阻塞的函数。
 *		根据POSIX标准，pthread_join()、pthread_testcancel()、pthread_cond_wait()，pthread_cond_timedwait()，
 *		sem_wait()、sigwait()等函数以及read()、write()等会引起阻塞的系统调用都是Cancelation-point。
 *
 */
int FreeThread::SetCancelType( int  type )
{
	int ret;
	ret = pthread_setcanceltype(type, NULL);
	return ret;
}

int FreeThread::SetIsDetachMode( bool isdetach )
{
	/* 或者创建者create后 pthread_detach(m_thread)，或者线程自己pthread_detach(pthread_self())
	 * 最后 在 threadfun 函数末尾 pthread_exit((void *)0);   一样的效果
	 */
	int error = 0;
	if( m_thread == 0 )
	{
		printf("thread is not start \n");
		if( isdetach == true )
			error = pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_DETACHED);//分离态
		else
			error = pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_JOINABLE);
		if( error )
		{
			printf("pthread_attr_setdetachstate\n");
		}
	}
	else
	{
		if( isdetach == true )
			error = pthread_detach(m_thread);
	}
	m_isDetach = isdetach;
	return  error;
}

/*
 * 先进先出（SCHED_FIFO）、轮转法（SCHED_RR）,或其它（SCHED_OTHER）
 */
bool FreeThread::SetSchedPolicy( int policy )
{
	int ret = 0;
	if( m_thread == 0 )/* 未开启之前，才能设置 */
		return false;
	ret = pthread_attr_setinheritsched(&m_attr, PTHREAD_EXPLICIT_SCHED);
	ret |= pthread_attr_setschedpolicy(&m_attr,policy);
	if ( ret != 0 )
	{
		printf(" SetSchedPolicy Error \n ");
		return false;
	}
	return true;
}

bool FreeThread::SetPriority( float dvi )
{
	int ret = 0;
	if( m_thread == 0 )/* 未开启之前，才能设置 */
		return false;
	struct sched_param sched;
	int CurSchedPolicy = 0;
	pthread_attr_getschedpolicy(&m_attr,&CurSchedPolicy);
	sched.sched_priority = (sched_get_priority_min(CurSchedPolicy) + sched_get_priority_max(CurSchedPolicy)) * dvi;
	ret = pthread_attr_setinheritsched(&m_attr, PTHREAD_EXPLICIT_SCHED);
	ret |= pthread_attr_setschedparam(&m_attr, &sched);
	if ( ret != 0 )
	{
		printf(" SetPriority Error \n ");
		return false;
	}
	return true;
}
