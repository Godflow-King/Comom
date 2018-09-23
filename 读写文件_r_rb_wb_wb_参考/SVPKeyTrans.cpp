#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SVPKeyTrans.h"
#include "SVPLog.h"
#include "Can/SVPCanService.h"

#define  FIFO_APP2OS   "/tmp/fifo_app2os"
#define  FIFO_OS2APP   "/tmp/fifo_os2app"
#define  FIFOBUFFER_SIZE    PIPE_BUF

#define  OUTPUTPATH   "/tmp/personalInfo"
#define  INPUTPATH    "/tmp/personalInfo-e"
//#define  INPUTPATH    "/storage/app/sv.g5.cn210m.auxin/bin/testforos/secret-d.txt"

//#define  CHAR    "0144917b0000529429ce7318c618c610:MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCcsQYzV77P1Ge73uE1XhJIKEQLh4mdQd00BYKvUg00HNUDOjm+fG1rPVBpBDaL7uAY8euTQF/EticXKDfFzrJ4pSYWiCfimLTeLV+zaLVZcKQRcvdM5aASC/y3kPSl52WTXhf7AZLjqGyLd2VMysh+WJH+L6s1ICZfXoTSWbHbAS+UgziH9iCh0Zj0spB4JjznDFtWqJQkErX/xgphvDUUudhr5zOgrzDgFo34lY0sAwpXlkvFN4w48WkJvvXrn2DaZbXOEDECBm1O1tu04m28WWfMrYu2ciJEB2/xbfBJtVlD0S3Mo2bxqzVgvBvN7/IpO8eXsNvIYRBeKDsp1boVAgMBAAECggEARQg0rCAHP7bbcMiTvZhRQ5sBx3aSKUcgNlALMERc7s2ZBw/66rIU3r8nkuy5P1MhRbwnRjuBNb/uXuRoFeynckGIEzsvH0SZwpyI4hjKdLoMfmeBDk7IA8tmLlVdg0370T4OIJaj6UqFb7hNV9SrGgbZYvwLWJ9uaGvS5n1/vvUHCuGE7F9xV+zQ46Y+xMtWoHuVohqvToa7TpXeeypGMvuL4onMfBEPhYq9//BMhuj891+8+eElPwRxjKdBRMXnr/j0bdODRfCfqkmVvzfCrHElL3VWFVpKC5sqNLuetU3I2DyyzWJCFEBLOMg1XcAyAP0sjFe0doj4vvEn3N3bQQKBgQDrGnZ5BOjAEZPVD2rq0qohgD55dhWQcdSwfJNwm00N6LlXmT0gSkp9s4iOyqPUGIYEg2SqNTuyBNFIUgV3DzoSBH91Enfi33RV/EqfFVVzQC7ppC+E1p0bAeTFicXr5kG2jkRgQKHhTAuik1MJEUOL62jqSb48/XKUNbzJxGBShQKBgQCqnmEfZPBMQY8tpCek+yj2gpAnQDamO8cbTabdinR/CSjCBpkGdK0bUEizcgSWKIhdL7u9ejBnCUtq4CFtqOpxJArVBojOS4jPvF3CPesIyyrXhrqSH8hEHAaUGRDim62aVs67tPokHumfHf5UQFWwIe/uxEkYnXg45eVXFKKGUQKBgQCwF3cLN+kZC1Fd8RlU3ws84nBoy1Bli00SD4zjd/7j3x8LhKRrDhnzQsiSoybIUsH2mW+JqSuYL9GNSJn9TjNhkWriBj4zeMkLKle+Bd2l4DoDF83bwl+T/fwbFRFgmms30CFYsrBNLoc8cvprvTmJkH5ZlXhe+Dqc1g8cVPEMfQKBgQCaPvSu3SIgzmQSbwUoMaemtAJ+eZ7uSbyHAnyIbFNFRZKDVlOhcnnM55fSIpHi7mHZXP3tHjoD7HMx+848xSitgFgKng2v8rmMlE5u/GsvV/0zO6oP0IvMh0mrOb36H6OilDfxmntJjGjIYOU8Za0RvpyazvkGN9YoGi5ru0vzAQKBgCBdpb/aX+o4Ue5K6V15fqvqDcp/leon/DsK863IHhsHr489lwyR5hwrsmL0sLkK4PLbGPbhGZJaa+uskHVS17IjR7QrNsgynTqXtJJT04DnQYOcDL5k0aQ/H/1F2xHJosM5WSt/rjBE430SHCPpA4iEn6hABcEs9Qr6lCxU3kp2:Q3zeZ0zujiBym9i1nnNenGVUgv2WU/5OSgERjqvDSL8=:88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888:"
//#define  TempMd5  "e8c81928b5e3657040ba69412c5051e5"



