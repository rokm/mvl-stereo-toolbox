/*
 * MVL Stereo Toolbox: calibration wizard: calibration page
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_CALIBRATION_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_CALIBRATION_H


#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


class CameraParametersWidget;
class CalibrationFlagsWidget;
class StereoCalibrationFlagsWidget;


// *********************************************************************
// *                      Calibration page: common                     *
// *********************************************************************
class PageCalibration : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(cv::Mat cameraMatrix READ getCameraMatrix);
    Q_PROPERTY(cv::Mat distCoeffs READ getDistCoeffs);
    Q_PROPERTY(int calibrationFlags READ getCalibrationFlags WRITE setCalibrationFlags);

public:
    PageCalibration (const QString &fieldPrefixString, QWidget *parent = Q_NULLPTR);
    virtual ~PageCalibration ();

    virtual void initializePage () override;
    virtual bool isComplete () const override;
    virtual void setVisible (bool visible) override;

    cv::Mat getCameraMatrix () const;
    cv::Mat getDistCoeffs () const;

    void setCalibrationFlags (int flags);
    int getCalibrationFlags (void) const;

protected:
    void calibrationBegin ();
    void calibrationFinished ();

    bool calibrationFunction ();

signals:
    void error (QString message);

protected:
    QString fieldPrefix;

    CameraParametersWidget *widgetCameraParameters;
    CalibrationFlagsWidget *widgetCalibrationFlags;

    QMetaObject::Connection customButtonConnection;

    // Worker
    bool calibrationComplete;
    QFutureWatcher<bool> calibrationWatcher;
    QProgressDialog *dialogBusy;

    // Error
    double calibrationRMSE;

    // Data
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
};


// *********************************************************************
// *                    Calibration page: left camera                  *
// *********************************************************************
class PageLeftCameraCalibration : public PageCalibration
{
    Q_OBJECT

public:
    PageLeftCameraCalibration (QWidget *parent = Q_NULLPTR);
    virtual ~PageLeftCameraCalibration ();

    virtual int nextId () const override;
};


// *********************************************************************
// *                   Calibration page: right camera                  *
// *********************************************************************
class PageRightCameraCalibration : public PageCalibration
{
    Q_OBJECT

public:
    PageRightCameraCalibration (QWidget *parent = Q_NULLPTR);
    virtual ~PageRightCameraCalibration ();

    virtual int nextId () const override;

    virtual void initializePage () override;
};


// *********************************************************************
// *                      Calibration page: stereo                     *
// *********************************************************************
class PageStereoCalibration : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(cv::Mat cameraMatrix1 READ getCameraMatrix1);
    Q_PROPERTY(cv::Mat distCoeffs1 READ getDistCoeffs1);
    Q_PROPERTY(cv::Mat cameraMatrix2 READ getCameraMatrix2);
    Q_PROPERTY(cv::Mat distCoeffs2 READ getDistCoeffs2);
    Q_PROPERTY(cv::Mat R READ getR);
    Q_PROPERTY(cv::Mat T READ getT);

public:
    PageStereoCalibration (QWidget *parent = Q_NULLPTR);
    virtual ~PageStereoCalibration ();

    virtual void initializePage () override;
    virtual bool isComplete () const override;
    virtual void setVisible (bool visible) override;

    const cv::Mat &getCameraMatrix1 () const;
    const cv::Mat &getDistCoeffs1 () const;
    const cv::Mat &getCameraMatrix2 () const;
    const cv::Mat &getDistCoeffs2 () const;
    const cv::Mat &getR () const;
    const cv::Mat &getT () const;
    const cv::Size &getImageSize () const;

protected:
    void calibrationBegin ();
    void calibrationFinished ();

    bool calibrationFunction ();

signals:
    void error (QString message);

protected:
    QString fieldPrefix;

    CameraParametersWidget *widgetLeftCameraParameters;
    CameraParametersWidget *widgetRightCameraParameters;
    StereoCalibrationFlagsWidget *widgetCalibrationFlags;

    QMetaObject::Connection customButtonConnection;

    // Worker
    bool calibrationComplete;
    QFutureWatcher<bool> calibrationWatcher;
    QProgressDialog *dialogBusy;

    // Error
    double calibrationRMSE;
    double averageReprojectionError;

    // Data
    cv::Mat cameraMatrix1;
    cv::Mat distCoeffs1;
    cv::Mat cameraMatrix2;
    cv::Mat distCoeffs2;
    cv::Mat R, T;
    cv::Size imageSize;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
