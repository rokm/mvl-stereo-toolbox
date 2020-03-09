/*
 * OpenCV Camera Source: camera
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__CAMERA_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__CAMERA_H

#include <QtWidgets>
#include <QtConcurrent>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceOpenCvCam {


struct ocv_camera_id_t {
    int interface_idx; // Interface index (in our list!)
    int camera_idx; // Camera index... 0-99
};

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera (cv::VideoCapture *capture, ocv_camera_id_t id, QObject *parent = nullptr);
    virtual ~Camera ();

    // Config widget
    QWidget *createConfigWidget (QWidget *parent = nullptr);

    // Camera identification
    const ocv_camera_id_t &getId () const;
    bool isSameCamera (const ocv_camera_id_t &otherId) const;

    // Camera start/stop
    void startCapture ();
    void stopCapture ();

    bool getCaptureState () const;

    // Frame
    void copyFrame (cv::Mat &frame);

    // Properties
    void setProperty (int prop, double value);
    double getProperty (int prop);

protected:
    void captureFunction ();

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (QString message);

    void propertyChanged ();

protected:
    cv::VideoCapture *capture;
    ocv_camera_id_t id;

    // Capture thread
    bool captureActive;
    QFutureWatcher<void> captureWatcher;

    QReadWriteLock frameBufferLock;
    cv::Mat frameBuffer;
};


} // SourceOpenCvCam
} // Pipeline
} // StereoToolbox
} // MVL


#endif
