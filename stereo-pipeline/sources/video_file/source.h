/*
 * Video File Source: source
 * Copyright (C) 2014-2015 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__VIDEO_FILE__SOURCE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__VIDEO_FILE__SOURCE_H

#include <QtConcurrent>

#include <stereo-pipeline/image_pair_source.h>

#include <opencv2/videoio.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceVideoFile {


class Source : public QObject, public ImagePairSource
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::ImagePairSource)

public:
    Source (QObject *parent = nullptr);
    virtual ~Source ();

    virtual QString getShortName () const;
    virtual void getImages (cv::Mat &left, cv::Mat &right);
    virtual void stopSource ();
    virtual QWidget *createConfigWidget (QWidget *parent = nullptr);

    int getVideoWidth ();
    int getVideoHeight ();
    float getVideoFramerate ();
    int getVideoLength ();

    void stopPlayback ();
    void startPlayback ();

    void setVideoPosition (int frame);

    void openVideoFile (const QString &filename);

protected:
    void playbackFunction ();

signals:
    // Signals from interface
    void imagesChanged (cv::Mat imageL, cv::Mat imageR);
    void error (QString message);

    void playbackStateChanged (bool playing);
    void videoFileChanged (bool available);
    void videoPositionChanged (int position, int length);

protected:
    // Images
    QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;

    cv::VideoCapture video;

    // Playback thread
    QTimer *playbackTimer;
};


} // SourceVideoFile
} // Pipeline
} // StereoToolbox
} // MVL


#endif
