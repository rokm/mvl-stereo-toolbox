#ifndef IMAGE_FILE_CAPTURE_H
#define IMAGE_FILE_CAPTURE_H

#include <QtCore>
#include <QtNetwork>
#include <opencv2/core/core.hpp>


class ImageFileCapture : public QObject
{
    Q_OBJECT

public:
    ImageFileCapture (QObject * = 0);
    virtual ~ImageFileCapture ();

    QWidget *createConfigWidget (QWidget * = 0);
    
    const QString &getImageFilename ();
    int getImageWidth ();
    int getImageHeight ();
    int getImageChannels ();

    void copyFrame (cv::Mat &);

public slots:
    void setImageFileOrUrl (const QString &, bool);

    void refreshImage ();

protected:
    void imageLoadingError (const QString &);
    void loadLocalImage ();
    void loadRemoteImage ();

protected slots:
    void processRemoteReply (QNetworkReply *);

signals:
    void imageReady ();
    void error (const QString);

protected:
    QString fileNameOrUrl;
    bool isRemote;
    
    QNetworkAccessManager *network;
    bool waitingForReply;

    // Frame buffer
    QReadWriteLock frameBufferLock;
    cv::Mat frameBuffer;
};

#endif
