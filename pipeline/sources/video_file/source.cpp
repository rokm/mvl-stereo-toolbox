/*
 * Video File Source: source
 * Copyright (C) 2014 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "source.h"
#include "source_widget.h"

using namespace SourceVideoFile;


Source::Source (QObject *parent)
    : QObject(parent), ImagePairSource()
{
    playbackTimer = new QTimer(this);
    connect(playbackTimer, SIGNAL(timeout()), this, SLOT(playbackFunction()));
}

Source::~Source ()
{
}


// *********************************************************************
// *                     ImagePairSource interface                     *
// *********************************************************************
QString Source::getShortName () const
{
    return "VIDEO";
}

void Source::getImages (cv::Mat &left, cv::Mat &right)
{
    // Copy images under lock
    QReadLocker lock(&imagesLock);
    imageLeft.copyTo(left);
    imageRight.copyTo(right);
}

void Source::stopSource ()
{
    stopPlayback();
}

QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


// *********************************************************************
// *                            Video file                             *
// *********************************************************************
void Source::openVideoFile (const QString &filename)
{
    // Make sure playback is stopped
    stopPlayback();

    // Open video file
    video.open(filename.toStdString());
    if (!video.isOpened()) {
	emit error(QString("Failed to open video '%1'").arg(filename));
	emit videoFileReadyChanged(false);
    }

    emit videoFileReadyChanged(true);
    emit videoPositionChanged(video.get(CV_CAP_PROP_POS_FRAMES), video.get(CV_CAP_PROP_FRAME_COUNT));
}

int Source::getVideoWidth ()
{
    return video.get(CV_CAP_PROP_FRAME_WIDTH);
}

int Source::getVideoHeight ()
{
    return video.get(CV_CAP_PROP_FRAME_HEIGHT);
}

float Source::getVideoFps ()
{
    return video.get(CV_CAP_PROP_FPS);
}

int Source::getVideoLength ()
{
    return video.get(CV_CAP_PROP_FRAME_COUNT);
}


// *********************************************************************
// *                             Playback                              *
// *********************************************************************
void Source::stopPlayback ()
{
    playbackTimer->stop();
    emit playbackStateChanged(false);
}

void Source::startPlayback ()
{
    // Make sure video is open
    if (!video.isOpened()) {
	stopPlayback();
	return;
    }

    // Start playback timer with specified FPS
    float fps = getVideoFps();
    if (!fps) {
	fps = 25;
    }
    playbackTimer->start(1000/fps);

    emit playbackStateChanged(true);
}

void Source::setVideoPosition (int frame)
{
    video.set(CV_CAP_PROP_POS_FRAMES, frame);

    // Get frame
    playbackFunction();
}


void Source::playbackFunction ()
{
    // Grab next frame
    cv::Mat frame;

    if (!video.grab()) {
	stopPlayback();
	return;
    }

    // Decode frame
    video.retrieve(frame);
    emit videoPositionChanged(video.get(CV_CAP_PROP_POS_FRAMES), video.get(CV_CAP_PROP_FRAME_COUNT));

    // Update images
    imagesLock.lockForWrite();

    frame(cv::Rect(0, 0, frame.cols/2, frame.rows)).copyTo(imageLeft);
    frame(cv::Rect(frame.cols/2, 0, frame.cols/2, frame.rows)).copyTo(imageRight);

    imagesLock.unlock();

    emit imagesChanged();
}
