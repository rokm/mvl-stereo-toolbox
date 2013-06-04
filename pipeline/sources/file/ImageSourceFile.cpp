/*
 * File Image Source: source
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

#include "ImageSourceFile.h"
#include "ImageSourceFileConfigWidget.h"

#include <opencv2/highgui/highgui.hpp>


ImageSourceFile::ImageSourceFile (QObject *parent)
    : ImageSource(parent)
{
    shortName = "FILE";

    refreshPeriod = 1000;
    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(periodicRefresh()));

    remote = false;
    network = new QNetworkAccessManager(this);
    connect(network, SIGNAL(finished(QNetworkReply *)), this, SLOT(processRemoteReply(QNetworkReply *)));
    replyLeft = replyRight = NULL;
}

ImageSourceFile::~ImageSourceFile ()
{
}


QWidget *ImageSourceFile::createConfigWidget (QWidget *parent)
{
    return new ImageSourceFileConfigWidget(this, parent);
}


void ImageSourceFile::stopSource ()
{
    // Stop periodic refresh
    setPeriodicRefreshState(false);
}


void ImageSourceFile::loadImagePair (const QString &left, const QString &right, bool remoteAccess)
{
    filenameLeft = left;
    filenameRight = right;
    remote = remoteAccess;

    // Loading images stops the periodic update
    setPeriodicRefreshState(false);

    // Load images via periodicRefresh() function
    periodicRefresh();
}


// *********************************************************************
// *                          Left image info                          *
// *********************************************************************
const QString &ImageSourceFile::getLeftFilename () const
{
    return filenameLeft;
}

int ImageSourceFile::getLeftWidth () const
{
    return imageLeft.cols;
}

int ImageSourceFile::getLeftHeight () const
{
    return imageLeft.rows;
}

int ImageSourceFile::getLeftChannels () const
{
    return imageLeft.channels();
}


// *********************************************************************
// *                         Right image info                          *
// *********************************************************************
const QString &ImageSourceFile::getRightFilename () const
{
    return filenameRight;
}

int ImageSourceFile::getRightWidth () const
{
    return imageRight.cols;
}

int ImageSourceFile::getRightHeight () const
{
    return imageRight.rows;
}

int ImageSourceFile::getRightChannels () const
{
    return imageRight.channels();
}


// *********************************************************************
// *                         Periodic refresh                          *
// *********************************************************************
void ImageSourceFile::setPeriodicRefreshState (bool enable)
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

bool ImageSourceFile::getPeriodicRefreshState () const
{
    return refreshTimer->isActive();
}

int ImageSourceFile::getRefreshPeriod () const
{
    return refreshPeriod;
}

void ImageSourceFile::setRefreshPeriod (int newPeriod)
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



void ImageSourceFile::periodicRefresh ()
{
    if (remote) {
        loadRemoteImages();
    } else {
        loadLocalImages();
    }
}

void ImageSourceFile::imageLoadingError (const QString &errorDescription)
{
    qWarning() << qPrintable(errorDescription);
    emit error(errorDescription);

    // Stop periodic refresh (no-op if already inactive)
    setPeriodicRefreshState(false);
}

void ImageSourceFile::loadLocalImages ()
{
    try {
        // Load both images and amit the change signal
        imageLeft = cv::imread(filenameLeft.toStdString(), -1);
        imageRight = cv::imread(filenameRight.toStdString(), -1);
        emit imagesChanged();
    } catch (std::exception e) {
        imageLoadingError(QString("Error while loading images: %1").arg(QString::fromStdString(e.what())));
    }
}

void ImageSourceFile::loadRemoteImages ()
{
    // Make sure we are not already processing requests
    if ((replyLeft && replyLeft->isRunning()) || (replyRight && replyRight->isRunning())) {
        imageLoadingError("Still processing previous request for remote images!");
        return;
    }

    // Submit network requests
    replyLeft = network->get(QNetworkRequest(filenameLeft));
    replyRight = network->get(QNetworkRequest(filenameRight));
}

void ImageSourceFile::processRemoteReply (QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        imageLoadingError(QString("Error while retrieving left image; network error code %1").arg(reply->error()));
    } else {
        QByteArray payload = reply->readAll();
        reply->deleteLater();

        // Set output image, and decode
        cv::Mat *image;
        if (reply == replyLeft) {
            replyLeft = NULL;
            image = &imageLeft;
        } else {
            replyRight = NULL;
            image = &imageRight;
        }

        try {
            cv::imdecode(cv::Mat(1, payload.size(), CV_8UC1, payload.data()), -1, image);
        } catch (std::exception e) {
            imageLoadingError(QString("Error while decoding retrieved image: %1").arg(QString::fromStdString(e.what())));
        }
    }

    // If both images have been retrieved, send signal
    if (!replyLeft && !replyRight) {
        emit imagesChanged();
    }
}



