/*
 * Image File Pair Source: source
 * Copyright (C) 2013 Rok Mandeljc
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
#include "image_file.h"

#include <opencv2/highgui/highgui.hpp>

using namespace SourceImageFile;


Source::Source (QObject *parent)
    : QObject(parent), ImagePairSource()
{
    refreshPeriod = 1000;
    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(periodicRefresh()));

    leftImageReady = rightImageReady = false;

    leftImageFile = new ImageFile(this);
    connect(leftImageFile, SIGNAL(imageReady()), this, SLOT(synchronizeFrames()));

    rightImageFile = new ImageFile(this);    
    connect(rightImageFile, SIGNAL(imageReady()), this, SLOT(synchronizeFrames()));
}

Source::~Source ()
{
}


// *********************************************************************
// *                     ImagePairSource interface                     *
// *********************************************************************
QString Source::getShortName () const
{
    return "IMAGE";
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
    // Stop periodic refresh
    setPeriodicRefreshState(false);
}

QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


// *********************************************************************
// *                           Image loading                           *
// *********************************************************************
void Source::loadImagePair (const QString &left, const QString &right, bool remote)
{
    // Loading images stops the periodic update
    setPeriodicRefreshState(false);

    // Set images
    leftImageFile->setImageFileOrUrl(left, remote);
    rightImageFile->setImageFileOrUrl(right, remote);

    // Get images
    periodicRefresh();
}


ImageFile *Source::getLeftImageFile ()
{
    return leftImageFile;
}

ImageFile *Source::getRightImageFile ()
{
    return rightImageFile;
}


// *********************************************************************
// *                         Periodic refresh                          *
// *********************************************************************
void Source::setPeriodicRefreshState (bool enable)
{
    if (enable == refreshTimer->isActive()) {
        return;
    }

    if (enable) {
        refreshTimer->start(refreshPeriod);
        periodicRefresh();
    } else {
        refreshTimer->stop();
    }

    emit periodicRefreshStateChanged(enable);
}

bool Source::getPeriodicRefreshState () const
{
    return refreshTimer->isActive();
}

int Source::getRefreshPeriod () const
{
    return refreshPeriod;
}

void Source::setRefreshPeriod (int newPeriod)
{
    if (refreshPeriod == newPeriod) {
        return;
    }

    refreshPeriod = newPeriod;

    // Restart timer if it is running
    if (refreshTimer->isActive()) {
        refreshTimer->start(refreshPeriod);
    }

    emit refreshPeriodChanged(refreshPeriod);
}


void Source::periodicRefresh ()
{
    // Clear both
    leftImageReady = false;
    rightImageReady = false;
    
    // Get images
    leftImageFile->refreshImage();
    rightImageFile->refreshImage();
}


void Source::synchronizeFrames ()
{
    if (QObject::sender() == leftImageFile) {
        leftImageReady = true;
    } else if (QObject::sender() == rightImageFile) {
        rightImageReady = true;
    }
    
    bool requireLeft = !leftImageFile->getImageFilename().isEmpty();
    bool requireRight = !rightImageFile->getImageFilename().isEmpty();

    if (((!requireLeft || leftImageReady) && (!requireRight || rightImageReady))) {
        if (requireLeft) {
            leftImageFile->copyFrame(imageLeft);
        } else {
            imageLeft = cv::Mat();
        }
        if (requireRight) {
            rightImageFile->copyFrame(imageRight);
        } else {
            imageRight = cv::Mat();
        }
        leftImageReady = false;
        rightImageReady = false;

        emit imagesChanged();
    }
}
