#ifndef SVP_KEYTRANS_API_H_
#define SVP_KEYTRANS_API_H_

#include <stdio.h>
#include "SVPType.h"
#include "SVPCriticalSection.h"
#include "SVPThread.h"
#include <boost/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>



#pragma  pack (push,1)

typedef struct MSGMD5
{
    unsigned int msg_id;
    unsigned int result;
    char msg_text[33];
}T_MsgMd5,*PT_MsgMd5;

typedef struct toMCUPAG
{
	unsigned char total;
	unsigned char index;
	unsigned char len;
	unsigned char date[100];
}T_toMCUPAG,*PtoMCUPAG;

#pragma pack(pop)


#define  DBG_PRINTF    SVP_INFO
#define  KEYBUF_SIZE   2048


class SVPKeyTrans
{
public:

	static SVPKeyTrans& GetInstance()
	{
		static SVPKeyTrans instance;
	        return instance;
	}

	void CreatWaittingThread();


	bool mcuInputAPP_key( void * pSrc,int index);

	bool mcuInputAPP_MD5(const unsigned char * pmd5);

	bool mcuInputAPP_Request();
	bool CharBuf2HexBuf(unsigned char *pDes,int nDesLen, char *pSrc ,int nSrcLen);

private:
	SVPKeyTrans()
	{
	};
	~SVPKeyTrans()
	{
		KeyThreadExit();
	};
	bool APPtoMCU(char * pmd5);
	bool APPOutputOS();
	bool SendMsgToOS(const void * pdata,int size);
	static SVPVoid* TransportThreadFun(SVPVoid* pdata);
	bool WattingOSinputAPP();
	SVPVoid KeyThreadExit();

	inline  bool HexToCharArray(char* pOut,  int nOutLen, unsigned char* pInput,  int nInLen);
//	bool CharBuf2HexBuf(unsigned char *pDes,int nDesLen, char *pSrc ,int nSrcLen);
	inline  unsigned char Char2Hex(const char C);

	void Test_Show_Printf(void *pbuf,int size,bool ishex);

	T_MsgMd5 m_inputmd5;
	char mkeybuf[KEYBUF_SIZE];
	unsigned char mfifobuffer[256];

	SVPThread * m_thread = NULL;

	int ismkeybufOK = -1;

};

#endif /* SVP_KEYTRANS_API_H_ */
