#include "VideoFileCapture.h"
#include "VideoFileCaptureConfigWidget.h"

#include <unistd.h>


VideoFileCapture::VideoFileCapture (QObject *parent)
    : QObject(parent)
{
}

VideoFileCapture::~VideoFileCapture ()
{
}


QWidget *VideoFileCapture::createConfigWidget (QWidget *parent)
{
    return new VideoFileCaptureConfigWidget(this, parent);
}


// *********************************************************************
// *                           Image loading                           *
// *********************************************************************
void VideoFileCapture::setVideoFile (const QString &name)
{
    // Set
    fileNameOrUrl = name;

    // Load video
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
    videoCapture.open(fileNameOrUrl.toStdString());
    if (videoCapture.isOpened()) {
        emit videoLoaded();
    } else {
        emit error("Failed to open video!");
    }
}

void VideoFileCapture::setCaptureState (bool enable)
{
    if (enable) {
        if (!captureWatcher.isRunning()) {
            // Start capture function
            captureActive = true;
            QFuture<void> future = QtConcurrent::run(this, &VideoFileCapture::captureFunction);
            captureWatcher.setFuture(future);
        } else {
            qWarning() << this << "Capture already running!";
        }
    } else {
         if (captureWatcher.isRunning()) {
            captureActive = false;
 
            // Make sure capture thread finishes
            captureWatcher.waitForFinished();
        } else {
            qWarning() << this << "Capture not running!";
        }
    }
}

bool VideoFileCapture::getCaptureState () const
{
    return captureWatcher.isRunning();
}


void VideoFileCapture::captureFunction ()
{
    qDebug() << "Starting capture function in thread" << QThread::currentThread();
    emit captureStarted();

    double fps = videoCapture.get(CV_CAP_PROP_FPS);
    int sleepTime;

    QTime timer;
    
    while (captureActive) {
        timer.restart();

        // Grab and decode frame
        if (!videoCapture.grab()) {
            break;
        }
        QWriteLocker locker(&frameBufferLock);
        videoCapture.retrieve(frameBuffer);
        locker.unlock();
        emit frameReady();

        if (!frameBuffer.data) {
            break;
        }

        // Sleep
        sleepTime = 1000/fps - timer.elapsed();
        if (sleepTime > 0) {
            usleep(sleepTime*1000);
        }
    }

    emit captureStopped();
    qDebug() << "Finished capture function in thread" << QThread::currentThread();
}


// *********************************************************************
// *                           Frame access                            *
// *********************************************************************
void VideoFileCapture::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}


// *********************************************************************
// *                         Video properties                          *
// *********************************************************************
const QString &VideoFileCapture::getVideoFilename () const
{
    return fileNameOrUrl;
}

int VideoFileCapture::getFrameWidth ()
{
    return videoCapture.get(CV_CAP_PROP_FRAME_WIDTH);
}

int VideoFileCapture::getFrameHeight ()
{
    return videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT);
}

float VideoFileCapture::getFPS ()
{
    return videoCapture.get(CV_CAP_PROP_FPS);
}
