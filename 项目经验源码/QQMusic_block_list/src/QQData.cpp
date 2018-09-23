#include "QQData.h"
#include "SVPLog.h"
#include <sys/time.h>
#include <string.h>

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG  "CQQData"

CQQData::CQQData()
{
}

void CQQData::resetblock( pcmblock_pt ptblock )
{
	ptblock->packLen = 0;
	ptblock->packcopyedLen = 0;
	ptblock->packindex = -1;
	ptblock->buffer.clear();
}

CQQData::~CQQData()
{
	m_blockList.clear();
	m_freeList.clear();
	for(int iLoop = 0;iLoop < QQDATA_BLOCK_NUMBER;iLoop ++)
	{
		resetblock( &m_blocks[iLoop] );
	}
}

void CQQData::Clear()
{
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	totalLen = 1;
	copyedLen = 0;
	localsize = 0;
	packindex = -1;
	songid.clear();
	pcur_block = NULL;

	m_blockList.clear();
	m_freeList.clear();
	for(int iLoop = 0;iLoop < QQDATA_BLOCK_NUMBER;iLoop++)
	{
		m_blocks[iLoop].id = iLoop;
		resetblock(&m_blocks[iLoop]);
		m_freeList.push_back(&m_blocks[iLoop]);
	}
}

void CQQData::ChangePack(int index,int len)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	if( pcur_block && (pcur_block->packLen == pcur_block->packcopyedLen) )
		m_blockList.push_back(pcur_block);
	std::list<pcmblock_pt>::iterator itor = m_freeList.begin();
	if( itor ==  m_freeList.end())
	{
		SVP_INFO(" +++++  itor ==  m_freeList.end() ++++ ");
		return;
	}
	pcur_block = *itor;
	pcur_block->packindex = index;
	pcur_block->packLen = len;
	pcur_block->packcopyedLen = 0;
	m_freeList.pop_front();
}

int CQQData::CopyData(char* pData, int len)
{
	int pos = 0;
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	if( pcur_block == NULL )
	{
		SVP_INFO(" pcur_block == NULL ");
		return 0;
	}
	int uncopylen = pcur_block->packLen - pcur_block->packcopyedLen;
	uncopylen > len ? pos = len : pos = uncopylen;
	pcur_block->buffer.append(pData, pos);
	pcur_block->packcopyedLen += pos;
	copyedLen += pos;
	localsize += pos;
	return pos;
}

bool CQQData::GetData(pcmblock_pt ptblock)
{
	std::lock_guard<std::recursive_mutex> lock(m_mtx);
	std::list<pcmblock_pt>::iterator itor = m_blockList.begin();
	if( itor ==  m_blockList.end())
	{
		return false;
	}
	pcmblock_pt p_block = *itor;
	ptblock->id = p_block->id;
	ptblock->packindex = p_block->packindex;
	ptblock->packLen = p_block->packLen;
	ptblock->packcopyedLen = p_block->packcopyedLen;
	ptblock->buffer.clear();
	ptblock->buffer.append(p_block->buffer);
	localsize -= p_block->packcopyedLen;
	resetblock(p_block);
	m_freeList.push_back(p_block);
	m_blockList.pop_front();
	return true;
}



