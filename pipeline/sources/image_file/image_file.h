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

#ifndef IMAGE_FILE_H
#define IMAGE_FILE_H

#include <QtCore>
#include <QtNetwork>
#include <opencv2/core.hpp>


namespace SourceImageFile {

class ImageFile : public QObject
{
    Q_OBJECT

public:
    ImageFile (QObject * = 0);
    virtual ~ImageFile ();

    QWidget *createConfigWidget (QWidget * = 0);
    
    const QString &getImageFilename ();
    int getImageWidth ();
    int getImageHeight ();
    int getImageChannels ();

    void copyFrame (cv::Mat &);

public:
    void setImageFileOrUrl (const QString &, bool);

    void refreshImage ();

protected:
    void imageLoadingError (const QString &);
    void loadLocalImage ();
    void loadRemoteImage ();

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

}

#endif
