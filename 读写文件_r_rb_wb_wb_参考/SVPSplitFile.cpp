#include <sys/types.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#include "SVPLog.h"

#include "SVPSplitFile.h"
#define  OUTPUTPATH   "/media/usbstorage0/LOG/DSVLog/"
//#define  OUTPUTPATH   "/storage/data/GYH/"
#define  HEADSIZE  1426   /* 实验看到的文件头是1482 - 56 */
#define  GETEND_M  1024*1024   /* 实验看到的文件头是1482 - 56 */


static const char  g_keyStr[11] =  "user.alert";

int SVPSplitFile::StringFind(const char *pSrc, const char *pDst)  
{  
    int i, j;  
    for (i=0; pSrc[i]!='\0'; i++)  
    {  
        if(pSrc[i]!=pDst[0])  
            continue;         
        j = 0;  
        while(pDst[j]!='\0' && pSrc[i+j]!='\0')  
        {  
            j++;  
            if(pDst[j]!=pSrc[i+j])  
            break;  
        }  
        if(pDst[j]=='\0')  
            return i;  
    }  
    return -1;  
}

bool SVPSplitFile::SplitOpenFile(const char *filepath )
{
    long fsta,fend;
    strncpy(tFileOpr.filepath, filepath, strlen(filepath));
    if((tFileOpr.fp = fopen(filepath, "r")) == NULL)
    {
    	SVP_ERROR("can't open source file %s \n",filepath);
        return false;
    }

    tFileOpr.curSplit = 0;

    fsta = ftell(tFileOpr.fp);
    fseek(tFileOpr.fp, 0, SEEK_END);
    fend = ftell(tFileOpr.fp);
    tFileOpr.fLen = fend - fsta;
    if(tFileOpr.fLen > HEADSIZE)
    {
        fseek(tFileOpr.fp, HEADSIZE, SEEK_SET);
        fsta = ftell(tFileOpr.fp);
        fseek(tFileOpr.fp, 0, SEEK_END);
        fend = ftell(tFileOpr.fp);
        tFileOpr.logsize = fend - fsta;
    }
    else
    {
    	SVP_ERROR("tFileOpr.fLen <= head size->%d",HEADSIZE);
    	return false;
    }

    return true;

}

bool SVPSplitFile:: GetLogDate(char * src, char **pdst)
{
    int SpecilPos = 0;
    int s32temp1 = 0;
    char * pdatestr;

    char * ptempdate1;
    
    SpecilPos = StringFind(src,g_keyStr);
    if(SpecilPos < 19)
    {
        s32temp1 = SpecilPos+sizeof(g_keyStr) - 1;
        SpecilPos = StringFind(src+s32temp1 ,g_keyStr);
        if(SpecilPos < 19)
        {
            return false;
        }
        SpecilPos += s32temp1;
    }
    pdatestr = src + SpecilPos - 19;


    strncpy(*pdst,pdatestr,15);
    (*pdst)[16] = '\0';
    ptempdate1 = (*pdst);
    ptempdate1 = ptempdate1 + 3;
    *ptempdate1++ = '-';
    if(' ' == *ptempdate1  )
    {
        *ptempdate1 = '0';
    }
    ptempdate1 = ptempdate1 + 2;
    *ptempdate1 = '-';
    ptempdate1 = ptempdate1 + 3;
    *ptempdate1 = '-';
    ptempdate1 = ptempdate1+3;
    *ptempdate1 = '-';
    
    //DBG_PRINTF("date 1:%s\n",(*pdst));
    return true;
}