/*===========================================inline========================================================*/
//16进制转换成字符串
inline bool SVPKeyTrans::HexToCharArray(char* pOut,  int nOutLen, unsigned char* pInput,  int nInLen)
{
	char	ddl,ddh;
	int i;
	if(nOutLen < nInLen*2)
		return false;

	for (i=0; i<nInLen; i++)
	{
	ddh = 48 + pInput[i] / 16;
	ddl = 48 + pInput[i] % 16;
	if (ddh > 57) ddh = ddh + 7;
	if (ddl > 57) ddl = ddl + 7;
	pOut[i*2] = ddh;
	pOut[i*2+1] = ddl;
	}

	//pOut[nInLen*2] = '\0';
	return true;
}
bool SVPKeyTrans::CharBuf2HexBuf(unsigned char *pDes,int nDesLen, char *pSrc ,int nSrcLen)
{
	if(nDesLen != 16 && nSrcLen >=32)
		return false;

	unsigned char tempu8date;
	for(int iLoop = 0;iLoop < nSrcLen; )
	{
		tempu8date = Char2Hex(pSrc[iLoop]);
		tempu8date <<= 4;
		tempu8date += Char2Hex(pSrc[iLoop+1]);

		pDes[iLoop/2] = tempu8date;
		iLoop = iLoop + 2;
	}
	return true;
}
inline unsigned char SVPKeyTrans::Char2Hex(const char C)
{
	unsigned char Data;
	if (C >= '0' && C <= '9')
	{
    	Data = C - '0';
	}
	else if (C >= 'A' && C <= 'Z')
	{
        Data = C - 'A' + 10;
	}
	else if (C >= 'a' && C <= 'z')
	{
        Data = C - 'a' + 10;
	}
	return Data;
}
/*===========================================inline========================================================*/


/*========================================MCU_INPUT========================================================*/

bool SVPKeyTrans::mcuInputAPP_key( void * pSrc,int index)
{
	SVP_INFO_FUNC();

	int bufindex;
	bufindex = index*64;
	if(index >= 32)
	{
		return false;
	}
	char *pCharBuf = (char *)pSrc;
	memcpy(&mkeybuf[bufindex],pCharBuf,64);
	ismkeybufOK = index;
	DBG_PRINTF( "========mcuInputAPP_key %d ========",index);
	Test_Show_Printf(&mkeybuf[bufindex],64,false);

	return true;
}

bool SVPKeyTrans::mcuInputAPP_MD5(const unsigned char * pmd5)
{
	SVP_INFO_FUNC();

	unsigned char original[16];
	memcpy(&original[0],pmd5,16);

	HexToCharArray(m_inputmd5.msg_text,sizeof(m_inputmd5.msg_text),original,sizeof(original));
	Test_Show_Printf(m_inputmd5.msg_text,sizeof(m_inputmd5.msg_text),false);

	if(ismkeybufOK == 31)
		APPOutputOS();
	else
	{
		DBG_PRINTF("mcuInputAPP_MD5 Fail");
		return false;
	}
	ismkeybufOK = -1;


	return true;
}

bool SVPKeyTrans::mcuInputAPP_Request()
{
	SVP_INFO_FUNC();
	m_inputmd5.msg_id = 0xB200;
	SendMsgToOS( &m_inputmd5,sizeof(m_inputmd5) );

	return true;
}
/*========================================MCU_INPUT=======================================================*/




void SVPKeyTrans::Test_Show_Printf(void *pbuf,int size,bool ishex)
{
	SVP_INFO_FUNC();
	/*show*/
	DBG_PRINTF("=====================Test=========================");
	int iLoop;
	char *pchar = (char *)pbuf;
	printf("\n");
	for(iLoop = 0; iLoop < size; iLoop++)
	{
		if(ishex)
			printf("0x[%02X]",pchar[iLoop]);
		else
			printf("[%c]",pchar[iLoop]);
	}
	printf("\n");
	DBG_PRINTF("=====================Test=========================");
}

