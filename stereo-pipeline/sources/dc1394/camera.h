/*
 * DC1394 Source: camera
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__CAMERA_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__CAMERA_H

#include <QtWidgets>
#include <dc1394/dc1394.h>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceDC1394 {


class CameraCaptureWorker;

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera (dc1394camera_t *c, QObject *parent = nullptr);
    virtual ~Camera ();

    // Config widget
    QWidget *createConfigWidget (QWidget *parent = nullptr);

    // Camera identification
    dc1394camera_id_t getId () const;
    bool isSameCamera (const dc1394camera_id_t &id) const;

    QString getVendor () const;
    QString getModel () const;

    // Basic parameters
    void setIsoSpeed (dc1394speed_t speed);
    dc1394speed_t getIsoSpeed () const;

    QVector<dc1394video_mode_t> getSupportedModes ();
    void setMode (dc1394video_mode_t mode);
    dc1394video_mode_t getMode () const;

    QVector<dc1394framerate_t> getSupportedFramerates ();
    void setFramerate (dc1394framerate_t fps);
    dc1394framerate_t getFramerate () const;

    // Camera features
    const dc1394featureset_t &getFeatureSet () const;

    void setFeatureValue (dc1394feature_t feature, int newValue);
    int getFeatureValue (dc1394feature_t);

    void setFeatureAbsoluteValue (dc1394feature_t feature, double newValue);
    double getFeatureAbsoluteValue (dc1394feature_t);

    QList<dc1394feature_mode_t> getFeatureModes (dc1394feature_t feature);
    void setFeatureMode (dc1394feature_t feature, dc1394feature_mode_t mode);
    dc1394feature_mode_t getFeatureMode (dc1394feature_t feature);

    // Camera start/stop
    void startCapture ();
    void stopCapture ();

    bool getCaptureState () const;

    // Frame
    void copyFrame (cv::Mat &frame);

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (QString message);

    void modeChanged ();
    void parameterChanged ();

    void workerStopCapture ();

protected:
    dc1394camera_id_t id;

    dc1394camera_t *camera;
    dc1394featureset_t features;

    QSocketNotifier *frameNotifier;

    // Capture thread
    QThread *captureThread;
    CameraCaptureWorker *captureWorker;
};


class CameraCaptureWorker : public QObject
{
    Q_OBJECT

public:
    CameraCaptureWorker (dc1394camera_t *c, QObject *parent = nullptr);
    virtual ~CameraCaptureWorker ();

    void copyFrame (cv::Mat &frame);

    void startCapture ();
    void stopCapture ();

protected:
    void grabFrame ();

    void dequeueCaptureBuffer (dc1394video_frame_t *&frame, bool drainQueue);
    void enqueueCaptureBuffer (dc1394video_frame_t *frame);
    void convertToOpenCVImage (dc1394video_frame_t *frame, cv::Mat &image) const;

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (QString);

protected:
    dc1394camera_t *camera;

    QSocketNotifier *frameNotifier;

    cv::Mat frameBuffer;
    QReadWriteLock frameBufferLock;
};


} // SourceDC1394
} // Pipeline
} // StereoToolbox
} // MVL


#endif
