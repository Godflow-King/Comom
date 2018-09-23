#include "showimage.h"
#include <QDebug>
#include <fstream>
#include <sstream>

ShowImage::ShowImage(QObject *parent) : QObject(parent)
{
    m_pImgProvider = new ImageProvider();
}
void ShowImage::setImage(QImage image)
{
    m_pImgProvider->img = image;
    emit callQmlRefeshImg();
}

void ShowImage::getImage(QString path,QImage &outimage)
{
    std::ifstream m_stream(path.toStdString(),std::ios::in|std::ios::binary);
    if (!m_stream.is_open())
    {
        return;
    }
    int fstart = m_stream.tellg();
    m_stream.seekg (0, std::ios::end);
    int fend = m_stream.tellg();
    m_stream.seekg (0, std::ios::beg);
    int  fsize = fend - fstart;
    unsigned char picbuf[fsize];
    m_stream.read(reinterpret_cast<char *>(picbuf),fsize);
    m_stream.close();
    qDebug()<<(QString("Get size : %1").arg(fsize) );
    outimage.loadFromData(picbuf, fsize );
    if( ! outimage.save("/tmp/qqmusic_output.png","PNG") ) /* Maybe QQuickImageProvider was better !*/
    {
        qDebug()<< " updateQQMusicPic : outimage save error ";
        return;
    }
}

void ShowImage::testShow()
{
//    static int g_index = 1;
//    QString str_testindex = QString("/tmp/qqmusic_output%1").arg(g_index++);
//    qDebug()<<"str_testindex :" << str_testindex;
//    QImage tmp(str_testindex);
//    setImage(tmp);
    static int g_index = 1;
    QString str_testindex = QString("/tmp/qqmusic_output%1").arg(g_index++);
    qDebug()<<"str_testindex :" << str_testindex;
    QImage image;
    getImage(str_testindex,image);
    setImage(image);
}
