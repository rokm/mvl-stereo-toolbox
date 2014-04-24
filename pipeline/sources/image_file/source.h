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

#ifndef SOURCE_IMAGE_FILE_H
#define SOURCE_IMAGE_FILE_H

#include <QtNetwork>

#include <image_pair_source.h>


namespace SourceImageFile {

class ImageFile;

class Source : public QObject, public ImagePairSource
{
    Q_OBJECT
    Q_INTERFACES(ImagePairSource)

public:
    Source (QObject * = 0);
    virtual ~Source ();

    virtual QString getShortName () const;
    virtual void getImages (cv::Mat &, cv::Mat &);
    virtual void stopSource ();
    virtual QWidget *createConfigWidget (QWidget * = 0);

    ImageFile *getLeftImageFile ();
    ImageFile *getRightImageFile ();

    void loadImagePair (const QString &, const QString &, bool);

    bool getPeriodicRefreshState () const;
    int getRefreshPeriod () const;

public slots:
    void setPeriodicRefreshState (bool);
    void setRefreshPeriod (int);

protected slots:
    void periodicRefresh ();
    void synchronizeFrames ();

signals:
    void periodicRefreshStateChanged (bool);
    void refreshPeriodChanged (int);

    // Signals from interface
    void imagesChanged ();
    void error (const QString);

protected:
    QTimer *refreshTimer;
    int refreshPeriod;

    ImageFile *leftImageFile;
    ImageFile *rightImageFile;

    bool leftImageReady, rightImageReady;

    // Images
    QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;
};

}

#endif
