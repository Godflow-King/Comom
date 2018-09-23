#ifndef  SVP_QMUSIC_BASE_H_
#define  SVP_QMUSIC_BASE_H_
#include <list>

/******************************************QplayAuto使用注意和配置*********************************************/
//这里是设置编译lib模式，还是调试的app模式，注意修改这点后还要到 .pro 文件将TEMPLATE 修改成 app/lib。
//#define appmode
//是否是直接进入道播放音乐，如果是，需要#define和刘治平商量的方案
#define Enter_PLAY_SCREEN
//是否支持歌词显示功能
#define ENABLE_LYRIC
//定义是否在长时间状态缓冲下切歌
#define LONGTIMEBUFFERPLAYNEXTSONG
#define HOWLONGPLAYNEXTSONG                     15              //约N秒后，时间都在缓冲，切歌
//定义时间获取下一个PCM包的时间间隔,ms
#define GETPCMinterval                          2000
//特殊，繁杂的trace用来跟踪问题 Etrace("");
//#define OPEN_ETRACE
//定义是否使用C语言char[]去中转音频数据，解决挂死问题，调试用
#define USERAUDIOBUF
//是否设置ALSA音频播放的优先级,解决断音问题
//#define ALSA_priority                                         //有设置优先级
//日期，标注库的修改是否完成,每次修改代编译后，打印的日期都是编译代码的日期，和时间，与编译代码的电脑相同
#define WHAT_DAY_IS_TODAY  __DATE__
#define THE_DAY_TIME       __TIME__
/**********************************************************************************************************/

typedef enum
{
    PCMData=1,
    PICData,
    Lyric
}E_DATATYPE;

typedef struct
{
	int curstate;
	int laststate;
}NetState;
//当前歌曲播放的状态数据结构
typedef struct
{
    int  State;                 // 当前歌曲状态
    int  CurTimes;              // 播放时长
    int  Duration;              // 播放总时长
    NetState netState; // 网络状态 1.connect,0.disconnect
    std::string SongID;         // 歌曲 id  每次换歌曲，都要将其设置
    std::string Playingstatus;
}PlayingState;


//目录或者歌曲列表的数据结构
typedef struct
{
    int Type;               //
    std::string ID;         // 歌词 id或者目录ID
    std::string Name;       // 歌词或者目录名
    std::string Artist;     // 歌手名
    std::string Album;      // 专辑 id
}Lists;
typedef struct
{
    int PageIndex;
    int Items;                  // 总记录数
    int HasGetItems;
    int PagePerCount;
    std::string ParentID;       //标记，和接收数据比对用
    std::string ParentName;     //标记，和接收数据比对用
    std::list<Lists> ItemsList;
}QQMusicItemsList;

typedef struct
{
public:
    int PageIndex;
    int TotalLength;
    int Length;
    int ValidLength;
    std::string SongID;
}PicOrPCMData;

typedef enum
{
    Q_PLAY    	             = 0,//²¥·Å×´Ì¬
    Q_PAUSE                  = 1,//1
    Q_STOP,	    	         //2
    Q_PREV,	    	         //3
    Q_NEXT,	                 //4

    Q_PHONE_CONNECTING = 10,    //ÍøÂçÁ¬½Ó×´Ì¬
    Q_PHONE_DISCONNECT = 11,  	//11
    Q_ABNORMAL_DISCONNECT,  	//12
    Q_RECONNECT_FAILUER,	    //13
    Q_CONNECTED_PHONE_CHANGE,   //14
    Q_SHOW_LOADING_AFTER_5Sec,

    Q_ERROR_NO_3G_NETWORK = 103,
    Q_ERROR_SONGID_NO_EXIST = 105,
    Q_ERROR_READ_DATA_ERROR = 106,
    Q_ERROR_NO_COPYRIGT = 109,
    Q_ERROR_NO_LOGIN = 110,
    Q_ERROR_PCM_REPEATREQUEST = 111,
}QPlayStateID;

#endif