bool SVPKeyTrans::APPtoMCU(char * pmd5)
{
	SVP_INFO_FUNC();
	T_toMCUPAG temppag;
	size_t rsize;
	unsigned char fbuffer[100];
	int fsta,fend,fLen;
	unsigned char u8md5buf[16] ;
	unsigned char tempu8date;
	char origiBuf[32];
	FILE *ReadOSfp;
	int iLoop;


	/*Transfor char buffer to Hex char*/
	memcpy( &origiBuf[0],pmd5,sizeof(origiBuf) );

	CharBuf2HexBuf(u8md5buf,16,origiBuf,32 );


    if( (ReadOSfp = fopen(INPUTPATH, "r")) == NULL)
    {
        DBG_PRINTF("gyh can't open source file\n" );
        return false;
    }

    /*
     * the Size of file
     * */
    fsta = ftell(ReadOSfp);
    fseek(ReadOSfp, 0, SEEK_END);
    fend = ftell(ReadOSfp);
    fLen = fend - fsta;
    if(fLen > 0)
        fseek(ReadOSfp, 0, SEEK_SET);

    temppag.total = fLen/100;
    if( fLen%100 > 0 )
    	temppag.total++;

    temppag.index = 0;
    while( temppag.index < temppag.total )
    {
    	rsize = fread(fbuffer, 1, 100, ReadOSfp);
    	if(rsize <= 100 && rsize > 0)
    	{
    		temppag.len = rsize;
    		if(rsize != 100)
    		{
    			memset(&temppag.date[0],' ',sizeof( temppag.date ));
    		}

    		memcpy(&temppag.date[0],fbuffer,rsize);

    		Test_Show_Printf( temppag.date,sizeof(temppag.date) ,true);
    		SVP_Sleep(80);
    		SVPCanService::GetInstance()->SPAPI_SendPack(0x7439,&temppag,sizeof(temppag) );


			temppag.index++;
    	}
    	else
    	{
    		DBG_PRINTF("APP Transport Data Had Done");
    		break;
    	}
    }

    fclose(ReadOSfp);

    if(temppag.index == temppag.total)
    {
    	/*J6发送Personal Info MD5校验码给MCU*/
//    	SVPCanService::GetInstance()->SPAPI_SendEcoEnergyMsg(1, 0x743A, u8md5buf);
    	Test_Show_Printf(u8md5buf,sizeof(u8md5buf) ,true );
    	SVPCanService::GetInstance()->SPAPI_SendPack(0x743A,u8md5buf,sizeof(u8md5buf) );
    }
    else
    {
    	DBG_PRINTF("========APP Transport Error =======");
    	return false;
    }


	return true;
}







/*================================================================================================*/

bool SVPKeyTrans::APPOutputOS()
{

	size_t wsize;
	char *pdata;
	FILE *WriterOSfp;

	remove(OUTPUTPATH);
	DBG_PRINTF("gyh============OutputOS===============");
    if( (WriterOSfp = fopen(OUTPUTPATH, "w")) == NULL)
    {
        DBG_PRINTF("gyh can't open source file\n" );
        return false;
    }

    wsize = fwrite(  mkeybuf, 1 , KEYBUF_SIZE ,  WriterOSfp);
	if(wsize == KEYBUF_SIZE)
	{
		m_inputmd5.msg_id = 0xB100;
		m_inputmd5.result = 0x0;
//		sprintf(m_inputmd5.msg_text,"%s",TempMd5);
		DBG_PRINTF("gyh===Send.msg_id:%08X=========",m_inputmd5.msg_id);
		DBG_PRINTF("gyh===Send.result:%08X=========",m_inputmd5.result);
		DBG_PRINTF("gyh===Send.msg_text:%s===========",m_inputmd5.msg_text);
		SendMsgToOS( &m_inputmd5,sizeof(m_inputmd5) );

	}
	else
	{
		 DBG_PRINTF("gyh wsize != KEYBUF_SIZE" );
	}
    fclose(WriterOSfp);

	return true;
}


