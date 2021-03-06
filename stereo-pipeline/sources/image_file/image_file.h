/*
 * Image File Source: image file
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__IMAGE_FILE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__IMAGE_FILE_H

#include <QtCore>
#include <QtNetwork>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceImageFile {


class ImageFile : public QObject
{
    Q_OBJECT

public:
    ImageFile (QObject *parent = nullptr);
    virtual ~ImageFile ();

    QWidget *createConfigWidget (QWidget *parent = nullptr);

    const QString &getImageFilename ();
    int getImageWidth ();
    int getImageHeight ();
    int getImageChannels ();

    void copyFrame (cv::Mat &frame);

public:
    void setImageFileOrUrl (const QString &name, bool remote);

    void refreshImage ();

protected:
    void imageLoadingError (const QString &message);
    void loadLocalImage ();
    void loadRemoteImage ();

    void processRemoteReply (QNetworkReply *reply);

signals:
    void imageReady ();
    void error (const QString message);

protected:
    QString fileNameOrUrl;
    bool isRemote;

    QNetworkAccessManager *network;
    bool waitingForReply;

    // Frame buffer
    QReadWriteLock frameBufferLock;
    cv::Mat frameBuffer;
};


} // SourceImageFile
} // Pipeline
} // StereoToolbox
} // MVL


#endif
