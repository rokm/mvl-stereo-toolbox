/*
 * Image File Pair Source: image file
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
 
#include "image_file.h"
#include "image_file_widget.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace SourceImageFile;


ImageFile::ImageFile (QObject *parent)
    : QObject(parent)
{
    isRemote = false;
    waitingForReply = false;

    network = new QNetworkAccessManager(this);
    connect(network, &QNetworkAccessManager::finished, this, &ImageFile::processRemoteReply);
}

ImageFile::~ImageFile ()
{
}


QWidget *ImageFile::createConfigWidget (QWidget *parent)
{
    return new ImageFileWidget(this, parent);
}


// *********************************************************************
// *                           Image loading                           *
// *********************************************************************
void ImageFile::setImageFileOrUrl (const QString &name, bool remote)
{
    // Set
    fileNameOrUrl = name;
    isRemote = remote;
}


void ImageFile::imageLoadingError (const QString &errorDescription)
{
    // Clear image
    frameBufferLock.lockForWrite();
    frameBuffer = cv::Mat();
    frameBufferLock.unlock();

    // Emit error
    qWarning() << qPrintable(errorDescription);
    emit error(errorDescription);
}


void ImageFile::refreshImage ()
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

void ImageFile::loadLocalImage ()
{
    try {
        // Load both images and amit the change signal
        QWriteLocker locker(&frameBufferLock);
        frameBuffer = cv::imread(fileNameOrUrl.toStdString(), -1);
        if (frameBuffer.channels() == 4) {
            // Strip alpha channel
            cv::cvtColor(frameBuffer, frameBuffer, cv::COLOR_BGRA2BGR);
        }
    } catch (std::exception e) {
        imageLoadingError(QString("Error while loading images: %1").arg(QString::fromStdString(e.what())));
    }

    emit imageReady();
}

void ImageFile::loadRemoteImage ()
{
    if (waitingForReply) {
        //qWarning() << "Previous request for remote image has not finished yet!";
        return;
    }

    // Submit request
    waitingForReply = true;
    network->get(QNetworkRequest(fileNameOrUrl));
}

void ImageFile::processRemoteReply (QNetworkReply *reply)
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
            if (frameBuffer.channels() == 4) {
                // Strip alpha channel
                cv::cvtColor(frameBuffer, frameBuffer, cv::COLOR_BGRA2BGR);
            }
        } catch (std::exception e) {
            imageLoadingError(QString("Error while decoding retrieved image: %1").arg(QString::fromStdString(e.what())));
        }
    }

    emit imageReady();
}


// *********************************************************************
// *                           Image access                            *
// *********************************************************************
void ImageFile::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}


// *********************************************************************
// *                         Image properties                          *
// *********************************************************************
const QString &ImageFile::getImageFilename ()
{
    return fileNameOrUrl;
}

int ImageFile::getImageWidth ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.cols;
}

int ImageFile::getImageHeight ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.rows;
}

int ImageFile::getImageChannels ()
{
    QReadLocker locker(&frameBufferLock);
    return frameBuffer.channels();
}
