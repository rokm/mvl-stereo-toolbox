/*
 * Unicap Source: source
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__SOURCE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__SOURCE_H

#include <stereo-pipeline/image_pair_source.h>

#include <unicap.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


class Camera;

class Source : public QAbstractListModel, public ImagePairSource
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::ImagePairSource)

public:
    Source (QObject * = 0);
    virtual ~Source ();

    virtual QString getShortName () const;
    virtual void getImages (cv::Mat &, cv::Mat &);
    virtual void stopSource ();
    virtual QWidget *createConfigWidget (QWidget * = 0);

    int getNumberOfCameras () const;
    const unicap_device_t &getCameraInfo (int) const;
    void setLeftCamera (int);
    void setRightCamera (int);

    Camera *getLeftCamera ();
    Camera *getRightCamera ();

    // Model
    virtual int rowCount (const QModelIndex &) const;
    virtual Qt::ItemFlags flags (const QModelIndex &index) const;
    virtual QVariant data (const QModelIndex &index, int role) const;

    void refreshCameraList ();
    void startStopCapture (bool);

protected:
    void createCamera (Camera *&, int);
    void releaseCamera (Camera *&);
    void setActive (int, bool);
    void setActive (const unicap_device_t &, bool);

    void synchronizeFrames ();

signals:
    void leftCameraChanged ();
    void rightCameraChanged ();

    // Signals from interface
    void imagesChanged ();
    void error (QString);

protected:
    QVector<unicap_device_t> entries;
    QVector<bool> active;

    Camera *leftCamera;
    Camera *rightCamera;

    bool leftFrameReady, rightFrameReady;

    // Images
    QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;
};


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL


#endif
