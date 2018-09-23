#include "qqmusicimage.h"

QQMusicImage * QQMusicImage::m_pInstance = NULL;
QQuickImageProvider * QQMusicImage::m_pBaseInstance = NULL;

QImage QQMusicImage::requestImage(const QString &id, QSize *size, const QSize& requestedSize)
{
     if(requestedSize.isValid())
         g_qqmusicImage.scaled(requestedSize);
     return g_qqmusicImage;
}
QQuickImageProvider* QQMusicImage::GetBaseInstance()
{
    if( NULL == m_pBaseInstance )
    {
        m_pBaseInstance = static_cast<QQuickImageProvider *>(GetInstance());
    }
    return( m_pBaseInstance );
}
QQMusicImage* QQMusicImage::GetInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new QQMusicImage(QQmlImageProviderBase::Image);
    }
    return( m_pInstance );
}
