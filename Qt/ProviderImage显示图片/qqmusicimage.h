#ifndef QQMUSICIMAGE_H
#define QQMUSICIMAGE_H

#include <QQuickImageProvider>
#include <QImage>
#include <QDebug>
#include "logutil.h"

class QQMusicImage : public QQuickImageProvider
{
public:
   QQMusicImage(ImageType type, Flags flags = Flags()) :
       QQuickImageProvider(type, flags)
   {
   }
   QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize);
   static QQuickImageProvider* GetBaseInstance();
   static QQMusicImage* GetInstance();

   QImage g_qqmusicImage;
   static QQMusicImage * m_pInstance;
   static QQuickImageProvider * m_pBaseInstance;
};

#endif // QQMUSICIMAGE_H
