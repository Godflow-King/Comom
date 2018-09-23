#include <QDebug>
#include "logutil.h"
#include "qqmusicservice.h"
#include <fstream>
#include <sstream>
#include <QImage>

QQMusicService::QQMusicService()
:m_ID3Title("")
,m_ID3Artist("")
,m_ID3Album("")
,m_strlistPath("")
,m_iDuration(0)
,m_iPos(0)
,m_iPlayStatus(0)
,m_iRepeatShuffleMode(0)
{
    SVP_INFO_FUNC();


#ifdef IMPORT_LIB_APPINTERFACE
    m_pShareMemory = InterfaceManager::GetInstance()->GetShareMemoryPointer();
    m_pLargeValueManager = InterfaceManager::GetInstance()->GetLargeValueManager();

    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_ID3_UPDATE,"updateQQMusicID3");
    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_LIST_UPDATE,"updateFileListData");
    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_SEARCHRESULT_UPDATE,"updateResultList");

    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_PLAY_UPDATE,"updatePlayStatus");
    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_REPEAT_UPDATE,"updateRepeatStatus");
    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_POSITION_UPDATE,"updatePos");
    registerCallbackForUpdateContent(NotifyScreenContentUpdateData::WEBAPP_QQMUSIC_PIC_UPDATE,"updateQQMusicPic");
#endif

}

QQMusicService::~QQMusicService()
{
    SVP_INFO_FUNC();
}

void QQMusicService::init()
{
    SVP_INFO_FUNC();
    updateQQMusicID3();
    updateFileListData();
    updatePlayStatus();
    updateRepeatStatus();
}

void QQMusicService::cancleConnectPhone()
{
#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_CANCLE_CONNECT);
#endif
}

void QQMusicService::updatePlayStatus()
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_iPlayStatus = m_pShareMemory->WebAppValue.qqmusicValue.PlayStatus;
     emit this->PlayStatusChanged();
#endif
}

void QQMusicService::updateRepeatStatus()
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_iRepeatShuffleMode = m_pShareMemory->WebAppValue.qqmusicValue.RepeatStatus;
     emit this->RepeatShuffleModeChanged();
#endif
}

void QQMusicService::updatePos()
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_iPos = m_pShareMemory->WebAppValue.qqmusicValue.pos;
    emit this->PosChanged();
#endif
}

void QQMusicService::updateQQMusicPic()
{
//    QString testpath = "qrc:/AppHMI/General/Music/Album_Default.png";
//    std::ifstream m_stream("/tmp/qqmusic_output",std::ios::in|std::ios::binary);
//    if (!m_stream.is_open())
//    {
//        SVP_INFO("/tmp/qqmusic_output no path");
//        setID3Pic(testpath);
//        return;
//    }
//    int fstart = m_stream.tellg();
//    m_stream.seekg (0, std::ios::end);
//    int fend = m_stream.tellg();
//    m_stream.seekg (0, std::ios::beg);
//    int  fsize = fend - fstart;
//    unsigned char picbuf[fsize];
//    m_stream.read(reinterpret_cast<char *>(picbuf),fsize);
//    m_stream.close();
//    SVP_INFO(QString("Get size : %1").arg(fsize) );
//    QImage mimage;
//    mimage.loadFromData(picbuf, fsize );
//    if( ! mimage.save("/tmp/qqmusic_output.png","PNG") ) /* Maybe QQuickImageProvider was better !*/
//    {
//        SVP_INFO(" updateQQMusicPic : mimage save error ");
//        setID3Pic(testpath);
//        return;
//    }
//    testpath = QString(QString("file://%1").arg("/tmp/qqmusic_output.png").toLocal8Bit());
//    setID3Pic(testpath);
//    QQMusicImage::GetInstance()->g_qqmusicImage.load("/tmp/qqmusic_output");
    emit id3PicChanged();

}

void QQMusicService::updateQQMusicID3()
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_strlistPath = m_pShareMemory->WebAppValue.qqmusicValue.ListPath;
    m_ID3Title = m_pShareMemory->WebAppValue.qqmusicValue.ID3.Title;
    m_ID3Artist = m_pShareMemory->WebAppValue.qqmusicValue.ID3.Artist;
    m_ID3Album = m_pShareMemory->WebAppValue.qqmusicValue.ID3.Album;
    m_iDuration = m_pShareMemory->WebAppValue.qqmusicValue.duration;
    m_iPos = m_pShareMemory->WebAppValue.qqmusicValue.pos;
#else
    m_strlistPath = "";
    m_ID3Title = "叶杰盛";
    m_ID3Artist = "甜蜜蜜";
    m_ID3Album = "carry姐";
    m_iDuration = 361;
#endif
    setExistContent(!m_ID3Title.isEmpty());
    emit this->listPathChanged();
    emit this->ID3TitleChanged();
    emit this->ID3ArtistChanged();
    emit this->ID3AlbumChanged();
    emit this->DurationChanged();
    emit this->PosChanged();
    //setID3Pic("qrc:/AppHMI/General/Music/Album_Default.png");
}

