/*
 * OpenCV Camera Source: source
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__SOURCE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__SOURCE_H

#include <stereo-pipeline/image_pair_source.h>
#include "camera.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceOpenCvCam {


class Source : public QAbstractListModel, public ImagePairSource
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::ImagePairSource)

public:
    Source (QObject *parent = Q_NULLPTR);
    virtual ~Source ();

    virtual QString getShortName () const override;
    virtual void getImages (cv::Mat &left, cv::Mat &right) const override;
    virtual void stopSource () override;
    virtual QWidget *createConfigWidget (QWidget *parent = Q_NULLPTR) override;

    bool getSingleCameraMode () const;
    void setSingleCameraMode (bool enabled);

    int getNumberOfCameras () const;
    const ocv_camera_id_t& getCameraInfo (int c) const;
    void setLeftCamera (int c);
    void setRightCamera (int c);

    Camera *getLeftCamera ();
    Camera *getRightCamera ();

    // Model
    virtual int rowCount (const QModelIndex &index) const override;
    virtual Qt::ItemFlags flags (const QModelIndex &index) const override;
    virtual QVariant data (const QModelIndex &index, int role) const override;

    void refreshCameraList ();
    void startStopCapture (bool start);

protected:
    void createCamera (Camera *&camera, int c);
    void releaseCamera (Camera *&camera);
    void setActive (int c, bool value);
    void setActive (const ocv_camera_id_t &id, bool value);

    void synchronizeFrames ();

signals:
    void singleCameraModeChanged (bool enabled);

    void leftCameraChanged ();
    void rightCameraChanged ();

    // Signals from interface
    void imagesChanged () override;
    void error (QString message) override;

protected:
    QVector<ocv_camera_id_t> entries;
    QVector<bool> active;

    bool singleCameraMode;
    Camera *leftCamera;
    Camera *rightCamera;

    bool leftFrameReady, rightFrameReady;

    // Images
    mutable QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;

    cv::Mat imageCombined;
};


} // SourceOpenCvCam
} // Pipeline
} // StereoToolbox
} // MVL


#endif
