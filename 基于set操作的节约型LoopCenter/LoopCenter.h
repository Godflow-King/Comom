#ifndef  SVP_LOOP_CENTER_H_
#define  SVP_LOOP_CENTER_H_

#include <list>
#include <set>
#include <map>
#include <pthread.h>

typedef void (*TIMER_CMD)( void * );
typedef struct loopNode
{
	bool           bStart;
    bool           bPause;
    unsigned int   nPauseLeftTime;
    unsigned int   interval;
    TIMER_CMD      fun;
    void*          arg;
    unsigned int   id;
    unsigned int   leftTime;
} LOOP_NODE;

///*“仿函数"。为Student set指定排序准则*/
//class loopNodeSort
//{
//    public:
//        bool operator() (const LOOP_NODE &a, const LOOP_NODE &b) const
//        {
//            return (a.id < b.id);
//        }
//};

class LoopCenter
{
public:
	static LoopCenter* GetInstance()
	{
		static LoopCenter instance;
	        return &instance;
	}
	unsigned int GetID();
	void LoopStart();
	void LoopExit();
	unsigned int RegisterLoopNode( LOOP_NODE& node,bool autoid = false );
	void DeleLoopNode(unsigned int id);
	bool isRun();
	bool SetLoopNode( LOOP_NODE& node );
	LOOP_NODE * GetLoopNode( LOOP_NODE& node );

private:
	LoopCenter();
	virtual ~LoopCenter();
	LoopCenter(const LoopCenter &other) {};
	LoopCenter &operator = (const LoopCenter &other) {};
	static void* LoopFun(void *arg);
private:
    pthread_t	m_looptId;
//    static std::map<LOOP_NODE,loopNodeSort>	m_listTimer;
	static std::map<int,LOOP_NODE>	m_listTimer;
    unsigned int m_nodeID;
    pthread_mutex_t             m_listMutex;
    static bool	isLoopRun;
};

class LTimer
{
public:
	LTimer()
	{

	};
	LTimer(void *pdata,TIMER_CMD fun,unsigned int interval = 1000)
	{
		Set(pdata,fun, interval );
	};
	virtual ~LTimer()
	{
		Exit();
	};

	void Set(void *pdata,TIMER_CMD fun,unsigned int interval = 1000)
	{
		loop_node.arg = pdata;
		loop_node.fun = fun;
		loop_node.interval = interval;
		loop_node.id = LoopCenter::GetInstance()->GetID();
	}

	void Start()
	{
		if(!LoopCenter::GetInstance()->isRun())
			LoopCenter::GetInstance()->LoopStart();
		loop_node.bStart = true;
		LoopCenter::GetInstance()->RegisterLoopNode(loop_node);
	}
	void Start(unsigned int interval)
	{
		loop_node.interval = interval;
		this->Start();
	}

	void Stop()
	{
		loop_node.bStart =false;
		LoopCenter::GetInstance()->SetLoopNode(loop_node);
	}

	void Exit()
	{
		LoopCenter::GetInstance()->DeleLoopNode( loop_node.id );
	}

	bool isRun()
	{
		return LoopCenter::GetInstance()->GetLoopNode(loop_node)->bStart;
	}

private:
	LOOP_NODE loop_node;
};

#endif
