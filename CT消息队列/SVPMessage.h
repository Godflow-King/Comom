//============================================================================
// Name        : SVPMsg.h
// Author      : westchou
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================
#ifndef __SVP_MSG_H__
#define __SVP_MSG_H__

#include "SVPType.h"


typedef SVPLong (*MsgProc)(SVP_HANDLE hMsg, SVPUint32 message, SVPUint32 wParam, SVPLong lParam);

namespace SVPMessage
{
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
	typedef SVPWChar* MsgFlag;
#else
	typedef SVPInt32 MsgFlag;
#endif

	SVP_HANDLE CreateMsg(MsgFlag msgFlag,MsgProc msgProc,SVPVoid* hInstance = 0);//param3 only need in windows
	SVP_HANDLE FindMsg(MsgFlag msgFlag,SVPBool bWaitTimeout = SVP_TRUE);
	SVPBool PostMsg(SVP_HANDLE hMsg, SVPUint32 iMsg,SVPUint32 wParam = 0, SVPLong lParam = 0,
			SVPBool bSync = SVP_FALSE,SVP_HANDLE hSem = SVP_INVALID_HANDLE_VALUE);
	SVPBool PostMsgEx(MsgFlag msgFlag, SVPUint32 iMsg,SVPUint32 wParam = 0, SVPLong lParam = 0,
			SVPBool bSync = SVP_FALSE,SVP_HANDLE hSem = SVP_INVALID_HANDLE_VALUE);
	SVPBool DestroyMsg(SVP_HANDLE hMsg);
};
#endif //__SVP_MSG_H__