bool SVPSplitFile::OutPutnMLog( int nM)
{
	bool ret = false;
    int rsize = 0;
    int wsize = 0;
    FILE * pOutputFilefd;

	char *pfbufPos;
    char datetime[20] = {0};
    char logfilename[100] = {0};
    char * ptemp1 = datetime;
    char cfiletype[] = { ".txt"};
    char cpath[] = {OUTPUTPATH};


    //取最后nM的log,定位文件指针
    if( tFileOpr.logsize > nM*GETEND_M )
    {
    	fseek(tFileOpr.fp, -nM*GETEND_M, SEEK_END);
    }
    else
    {
    	fseek(tFileOpr.fp, HEADSIZE, SEEK_SET);
    }
    tFileOpr.curSplit = 0;


	memset(tFileOpr.fBuf,'\0',sizeof(tFileOpr.fBuf  ));
	rsize = fread(tFileOpr.fBuf, 1, BUFSIZE, tFileOpr.fp);
	if(rsize < 20)
	{
		SVP_ERROR("rsize too small \n");
		return false;
	}
	pfbufPos = tFileOpr.fBuf;

	ret = GetLogDate(pfbufPos, &ptemp1);
	if( ret == false )
	{
		SVP_ERROR("GetLogDate  error \n");
		return false;
	}

	memset(logfilename,'\0',sizeof(logfilename));
	strcat(logfilename,cpath);
	strcat(logfilename,datetime);
	strcat(logfilename,cfiletype);

	//DBG_PRINTF("gyh File:%s\n",logfilename);

	pOutputFilefd = fopen (logfilename, "w");
	if(NULL == pOutputFilefd)
	{
		SVP_ERROR("fopen OutputFile Error \n");
		return false;
	}

    while( tFileOpr.curSplit <  tFileOpr.logsize)
	{
		DBG_PRINTF("curSplit:%d  rsize:%d \n", tFileOpr.curSplit,rsize);
    	wsize = fwrite(  pfbufPos, 1 , rsize ,  pOutputFilefd);
		if( wsize <= 0)
		{
			SVP_ERROR("fwrite Error \n");
			break;
		}

		tFileOpr.curSplit += rsize;

		memset(tFileOpr.fBuf,'\0',sizeof(tFileOpr.fBuf  ));
		rsize = fread(tFileOpr.fBuf, 1, BUFSIZE, tFileOpr.fp);
		if(rsize <= 0)
		{
			DBG_PRINTF("rsize too small,break while \n");
			break;
		}
		pfbufPos = tFileOpr.fBuf;

	}

	fclose(pOutputFilefd);


	return true;

}

bool SVPSplitFile::SplitFileAction( )
{
    bool  iError = false;
    int rsize = 0;
    int wsize = 0;
    
    char *pfbufPos;
    char datetime[20] = {0};
    char logfilename[100] = {0};
    char * ptemp1 = datetime;
    char cfiletype[] = { ".txt"};
    char cpath[] = {OUTPUTPATH};

    FILE * pOutputFilefd;
    
    if(tFileOpr.fp == NULL)
    {
        return false;
    }
    tFileOpr.curSplit = 0;
    fseek(tFileOpr.fp, HEADSIZE, SEEK_SET);/* 实验看到的文件头是1482 - 56 */
    while( tFileOpr.curSplit <  tFileOpr.fLen)
    {
        memset(tFileOpr.fBuf,'\0',sizeof(tFileOpr.fBuf  ));
        rsize = fread(tFileOpr.fBuf, 1, BUFSIZE, tFileOpr.fp);
        if(rsize < 20)
        {
            DBG_PRINTF("rsize too small \n");
            break;
        }
        pfbufPos = tFileOpr.fBuf;
        
        iError = GetLogDate(pfbufPos, &ptemp1);
        if( iError == false )
        {
            DBG_PRINTF("GetLogDate  error \n");
            return false;
        }

        memset(logfilename,'\0',sizeof(logfilename  ));
        strcat(logfilename,cpath);
        strcat(logfilename,datetime);
        strcat(logfilename,cfiletype);
        
        //DBG_PRINTF("gyh File:%s\n",logfilename);
        
        pOutputFilefd = fopen (logfilename, "w");
        if(NULL == pOutputFilefd)
        {
            DBG_PRINTF("fopen OutputFile Error \n");
            return false;
        }
        wsize = fwrite(  pfbufPos, 1 , rsize ,  pOutputFilefd);
        if( wsize <= 0)
        {
            DBG_PRINTF("fwrite Error \n");
            fclose(pOutputFilefd);
            continue;
        }
        fclose(pOutputFilefd);

        tFileOpr.curSplit += rsize;
        //DBG_PRINTF("curSplit:%d  rsize:%d \n", tFileOpr.curSplit,rsize);
    }

	return true;
}

void SVPSplitFile:: SplitExit()
{
    fclose(tFileOpr.fp);
    tFileOpr.fp = NULL;
}


