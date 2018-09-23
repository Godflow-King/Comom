#ifndef SVP_GYHSPLIT_FILE_H_
#define SVP_GYHSPLIT_FILE_H_

/* 用最严谨的代码跑最垃圾的流程，╭(╯^╰)╮，日后如果由后者在维护这个项目，希望不是我，千万不要喷我，合作的人不愿意按照需求更改，我这里也显得逻辑雍炯;形式所迫，让我在垃圾里溺死吧 */

#include <stdio.h>

#define  BUFSIZE     1024*1024
#define DBG_PRINTF SVP_DEBUG

#pragma  pack (push,1)
typedef struct FileOpr
{
    FILE *fp;
    char filepath[100];
    char fBuf[BUFSIZE];
    unsigned int fLen;
    unsigned int logsize;
    unsigned int curSplit;
}T_FileOpr, *PT_FileOpr;
#pragma pack(pop)

class SVPSplitFile
{
public:
    SVPSplitFile(){tFileOpr.fp = NULL;}
    ~SVPSplitFile()
    {
         if( tFileOpr.fp != NULL)
        {
            fclose(tFileOpr.fp);
         }
    } 

    bool OutPutnMLog( int nM);
 
    bool SplitOpenFile(const char *filepath );

    bool SplitFileAction();

    void  SplitExit();
    
private:
    bool  GetLogDate(char * src, char **pdst);
    int  StringFind(const char *pSrc, const char *pDst);
    T_FileOpr tFileOpr;
};

#endif /* SVP_SVPSPLIT_FILE_H_ */
