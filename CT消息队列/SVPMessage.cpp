//============================================================================
// Name        : SVPMsg.cpp
// Author      : westchou
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include "SVPMessage.h"

#include "SVPPlatform.h"
#include "SVPTime.h"
/*discard sem 5-0*/
//#include "SVPSemaphore.h"

#include "SVPLog.h"

#ifdef SVP_LOG_TAG
	#undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG		"libbasic|message"

#define MAXMSGFINDTIME  3000
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
	#define INVALIDHAND 0
#else
	#define INVALIDHAND -1

	/*discard sem 5-1*/
	#define BUFSIZE 12/*20*/ //unsigned int * 2 + long /*+ int *2*/
	#define EXITMSGID 0
	#define RECEIVEMSGTYPE 0

	struct SVPTypeMsgSt
	{
		long MsgStType;
		unsigned int msgID;
		unsigned int wParam;
		long lParam;
		/*discard sem 5-2*/
//		SVPBool bSync;
//		SVP_HANDLE hSem;
	};

	struct SVPTypeThreadParameter
	{
		SVP_HANDLE msgHandle;
		MsgProc msgProc;
	};
#endif

#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
#include <windows.h>
#else
#include <errno.h>
#include <stdio.h>
#include <sys/msg.h>

#include "SVPThread.h"

static std::map<SVP_HANDLE,SVPThread*> g_s_mapMsg;

SVPVoid* MsgThreadProc(SVPVoid* pThreadParameter)
{
	SVPTypeThreadParameter* pStThreadParameter = (SVPTypeThreadParameter*)pThreadParameter;

	SVPTypeMsgSt stRcvMsg;

	while(true)
	{
		if(msgrcv(pStThreadParameter->msgHandle,(void*)&stRcvMsg,BUFSIZE,RECEIVEMSGTYPE,0) != -1)
		{
			if(EXITMSGID != stRcvMsg.msgID)
			{
				pStThreadParameter->msgProc(pStThreadParameter->msgHandle, stRcvMsg.msgID, stRcvMsg.wParam, stRcvMsg.lParam);
				/*discard sem 5-3*/
//				if(stRcvMsg.bSync)
//				{
//					SVPSemaphore::SemaphoreV(stRcvMsg.hSem);
//					SVP_INFO("SemaphoreV(hSem):%d-MsgId:%d", stRcvMsg.hSem, stRcvMsg.msgID);
//				}
			}
			else
			{
				SVP_INFO("MsgThreadProc Exit Normal");
				break;
			}
		}
		else
		{
				SVP_ERROR("msgrcv:%d failed with error:%s", pStThreadParameter->msgHandle,strerror(errno));
				break;
		}

	}

	free(pStThreadParameter);
	pStThreadParameter = SVP_NULL;

	return SVP_NULL;
}

#endif

/**
@brief:  Create MsgQueue,same queue only can be created once
@param:	 param3 only need in windows
@retrun: NA
**/
SVP_HANDLE SVPMessage::CreateMsg(MsgFlag msgFlag,MsgProc msgProc,SVPVoid* hInstance)
{
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)msgProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = (HINSTANCE)hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = msgFlag;

	::RegisterClass(&wc);
	
	return ::CreateWindow(msgFlag, msgFlag, WS_POPUP, 0, 0, 0, 0, ::GetDesktopWindow(), (HMENU)0, (HINSTANCE)hInstance, NULL);
#else
	SVP_UnUsed(hInstance);

	/* if msq exist, don't care!,just use when app start*/
	//SVP_HANDLE msgHandle = msgget((key_t)msgFlag,0666);
	SVP_HANDLE msgHandle = INVALIDHAND;

	if(msgHandle == INVALIDHAND)
	{
		msgHandle = msgget((key_t)msgFlag,0666|IPC_CREAT);
		SVP_INFO("msgget IPC_CREAT");

		if(msgHandle == INVALIDHAND)
		{
			SVP_ERROR("msgget:%d failed with error:%s",msgFlag,strerror(errno));
		}
		else
		{

			std::map<SVP_HANDLE,SVPThread*>::iterator itrMapMsg = g_s_mapMsg.find(msgHandle);
			if(itrMapMsg == g_s_mapMsg.end())
			{

				SVP_INFO("new msgHandle");
				SVPThread* pMsgThread = new SVPThread();

				std::pair<SVP_HANDLE,SVPThread*> pairMsg;
				pairMsg.first = msgHandle;
				pairMsg.second = pMsgThread;
				g_s_mapMsg.insert(pairMsg);

				SVPTypeThreadParameter* pStThreadParameter = (SVPTypeThreadParameter*)malloc(sizeof(SVPTypeThreadParameter));
				if(pStThreadParameter != SVP_NULL)
				{
					pStThreadParameter->msgHandle = msgHandle;
					pStThreadParameter->msgProc = msgProc;
					pMsgThread->SetPriority(THREAD_PRIORITY_HIGHEST);
					pMsgThread->Start(MsgThreadProc,(SVPVoid*)pStThreadParameter);
				}
			}
			else
			{
				SVP_INFO("msgHandle has exist in map");
			}
		}
	}
	else
	{
		SVP_ERROR("msgqueue exist");
	}

	return msgHandle;
