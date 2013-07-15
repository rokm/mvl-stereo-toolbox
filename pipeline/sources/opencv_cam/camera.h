/*
 * OpenCV Camera: camera
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

#ifndef CAMERA_OPENCV_H
#define CAMERA_OPENCV_H

#include <QtCore>
#include <QtGui>
#include <dc1394/dc1394.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


namespace SourceOpenCvCam {

struct ocv_camera_id_t {
    int interface_idx; // Interface index (in our list!)
    int camera_idx; // Camera index... 0-99
};

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera (cv::VideoCapture *, ocv_camera_id_t, QObject * = 0);
    virtual ~Camera ();

    // Config widget
    QWidget *createConfigWidget (QWidget * = 0);

    // Camera identification
    const ocv_camera_id_t &getId () const;
    bool isSameCamera (const ocv_camera_id_t &) const;

    // Camera start/stop
    void startCapture ();
    void stopCapture ();

    bool getCaptureState () const;

    float getEffectiveCaptureFramerate () const;

    // Frame
    void copyFrame (cv::Mat &);

    // Properties
    double getProperty (int);
    
public slots:
    void setProperty (int, double);
    
protected:
    void captureFunction ();

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (const QString);

    void propertyChanged ();

    void workerStopCapture ();

protected:
    cv::VideoCapture *capture;
    ocv_camera_id_t id;

    // Capture thread
    bool captureActive;
    QFutureWatcher<void> captureWatcher;

    QReadWriteLock frameBufferLock;
    cv::Mat frameBuffer;
};

}

#endif
