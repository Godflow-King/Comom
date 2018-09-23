#ifndef QQMUSIC_SERVICE_H
#define QQMUSIC_SERVICE_H

#include <QVariant>
#include "appinterfaceservice.h"
#include "qqmusicimage.h"

#define REGISTER_QQMUSIC_SERVICE( engine ) \
    do{ engine.addImageProvider(QLatin1String("imageProvider"), new QQMusicImage(QQmlImageProviderBase::Image)); }while(0); \
    qmlRegisterType<QQMusicService>("QQMusic",1,0,"QQMusicService");

class QQMusicService : public AppInterfaceService
{
    Q_OBJECT
    Q_DISABLE_COPY(QQMusicService)

    Q_PROPERTY(int listloadstatus READ getlistloadstatus WRITE setlistloadstatus NOTIFY listloadstatusChanged)
    Q_PROPERTY(int searchstate READ getsearchstate WRITE setsearchstate NOTIFY searchstateChanged)
    Q_PROPERTY(QString listPath READ getlistPath  WRITE setlistPath NOTIFY listPathChanged)
    Q_PROPERTY(QString ID3Title READ getID3Title  WRITE setID3Title NOTIFY ID3TitleChanged)
    Q_PROPERTY(QString ID3Artist READ getID3Artist WRITE setID3Artist NOTIFY ID3ArtistChanged)
//    Q_PROPERTY(QString ID3Pic READ getID3Pic WRITE setID3Pic NOTIFY ID3PicChanged)
    Q_PROPERTY(QString ID3Album READ getID3Album WRITE setID3Album NOTIFY ID3AlbumChanged)
    Q_PROPERTY(int Duration READ getDuration WRITE setDuration NOTIFY DurationChanged)

    Q_PROPERTY(bool PlayStatus READ getPlayStatus WRITE setPlayStatus NOTIFY PlayStatusChanged)
    Q_PROPERTY(int RepeatShuffleMode READ getRepeatShuffleMode WRITE setRepeatShuffleMode NOTIFY RepeatShuffleModeChanged)
    Q_PROPERTY(int pos READ getPos WRITE setPos NOTIFY PosChanged)
    Q_PROPERTY(bool isExistPlayContent READ getContent WRITE setExistContent NOTIFY ContentChanged)

public:
    QQMusicService();
    ~QQMusicService();

signals:

    void searchstateChanged(const int listloadstatus);
    void listloadstatusChanged(const int listloadstatus);
    void searchresultChanged(const QVariantList &listData,const int highlightIndex);
    void fileListChanged(const QVariantList &listData,const int highlightIndex); //文件列表数据变化
    void qqMusicID3Changed();
    void listPathChanged();
    void ID3TitleChanged();
    void ID3ArtistChanged();
    void id3PicChanged();
    void ID3AlbumChanged();
    void DurationChanged();

    void PlayStatusChanged();
    void RepeatShuffleModeChanged();
    void PosChanged();

    void ContentChanged();

public slots:
    void init(); //初始化
    void updateFileListData(); //更新文件列表数据
    void updateResultList();
    void cancleConnectPhone();//取消连接手机

    void updateQQMusicID3();
    void updateQQMusicPic();

    void updatePlayStatus();
    void updateRepeatStatus();
    void updatePos();

    void listClick(int index);
    void searchSong(QString keyword);
    void selectResult(int index);


    void playPause();
    void previous();
    void next();
    void repeat();
    void shuffle();
public:

    QString getlistPath() const { return m_strlistPath; }
    QString getID3Title() const { return m_ID3Title; }
    QString getID3Artist() const { return m_ID3Artist; }
//    QString getID3Pic() const { return m_ID3Pic; }
    QString getID3Album() const { return m_ID3Album; }
    int getDuration() const { return m_iDuration; }
    bool getPlayStatus() const { return m_iPlayStatus; }
    int getRepeatShuffleMode() const { return m_iRepeatShuffleMode; }
    int getPos() const { return m_iPos; }
    bool getContent() const { return m_isExistPalyContent;}

    void setlistPath(QString path) { m_strlistPath = path; emit listPathChanged();}
    void setID3Title(QString Title) { m_ID3Title = Title; emit ID3TitleChanged();}
    void setID3Artist(QString Artist) { m_ID3Artist = Artist; emit ID3ArtistChanged();}
//    void setID3Pic(QString picpath) { m_ID3Pic = picpath; emit ID3PicChanged();}
    void setID3Album(QString Album) { m_ID3Album = Album; emit ID3AlbumChanged();}
    void setDuration(int duration) { m_iDuration = duration; emit DurationChanged();}
    void setPlayStatus(bool Status) { m_iPlayStatus = Status; emit PlayStatusChanged();}
    void setRepeatShuffleMode(int Status) { m_iRepeatShuffleMode = Status; emit RepeatShuffleModeChanged();}
    int  getlistloadstatus() const { return m_ilistLoadstatus; }
    void setlistloadstatus(int status) { m_ilistLoadstatus = status; emit listloadstatusChanged(m_ilistLoadstatus);}
    int getsearchstate() const { return m_searchstate; }
    void setsearchstate(int status) { m_searchstate = status; emit searchstateChanged(m_searchstate);}
    void setExistContent(bool isExist) { m_isExistPalyContent = isExist;emit ContentChanged();}

    void setPos(int pos) { m_iPos = pos; emit PosChanged();}
private:
#ifdef IMPORT_LIB_APPINTERFACE
    ShareMemoryType * m_pShareMemory;
    LargeValueManager * m_pLargeValueManager;
#endif

    QString m_ID3Title,m_ID3Artist/*,m_ID3Pic*/,m_ID3Album,m_strlistPath;
    int m_iDuration;
    QVariantList m_fileListData;
    QVariantList m_searchResult;
    bool m_iPlayStatus;
    bool m_isExistPalyContent;
    int m_iRepeatShuffleMode;
    int m_iPos;
    int m_ilistLoadstatus;
    int m_searchstate;
};

#endif // QQMUSIC_SERVICE_H
