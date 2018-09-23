#include "LoopCenter.h"
#include "config.h"
#include <unistd.h>

#define   TIMER_PRECISION         ( 50 )

std::map<int,LOOP_NODE>	LoopCenter::m_listTimer;
bool	LoopCenter::isLoopRun = false;

LoopCenter::LoopCenter():m_nodeID(1)
{
	m_listTimer.clear();
	pthread_mutex_init(&m_listMutex,NULL);
}
LoopCenter:: ~LoopCenter()
{
	isLoopRun = false;
	pthread_join(m_looptId, nullptr );
}

void LoopCenter::LoopStart()
{
	int iError = pthread_create(&m_looptId, NULL, LoopFun, this);
//	if(iError)
//		SVP_ERROR("fasfsa");
}
void LoopCenter::LoopExit()
{
	pthread_mutex_lock(&m_listMutex);
	m_listTimer.clear();
	pthread_mutex_unlock(&m_listMutex);
	isLoopRun = false;
	pthread_join(m_looptId, nullptr );
}

bool LoopCenter::isRun()
{
	return isLoopRun;
}

unsigned int LoopCenter::GetID()
{
	return m_nodeID++;
}
unsigned int LoopCenter::RegisterLoopNode( LOOP_NODE& node,bool autoid )
{
    pthread_mutex_lock(&m_listMutex);
    if( autoid  )
    	node.id = GetID();
    node.leftTime = node.interval;
    node.bPause = false;
    m_listTimer[node.id] = node;
//    m_listTimer.insert(node);
    pthread_mutex_unlock(&m_listMutex);
    return node.id;
}
void LoopCenter::DeleLoopNode(unsigned int id)
{
    pthread_mutex_lock(&m_listMutex);
//    std::set<LOOP_NODE,loopNodeSort>::iterator it = m_listTimer.begin();
    std::map<int,LOOP_NODE>::iterator iter;
	iter = m_listTimer.find(id);
	if( iter !=  m_listTimer.end())
		m_listTimer.erase(iter);
    pthread_mutex_unlock(&m_listMutex);
}

bool LoopCenter::SetLoopNode( LOOP_NODE& node )
{
	pthread_mutex_lock(&m_listMutex);
//	std::set<LOOP_NODE,loopNodeSort>::iterator iter;
	std::map<int,LOOP_NODE>::iterator iter;
	iter = m_listTimer.find(node.id);
	if( iter !=  m_listTimer.end())
	{
		iter->second = node;
		return true;
	}
	else
		return false;
	pthread_mutex_unlock(&m_listMutex);
}
LOOP_NODE * LoopCenter::GetLoopNode( LOOP_NODE& node )
{
	pthread_mutex_lock(&m_listMutex);
//	std::set<LOOP_NODE,loopNodeSort>::iterator iter;
	std::map<int,LOOP_NODE>::iterator iter;
	iter = m_listTimer.find(node.id);
	if( iter !=  m_listTimer.end())
		return &iter->second;
	else
		return NULL;
	pthread_mutex_lock(&m_listMutex);
}

void* LoopCenter::LoopFun(void *arg)
{
	LoopCenter* pthis = (LoopCenter*)arg;
	std::map<int,LOOP_NODE>::iterator it;
	LOOP_NODE * pnode;
	isLoopRun = true;
    while ( isLoopRun )
    {
        usleep( TIMER_PRECISION * 1000 );
        pthread_mutex_lock(&pthis->m_listMutex);
//        std::set<LOOP_NODE,loopNodeSort>::iterator it = m_listTimer.begin();
        it = m_listTimer.begin();
        while ( m_listTimer.end() != it )
        {
        	pnode = &it->second;
        	if( ! pnode->bStart)
        	{
        		it++;
        		continue;
        	}
			if ( !pnode->bPause )
			{
				if ( pnode->leftTime <= 0 )
				{
					pnode->leftTime = pnode->interval;
					SVP_INFO("run pnode->fun(pnode->arg);");
					pnode->fun(pnode->arg);
				}
				else
				{
					pnode->leftTime -= TIMER_PRECISION;
				}
			}
			else
			{
				if ( -1 != pnode->nPauseLeftTime ) //parasoft-suppress MISRA2008-0_1_2_j "unsigned int 不能为零,暂时不改"
				{
					if ( pnode->nPauseLeftTime <= 0 )
					{
						pnode->bPause = false;
					}
					else
					{
						pnode->nPauseLeftTime -= TIMER_PRECISION;
						if ( -1 == pnode->nPauseLeftTime ) //parasoft-suppress MISRA2008-0_1_2_j "unsigned int 不能为零,暂时不改"
						{
							pnode->nPauseLeftTime = 0;
							pnode->bPause = false;
						}
					}
				}
			}
            it++;
        }
        pthread_mutex_unlock(&pthis->m_listMutex);
    }
	pthread_exit((void *)0);
    return NULL;
}
