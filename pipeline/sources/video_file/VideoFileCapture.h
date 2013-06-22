#ifndef VIDEO_FILE_CAPTURE_H
#define VIDEO_FILE_CAPTURE_H

#include <QtCore>
#include <opencv2/highgui/highgui.hpp>


class VideoFileCapture : public QObject
{
    Q_OBJECT

public:
    VideoFileCapture (QObject * = 0);
    virtual ~VideoFileCapture ();

    QWidget *createConfigWidget (QWidget * = 0);
    
    const QString &getVideoFilename () const;
    int getFrameWidth ();
    int getFrameHeight ();
    float getFPS ();

    void copyFrame (cv::Mat &);

    bool getCaptureState () const;

public slots:
    void setVideoFile (const QString &);

    void setCaptureState (bool);

protected slots:
    void captureFunction ();

signals:
    void videoLoaded ();
    void frameReady ();
    void error (const QString);

    void captureStarted ();
    void captureStopped ();

protected:
    QString fileNameOrUrl;
    cv::VideoCapture videoCapture;

    // Capture thread
    bool captureActive;
    QFutureWatcher<void> captureWatcher;

    // Frame buffer
    QReadWriteLock frameBufferLock;
    cv::Mat frameBuffer;
};

#endif
