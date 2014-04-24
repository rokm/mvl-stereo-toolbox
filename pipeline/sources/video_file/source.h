/*
 * Video File Source: source
 * Copyright (C) 2014 Rok Mandeljc
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

#ifndef SOURCE_VIDEO_FILE_H
#define SOURCE_VIDEO_FILE_H

#include <QtConcurrent>

#include <image_pair_source.h>

#include <opencv2/highgui/highgui.hpp>


namespace SourceVideoFile {

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

    int getVideoWidth ();
    int getVideoHeight ();
    float getVideoFps ();
    int getVideoLength ();

public slots:
    void stopPlayback ();
    void startPlayback ();

    void setVideoPosition (int);

    void openVideoFile (const QString &);

protected slots:
    void playbackFunction ();

signals:
    // Signals from interface
    void imagesChanged ();
    void error (const QString);

    void playbackStateChanged (bool);
    void videoFileReadyChanged (bool);
    void videoPositionChanged (int, int);

protected:
    // Images
    QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;

    cv::VideoCapture video;

    // Playback thread
    QTimer *playbackTimer;
};

}

#endif
