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

#include "SourceImageFile.h"
#include "SourceImageFileConfigWidget.h"
#include "ImageFileCapture.h"

#include <opencv2/highgui/highgui.hpp>


SourceImageFile::SourceImageFile (QObject *parent)
    : ImagePairSource(parent)
{
    shortName = "IMAGE";

    refreshPeriod = 1000;
    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(periodicRefresh()));

    leftImageReady = rightImageReady = false;

    leftImageCapture = new ImageFileCapture(this);
    connect(leftImageCapture, SIGNAL(imageReady()), this, SLOT(synchronizeFrames()));

    rightImageCapture = new ImageFileCapture(this);    
    connect(rightImageCapture, SIGNAL(imageReady()), this, SLOT(synchronizeFrames()));
}

SourceImageFile::~SourceImageFile ()
{
}


QWidget *SourceImageFile::createConfigWidget (QWidget *parent)
{
    return new SourceImageFileConfigWidget(this, parent);
}


void SourceImageFile::stopSource ()
{
    // Stop periodic refresh
    setPeriodicRefreshState(false);
}


void SourceImageFile::loadImagePair (const QString &left, const QString &right, bool remote)
{
    // Loading images stops the periodic update
    setPeriodicRefreshState(false);

    // Set images
    leftImageCapture->setImageFileOrUrl(left, remote);
    rightImageCapture->setImageFileOrUrl(right, remote);
}


ImageFileCapture *SourceImageFile::getLeftImageCapture ()
{
    return leftImageCapture;
}

ImageFileCapture *SourceImageFile::getRightImageCapture ()
{
    return rightImageCapture;
}


// *********************************************************************
// *                         Periodic refresh                          *
// *********************************************************************
void SourceImageFile::setPeriodicRefreshState (bool enable)
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

bool SourceImageFile::getPeriodicRefreshState () const
{
    return refreshTimer->isActive();
}

int SourceImageFile::getRefreshPeriod () const
{
    return refreshPeriod;
}

void SourceImageFile::setRefreshPeriod (int newPeriod)
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


void SourceImageFile::periodicRefresh ()
{
    // Get images
    leftImageCapture->refreshImage();
    rightImageCapture->refreshImage();
}


void SourceImageFile::synchronizeFrames ()
{
    if (QObject::sender() == leftImageCapture) {
        leftImageReady = true;
    } else if (QObject::sender() == rightImageCapture) {
        rightImageReady = true;
    }
    
    bool requireLeft = !leftImageCapture->getImageFilename().isEmpty();
    bool requireRight = !rightImageCapture->getImageFilename().isEmpty();

    // We do frame synchronization only if periodic refresh is running
    if (!getPeriodicRefreshState() || ((!requireLeft || leftImageReady) && (!requireRight || rightImageReady))) {
        if (requireLeft) {
            leftImageCapture->copyFrame(imageLeft);
        } else {
            imageLeft = cv::Mat();
        }
        if (requireRight) {
            rightImageCapture->copyFrame(imageRight);
        } else {
            imageRight = cv::Mat();
        }
        leftImageReady = false;
        rightImageReady = false;

        emit imagesChanged();
    }
}
