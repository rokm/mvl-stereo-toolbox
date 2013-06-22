/*
 * Image File Pair Source: image capture
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
 
#include "ImageFileCapture.h"
#include "ImageFileCaptureConfigWidget.h"

#include <opencv2/highgui/highgui.hpp>


ImageFileCapture::ImageFileCapture (QObject *parent)
    : QObject(parent)
{
    isRemote = false;
    waitingForReply = false;

    network = new QNetworkAccessManager(this);
    connect(network, SIGNAL(finished(QNetworkReply *)), this, SLOT(processRemoteReply(QNetworkReply *)));
}

ImageFileCapture::~ImageFileCapture ()
{
}


QWidget *ImageFileCapture::createConfigWidget (QWidget *parent)
{
    return new ImageFileCaptureConfigWidget(this, parent);
}


// *********************************************************************
// *                           Image loading                           *
// *********************************************************************
void ImageFileCapture::setImageFileOrUrl (const QString &name, bool remote)
{
    // Set
    fileNameOrUrl = name;
    isRemote = remote;

    // Load image via periodic refresh function
    refreshImage();
}


void ImageFileCapture::imageLoadingError (const QString &errorDescription)
{
    // Clear image
    frameBufferLock.lockForWrite();
    frameBuffer = cv::Mat();
    frameBufferLock.unlock();

    // Emit error
    qWarning() << qPrintable(errorDescription);
    emit error(errorDescription);
}


void ImageFileCapture::refreshImage ()
{
    // Make sure we have filename or URL
    if (fileNameOrUrl.isEmpty()) {
        return;
    }

    // Load
    if (isRemote) {
        loadRemoteImage();
    } else {
        loadLocalImage();
    }
}

void ImageFileCapture::loadLocalImage ()
{
    try {
        // Load both images and amit the change signal
        QWriteLocker locker(&frameBufferLock);
        frameBuffer = cv::imread(fileNameOrUrl.toStdString(), -1);
    } catch (std::exception e) {
        imageLoadingError(QString("Error while loading images: %1").arg(QString::fromStdString(e.what())));
    }

    emit imageReady();
}

void ImageFileCapture::loadRemoteImage ()
{
    if (waitingForReply) {
        //qWarning() << "Previous request for remote image has not finished yet!";
        return;
    }

    // Submit request
    waitingForReply = true;
    network->get(QNetworkRequest(fileNameOrUrl));
}

void ImageFileCapture::processRemoteReply (QNetworkReply *reply)
{
    waitingForReply = false;

    if (reply->error() != QNetworkReply::NoError) {
        imageLoadingError(QString("Error while retrieving left image; network error code %1").arg(reply->error()));
    } else {
        QByteArray payload = reply->readAll();
        reply->deleteLater();

        // Set output image, and decode
        try {
            QWriteLocker locker(&frameBufferLock);
            cv::imdecode(cv::Mat(1, payload.size(), CV_8UC1, payload.data()), -1, &frameBuffer);
        } catch (std::exception e) {
            imageLoadingError(QString("Error while decoding retrieved image: %1").arg(QString::fromStdString(e.what())));
        }
    }

    emit imageReady();
}


// *********************************************************************
// *                           Image access                            *
// *********************************************************************
void ImageFileCapture::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}


// *********************************************************************
// *                         Image properties                          *
// *********************************************************************
const QString &ImageFileCapture::getImageFilename ()
{
    return fileNameOrUrl;
}

int ImageFileCapture::getImageWidth ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.cols;
}

int ImageFileCapture::getImageHeight ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.rows;
}

int ImageFileCapture::getImageChannels ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.channels();
}
