/*
 * Image File Pair Source: source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "SourceVideoFile.h"
#include "SourceVideoFileConfigWidget.h"
#include "VideoFileCapture.h"

#include <opencv2/highgui/highgui.hpp>


SourceVideoFile::SourceVideoFile (QObject *parent)
    : ImagePairSource(parent)
{
    shortName = "VIDEO";

    leftFrameReady = rightFrameReady = false;

    leftVideoCapture = new VideoFileCapture(this);
    connect(leftVideoCapture, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

    rightVideoCapture = new VideoFileCapture(this);    
    connect(rightVideoCapture, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));
}

SourceVideoFile::~SourceVideoFile ()
{
    // Stop capture
    setPlaybackState(false);
}


QWidget *SourceVideoFile::createConfigWidget (QWidget *parent)
{
    return new SourceVideoFileConfigWidget(this, parent);
}


void SourceVideoFile::stopSource ()
{
    // Stop periodic refresh
    setPlaybackState(false);
}


void SourceVideoFile::loadVideoPair (const QString &left, const QString &right)
{
    // Loading images stops the periodic update
    setPlaybackState(false);

    // Set videos
    leftVideoCapture->setVideoFile(left);
    rightVideoCapture->setVideoFile(right);
}


VideoFileCapture *SourceVideoFile::getLeftVideoCapture ()
{
    return leftVideoCapture;
}

VideoFileCapture *SourceVideoFile::getRightVideoCapture ()
{
    return rightVideoCapture;
}


// *********************************************************************
// *                             Playback                              *
// *********************************************************************
void SourceVideoFile::setPlaybackState (bool enable)
{
    if (enable != getPlaybackState()) {
        leftVideoCapture->setCaptureState(enable);
        rightVideoCapture->setCaptureState(enable);

        emit playbackStateChanged(enable);
    }
}

bool SourceVideoFile::getPlaybackState () const
{
    return leftVideoCapture->getCaptureState() && rightVideoCapture->getCaptureState();
}


void SourceVideoFile::synchronizeFrames ()
{
    if (QObject::sender() == leftVideoCapture) {
        leftFrameReady = true;
    } else if (QObject::sender() == rightVideoCapture) {
        rightFrameReady = true;
    }
    
    bool requireLeft = !leftVideoCapture->getVideoFilename().isEmpty();
    bool requireRight = !rightVideoCapture->getVideoFilename().isEmpty();

    // We do frame synchronization only if periodic refresh is running
    if ((!requireLeft || leftFrameReady) && (!requireRight || rightFrameReady)) {
        if (requireLeft) {
            leftVideoCapture->copyFrame(imageLeft);
        } else {
            imageLeft = cv::Mat();
        }
        if (requireRight) {
            rightVideoCapture->copyFrame(imageRight);
        } else {
            imageRight = cv::Mat();
        }
        leftFrameReady = false;
        rightFrameReady = false;

        emit imagesChanged();
    }
}
