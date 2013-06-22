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
