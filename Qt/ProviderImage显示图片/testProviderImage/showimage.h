#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include <QObject>
#include <QImage>
#include "imageprovider.h"

class ShowImage : public QObject
{
    Q_OBJECT
public:
    explicit ShowImage(QObject *parent = nullptr);
    ImageProvider *m_pImgProvider;
public slots:
    void setImage(QImage image);
    void testShow();
signals:
    void callQmlRefeshImg();
private:
    void getImage(QString path,QImage &outimage);
};

#endif // SHOWIMAGE_H
