#ifndef SVP_FREE_MANAGER_H_
#define SVP_FREE_MANAGER_H_

#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "config.h"

/*
 * 模板类本身不会实例化，但是传入TNode值后，会实例一个类
 */

template <class TNode>
class Freemanager
{
public:
	void deleteNode(std::string type);
	void addNode( TNode * ptNode );
	TNode * GetNodeByType(std::string type);

	static Freemanager* GetInstance()
	{
		static Freemanager instance;
	        return &instance;
	};

private:
	Freemanager(void){};
    ~Freemanager(void);
    Freemanager(Freemanager const&);
    Freemanager& operator=(Freemanager const&);

	std::vector<TNode *> g_NodeList;
};

/* 往下的定义，这里自己吃了大亏，经验 */
template <class TNode>
Freemanager<TNode>::~Freemanager(void)
{
	typename  std::vector<TNode *>::iterator it;/* 根据资料所，gcc不知道 TNode是种类型还是变量，所以加typename 说明这是个模板类型*/
	for( it = g_NodeList.begin(); it != g_NodeList.end(); it++)
	{
		if( (*it) != NULL )
		{
			delete (*it);
			(*it) = NULL;
		}
	}
	g_NodeList.clear();
}

template <class TNode>
void Freemanager<TNode>::deleteNode(std::string type)
{
	typename std::vector<TNode *>::iterator it;
	for( it = g_NodeList.begin(); it != g_NodeList.end(); it++)
	{
		if(  (*it)->ID == type  )
		{
			delete (*it);
			(*it) = NULL;
			return;
		}
	}
}

template <class TNode>
void Freemanager<TNode>::addNode( TNode * ptNode )
{
	g_NodeList.push_back(ptNode);
}

template <class TNode>
TNode * Freemanager<TNode>::GetNodeByType(std::string type)
{
	typename  std::vector<TNode *>::iterator it;
	for( it = g_NodeList.begin(); it !=  g_NodeList.end() ; it++)
	{
		if(  (*it)->ID == type  )
			return (*it);
	}
	return NULL;
}


#endif /* SVP_FREE_MANAGER_H_ */