bool SVPKeyTrans::WattingOSinputAPP()
{
    int pipe_fd;
    int res;
    int bytes = 0;

    PT_MsgMd5  pmsg;

    if (access(FIFO_OS2APP, F_OK) == -1)
    {
        res = mkfifo(FIFO_OS2APP, 0777);
        if (res != 0)
        {
        	DBG_PRINTF("Could not create fifo %s\n", FIFO_OS2APP);
            return false;
        }
    }

    pipe_fd = open(FIFO_OS2APP, O_RDWR);  //O_RDONLY
    if (pipe_fd == -1)
    {
    	DBG_PRINTF("Could not open fifo file-> %s\n", FIFO_OS2APP);
        return false;
    }

	do
	{
		memset(mfifobuffer, '\0', sizeof(mfifobuffer));
		bytes = read(pipe_fd, mfifobuffer, sizeof(T_MsgMd5) );
		if( bytes != sizeof(T_MsgMd5) )
		{
			DBG_PRINTF( "bytes != sizeof(T_MsgMd5)" );
			continue;
		}
		pmsg = (PT_MsgMd5)mfifobuffer;

		DBG_PRINTF("read : %d",bytes);
		if(pmsg->msg_id == 0xB100)
		{
			DBG_PRINTF("Get pmsg->msg_id : %04X",pmsg->msg_id);
			DBG_PRINTF("Get pmsg->result : %04X",pmsg->result);
			DBG_PRINTF("Get pmsg->msg_text : %s",pmsg->msg_text);

	    	if( pmsg->result == 0x10 )
	    	{
	    		SVPCanService::GetInstance()->SPAPI_SendEcoEnergyMsg(1, 0x7438, 0);
	    		DBG_PRINTF("gyh Succeed !");
	    	}
	    	else
	    	{
	    		SVPCanService::GetInstance()->SPAPI_SendEcoEnergyMsg(1, 0x7438, 1);
	    		DBG_PRINTF("gyh Fail !");
	    	}

		}
		else if(pmsg->msg_id == 0xB200)
		{
			DBG_PRINTF("Get pmsg->msg_id : %04X",pmsg->msg_id);
			DBG_PRINTF("Get pmsg->result : %04X",pmsg->result);
			DBG_PRINTF("Get pmsg->msg_text : %s",pmsg->msg_text);

			APPtoMCU(pmsg->msg_text);
		}
		else
		{
			DBG_PRINTF("Get Others pmsg->msg_id : %04X",pmsg->msg_id);
		}



	}while(bytes > 0);

   close(pipe_fd);

	return true;
}


bool  SVPKeyTrans::SendMsgToOS(const void * pdata,int size)
{
    int pipe_fd;
    int res;

    int bytes = 0;

    if (access(FIFO_APP2OS, F_OK) == -1)
    {
        res = mkfifo(FIFO_APP2OS, 0777);
        if (res != 0)
        {
        	DBG_PRINTF("Could not create fifo %s\n", FIFO_APP2OS);
            return false;
        }
    }

    pipe_fd = open(FIFO_APP2OS,  O_WRONLY);  //only writer O_WRONLY     // O_RDWR
    if( pipe_fd == -1 )
    {
    	DBG_PRINTF("Could not open fifo file-> %s\n", FIFO_APP2OS);
        return false;
    }


	bytes = write(pipe_fd, pdata, size);
	if (bytes == -1)
	{
		DBG_PRINTF("Write error on pipe");
		return false;
	}
	close(pipe_fd);

    return true;
}

SVPVoid* SVPKeyTrans::TransportThreadFun(SVPVoid* pdata)
{
	SVP_INFO("gyh ==========TransportThreadFun===Case1=================");
	SVPKeyTrans * pthis = (SVPKeyTrans *)pdata;

	pthis->WattingOSinputAPP();

	pthis->KeyThreadExit();

	DBG_PRINTF( "gyh ========TransportThreadFun=====Done=================");
	return NULL;
}


SVPVoid SVPKeyTrans::KeyThreadExit()
{
	if(m_thread == NULL)
	{
		m_thread->WaitQuit();
	    m_thread = NULL;
	}
}

void SVPKeyTrans::CreatWaittingThread()
{
	m_thread = new SVPThread();
	if(m_thread == NULL)
	{
		DBG_PRINTF("m_thread == NULL");
		return;
	}
	m_thread->Start(TransportThreadFun, this);
}