void QQMusicService::updateResultList()
{
    SVP_INFO_FUNC();
    int hightlight = -1;
    m_searchResult.clear();
#ifdef IMPORT_LIB_APPINTERFACE
    std::vector<string> vecname;			//文件名
    std::vector<unsigned int> vecfiletype;	//文件类型

    int loadstatus = m_pShareMemory->WebAppValue.qqmusicValue.SearchloadStatus;
    setsearchstate(loadstatus);
    m_pLargeValueManager->GetValue( vecname, NotifyGeneralCommunication::ID_WEBAPP_QQMUSICSEARCH_LIST_NAME );
    m_pLargeValueManager->GetValue( vecfiletype, NotifyGeneralCommunication::ID_WEBAPP_QQMUSICSEARCH_LIST_TYPE );

    for ( int i = 0; i < vecname.size(); ++i )
    {
        QVariantMap result;
        result["id"] = 0;
        result["filetype"] = vecfiletype[i];
        result["name"] = vecname[i].c_str();
        if( m_ID3Title == result["name"].toString() )
            hightlight = i;
        m_searchResult.push_back( result );
    }
#else
    for(int i = 0; i < 20; i++)
    {
        QVariantMap result;
        result["id"] = 0;
        if(i%2 == 0)
        {
             result["filetype"] = 1;
        }
        else
        {
             result["filetype"] = 2;
        }


        result["name"] =  "FileName";
        m_searchResult.push_back( result );
    }
#endif
    emit this->searchresultChanged(m_searchResult,hightlight);
}

void QQMusicService::updateFileListData()
{

    SVP_INFO_FUNC();
    int hightlight = -1;
    m_fileListData.clear();

#ifdef IMPORT_LIB_APPINTERFACE
    m_strlistPath = m_pShareMemory->WebAppValue.qqmusicValue.ListPath;
    std::vector<string> vecname;			//文件名
    std::vector<unsigned int> vecfiletype;	//文件类型

    int loadstatus = m_pShareMemory->WebAppValue.qqmusicValue.ListDownloadStatus;
    setlistloadstatus(loadstatus);
    m_pLargeValueManager->GetValue( vecname, NotifyGeneralCommunication::ID_WEBAPP_QQMUSICLIST_FILE_NAME );
    m_pLargeValueManager->GetValue( vecfiletype, NotifyGeneralCommunication::ID_WEBAPP_QQMUSICLIST_FILE_TYPE );

    for ( int i = 0; i < vecname.size(); ++i )
    {

        QVariantMap result;
        result["id"] = 0;
        result["filetype"] = vecfiletype[i];
        result["name"] = vecname[i].c_str();
        if( m_ID3Title == result["name"].toString() )
            hightlight = i;
        m_fileListData.push_back( result );
    }
#else
    for(int i = 0; i < 20; i++)
    {
        QVariantMap result;
        result["id"] = 0;
        if(i%2 == 0)
        {
             result["filetype"] = 1;
        }
        else
        {
             result["filetype"] = 2;
        }


        result["name"] =  "FileName";
        m_fileListData.push_back( result );
    }
#endif


//    for (int i = 0; i < m_fileListData.size(); i++)
//    {
//        QVariantMap item = m_fileListData[i].toMap();
//        SVP_INFO(QString("address: %1, signl : %5")
//                       .arg(item.value(QStringLiteral("name")).toString())
//                       .arg(item.value(QStringLiteral("filetype")).toInt())
//                       );
//    }
  emit this->listPathChanged();
  emit this->fileListChanged(m_fileListData,hightlight);
}
void QQMusicService::listClick(int index)
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_pShareMemory->WebAppValue.qqmusicValue.ListClickIndex = index;
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_LIST_ITEM_SELECT);
#endif
}

void QQMusicService::selectResult(int index)
{
#ifdef IMPORT_LIB_APPINTERFACE
    m_pShareMemory->WebAppValue.qqmusicValue.SearchClickIndex = index;
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_SEARCH_SELECT_LIST);
#endif
}

void QQMusicService::searchSong(QString keyword)
{
#ifdef IMPORT_LIB_APPINTERFACE
    char *pword = m_pShareMemory->WebAppValue.qqmusicValue.keywords;
    strncpy( pword, keyword.toStdString().c_str(), keyword.size() );
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_SEARCH_KEYWORD);
#else
    updateResultList();
#endif
}


void QQMusicService::playPause()
{
    SVP_INFO_FUNC();

#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_PLAY_PAUSE );
#endif
}

void QQMusicService::previous()
{
    SVP_INFO_FUNC();

#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_PREVIOUS );
#endif
}

void QQMusicService::next()
{
    SVP_INFO_FUNC();

#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_NEXT );
#else
//    static int g_index = 1;
//    QString str_testindex = QString("/tmp/qqmusic_output%1").arg(g_index++);
//    SVP_INFO("[GYH] === "+str_testindex);
//    QImage tmp(str_testindex);
//    QQMusicImage::GetInstance()->g_qqmusicImage = tmp;
//    emit id3PicChanged();
#endif
}

void QQMusicService::repeat()
{
    SVP_INFO_FUNC();

#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_REPEAT );
#else
    static int change = 1;
    change = change % 2 + 1;
    setRepeatShuffleMode(change);
#endif
}

void QQMusicService::shuffle()
{
    SVP_INFO_FUNC();

#ifdef IMPORT_LIB_APPINTERFACE
    AppInterfaceService::sendNotifyForSoftKey( NotifyScreenKeyData::QQMUSIC_SHUFFLE );
#endif
}
