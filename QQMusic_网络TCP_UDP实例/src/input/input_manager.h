#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include <string>

#define INPUT_TYPE_STDIN        0
#define INPUT_TYPE_TOUCHSCREEN  1


typedef struct InputEvent
{
	struct timeval tTime;
	int iType;  /* stdin, touchsceen */
	int iVal;   /*  */
}T_InputEvent, *PT_InputEvent;


typedef struct InputOpr
{
	std::string name;
//	pthread_t tTreadID;
	int (*DeviceInit)(void);
	int (*DeviceExit)(void);
//	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
//	struct InputOpr *ptNext;
}T_InputOpr, *PT_InputOpr;

PT_InputOpr GetInputOpr();


#endif /* _INPUT_MANAGER_H */

