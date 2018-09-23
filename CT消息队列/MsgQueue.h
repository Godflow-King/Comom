///////////////////////////////////////////////////////////
//  MsgQueue.h
//  Implementation of the Class MsgQueue
//  Created on:      03-ÁùÔÂ-2017 13:35:41
//  Original author: uida3992
///////////////////////////////////////////////////////////

#if !defined(EA_497875F2_E642_4c6d_977F_A43F54761000__INCLUDED_)
#define EA_497875F2_E642_4c6d_977F_A43F54761000__INCLUDED_

#include <sys/msg.h>
#include <pthread.h>

#include "tagNotifyType.h"
#include "Event.h"
#include "Mutex.h"

#include "SVPLog.h"
#ifdef PC_TEST
#define PRINTF(fmt,args...)        printf(fmt,##args)
#else
#define PRINTF(fmt,args...)        SVP_INFO(fmt,##args)
#endif

#define LARGE_VALUE_MSG




template<class T>
class MsgQueue
{

public:
	map<long int,long int> m_msgTypeMap;


	MsgQueue(int handleKey,int rcvMsgType)
	{
		m_rcvMsgType = rcvMsgType;
		m_notifyListener = NULL;
		m_largeValueMsgNotifyListener = NULL;
		pthread_mutex_init(&m_msgQueueLock,0);
		pthread_mutex_init(&m_largeValueMsgQueueLock,0);
		CreateMsgQueue(handleKey);
	}
	
	virtual ~MsgQueue()
	{
		msgctl(m_msgHandle,IPC_RMID,0);
	}
	
	
	void MsgSnd(T notify,long int msgType)
	{
		msg_st msgValue;
		msgValue.msg_type = msgType;   //write
		memcpy(&msgValue.notify,&notify,sizeof(T));			
		msgsnd(m_msgHandle,(void*)&msgValue,sizeof(T),0);
	}
	

	void MsgRecv()
	{
		msg_st msgValue;
		while(true)
		{
			msgrcv(m_msgHandle,(void*)&msgValue,sizeof(T),m_rcvMsgType,0);
			//SVP_INFO("MsgQueue::MsgRecv msgValue.msg_type = %d,msgValue.notify = %x\n",msgValue.msg_type,msgValue.notify);
			pthread_mutex_lock(&m_msgQueueLock);
			m_msgQueue.push(msgValue);
			m_msgTypeMap.insert(make_pair(msgValue.msg_type,msgValue.msg_type));
			pthread_mutex_unlock(&m_msgQueueLock);		

#ifdef LARGE_VALUE_MSG
			pthread_mutex_lock(&m_largeValueMsgQueueLock);
			if(m_largeValueMsgNotifyListener)
				m_largeValueMsgQueue.push(msgValue);
			pthread_mutex_unlock(&m_largeValueMsgQueueLock);	
#endif
		}
	}
	
	void MsgCallback()
	{
		msg_st msgValue;
		while(true)
		{
			
			usleep(1000);
			pthread_mutex_lock(&m_msgQueueLock);
			if(m_msgQueue.size() != 0)
			{
				msgValue = m_msgQueue.front();
				m_msgQueue.pop();			
			}
			else{
				pthread_mutex_unlock(&m_msgQueueLock);
				continue;
			}
			pthread_mutex_unlock(&m_msgQueueLock);	
			
			if(m_notifyListener)
			{
				//SVP_INFO("MsgQueue::MsgCallback Listener start\n");
				m_notifyListener->Listener(msgValue.notify,msgValue.msg_type);	
				//SVP_INFO("MsgQueue::MsgCallback Listener end\n");
			}


		}
	}


	void LargeValueMsgCallback()
	{
		msg_st msgValue;
		while(true)
		{
			
			usleep(1000);
			pthread_mutex_lock(&m_largeValueMsgQueueLock);
			if(m_largeValueMsgQueue.size() != 0)
			{
				msgValue = m_largeValueMsgQueue.front();
				m_largeValueMsgQueue.pop();			
			}
			else{
				pthread_mutex_unlock(&m_largeValueMsgQueueLock);
				continue;
			}
			pthread_mutex_unlock(&m_largeValueMsgQueueLock);	
			
			if(m_largeValueMsgNotifyListener)
			{
				//SVP_INFO("MsgQueue::LargeValueMsgCallback Listener \n");
				m_largeValueMsgNotifyListener->Listener(msgValue.notify,msgValue.msg_type);	
			}
			else
			{
				//SVP_INFO("MsgQueue::LargeValueMsgCallback m_largeValueMsgNotifyListener = NULL  msg_type:%d\n",msgValue.msg_type);
				break;
			}


		}
	}
	
	void AddListener(NotifyListenerInterface<T>* notifyListener)
	{
		m_notifyListener = notifyListener;
	}

	void AddLargeValueMsgListener(NotifyListenerInterface<T>* notifyListener)           //for temp
	{
		m_largeValueMsgNotifyListener = notifyListener;
	}
	
	void Start()
	{
		pthread_t id;
		pthread_create(&id,NULL,MsgRecvThread,(void*)this);
		pthread_create(&id,NULL,MsgCallbackThread,(void*)this);

#ifdef LARGE_VALUE_MSG
		if(m_largeValueMsgNotifyListener)
			pthread_create(&id,NULL,LargeValueMsgCallbackThread,(void*)this);
#endif
	}

private:
	typedef struct msg_st_
	{
		long int msg_type;
		T notify;
	}msg_st;

	int m_msgHandle;
	queue<msg_st> m_msgQueue;	
	pthread_mutex_t m_msgQueueLock;

///for large value
	queue<msg_st> m_largeValueMsgQueue;	
	pthread_mutex_t m_largeValueMsgQueueLock;


	

	NotifyListenerInterface<T>* m_notifyListener;
	NotifyListenerInterface<T>* m_largeValueMsgNotifyListener;

	long int m_rcvMsgType;


	void CreateMsgQueue(int handleKey)
	{
		m_msgHandle = msgget((key_t)handleKey,0666|IPC_CREAT);
		if(m_msgHandle == -1)
		{
			return;
		}
		
	}	
	
	static void* MsgRecvThread(void* lparam)
	{
		MsgQueue* instance = (MsgQueue*)lparam;
		if(instance)
			instance->MsgRecv();	
			
		return NULL;
	}
	
	static void* MsgCallbackThread(void* lparam)
	{
		MsgQueue* instance = (MsgQueue*)lparam;
		if(instance)
			instance->MsgCallback();	
			
		return NULL;
	}

	static void* LargeValueMsgCallbackThread(void* lparam)
	{
		MsgQueue* instance = (MsgQueue*)lparam;
		if(instance)
			instance->LargeValueMsgCallback();	
			
		return NULL;
	}
	
};
#endif // !defined(EA_497875F2_E642_4c6d_977F_A43F54761000__INCLUDED_)