#endif
}

/**
@brief:  Find MsgQueue
@param:	 NA
@retrun: NA
**/
SVP_HANDLE SVPMessage::FindMsg(MsgFlag msgFlag,SVPBool bWaitTimeout)
{
	SVP_HANDLE msgHandle = INVALIDHAND;

	SVPInt32 nTick = SVPTime::GetTickCount();

	while(true)
	{		
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
		msgHandle = ::FindWindow(NULL, msgFlag);
#else
		msgHandle = msgget((key_t)msgFlag,0666);
#endif

		if(msgHandle == INVALIDHAND)
		{
			if(SVPTime::GetTickCount() - nTick > MAXMSGFINDTIME || !bWaitTimeout)
			{
				break;
			}
		}
		else
		{
			break;
		}


		SVP_Sleep(100);
	}

	return msgHandle;
}

/**
@brief:  Post Msg
@param:	 NA
@retrun: NA
**/
SVPBool SVPMessage::PostMsg(SVP_HANDLE hMsg, SVPUint32 iMsg,SVPUint32 wParam, SVPLong lParam,SVPBool bSync,SVP_HANDLE hSem)
{
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
	return ::PostMessage((HWND)hMsg,iMsg,wParam,lParam);
#else
	SVPTypeMsgSt stSndMsg;
	stSndMsg.MsgStType = 1;
	stSndMsg.msgID = iMsg;
	stSndMsg.wParam = wParam;
	stSndMsg.lParam = lParam;
	/*discard sem 5-4-0*/
//	stSndMsg.bSync = bSync;
//	stSndMsg.hSem = hSem;

	SVPBool bRet = SVP_FALSE;
	if(hMsg != INVALIDHAND)
	{
		if(msgsnd(hMsg,(SVPVoid*)&stSndMsg,BUFSIZE,0) != -1)
		{
			/*discard sem 5-4-1*/
//			/*SYNC*/
//			if(bSync && stSndMsg.msgID != EXITMSGID)
//			{
//				SVP_INFO("SYNC SemaphoreP(hSem) : %d",hSem);
//				SVPSemaphore::SemaphoreP(hSem,10000);
//			}

			bRet = SVP_TRUE;
		}
		else
		{
			SVP_ERROR("msgsnd failed with error:%s",strerror(errno));
		}
	}

	return bRet;
#endif
}

/**
@brief:  Post Msg depend  by MsgFlags,if MsgFlag not exist,return soon
@param:	 NA
@retrun: NA
**/
SVPBool SVPMessage::PostMsgEx(MsgFlag msgFlag, SVPUint32 iMsg,SVPUint32 wParam, SVPLong lParam,SVPBool bSync,SVP_HANDLE hSem)
{
	SVP_INFO("PostMsgEx - MsgFlag:%d iMsg:%d",msgFlag,iMsg);
	//FindMsg when PostMsgEx,dosen't to wait
	return PostMsg(FindMsg(msgFlag,SVP_FALSE),iMsg,wParam,lParam,bSync,hSem);
}

/**
@brief:  Destroy MsgQueue
@param:	 NA
@retrun: NA
**/
SVPBool SVPMessage::DestroyMsg(SVP_HANDLE hMsg)
{
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
	return ::DestroyWindow((HWND)hMsg);
#else
	PostMsg(hMsg,EXITMSGID);

	std::map<SVP_HANDLE,SVPThread*>::iterator itrMapMsg = g_s_mapMsg.find(hMsg);
	if(itrMapMsg != g_s_mapMsg.end())
	{
		SVP_INFO("pMsgThread->WaitQuit()");
		SVPThread* pMsgThread = itrMapMsg->second;
		if(pMsgThread != SVP_NULL)
		{
			pMsgThread->WaitQuit();
			
			delete pMsgThread;
			pMsgThread = SVP_NULL;
		}
		g_s_mapMsg.erase(itrMapMsg);
	}
	else
	{
		SVP_ERROR("SVPThread Exit failed.");
	}

	if(msgctl(hMsg,IPC_RMID,0) == -1)
	{
		SVP_ERROR("msgctl(IPC_RMID) failed.");
	}
	else
	{
		SVP_INFO("msgctl(IPC_RMID) success");
	}

	return SVP_TRUE;
#endif
}
