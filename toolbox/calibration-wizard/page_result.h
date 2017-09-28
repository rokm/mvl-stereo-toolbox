/*
 * MVL Stereo Toolbox: calibration wizard: result page
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_RESULT_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_RESULT_H


#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {

namespace Widgets {
class ImageDisplayWidget;
class ImagePairDisplayWidget;
} // Widgets

namespace GUI {
namespace CalibrationWizard {

class CameraParametersWidget;


// *********************************************************************
// *                        Result page: common                        *
// *********************************************************************
class PageResult : public QWizardPage
{
    Q_OBJECT

public:
    PageResult (const QString &fieldPrefixString, QWidget *parent = Q_NULLPTR);
    virtual ~PageResult ();

    virtual void initializePage () override;
    virtual void setVisible (bool visible) override;

protected:
    void exportCalibration ();

    void displayTestImage (const QString &filename);

protected:
    QString fieldPrefix;

    CameraParametersWidget *widgetCameraParameters;
    Widgets::ImageDisplayWidget *widgetImage;

    QString customTestImage;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

private:
    QMetaObject::Connection customButtonConnection;
};


// *********************************************************************
// *                    Result page: single camera                     *
// *********************************************************************
class PageSingleCameraResult : public PageResult
{
    Q_OBJECT

public:
    PageSingleCameraResult (QWidget *parent = Q_NULLPTR);
    virtual ~PageSingleCameraResult ();
};



// *********************************************************************
// *                     Result page: left camera                      *
// *********************************************************************
class PageLeftCameraResult : public PageResult
{
    Q_OBJECT

public:
    PageLeftCameraResult (QWidget *parent = Q_NULLPTR);
    virtual ~PageLeftCameraResult ();

    virtual int nextId () const override;
};


// *********************************************************************
// *                     Result page: right camera                     *
// *********************************************************************
class PageRightCameraResult : public PageResult
{
    Q_OBJECT

public:
    PageRightCameraResult (QWidget *parent = Q_NULLPTR);
    virtual ~PageRightCameraResult ();

    virtual int nextId () const override;
};


// *********************************************************************
// *                       Result page: stereo                         *
// *********************************************************************
class PageStereoResult : public QWizardPage
{
    Q_OBJECT

public:
    PageStereoResult (QWidget *parent = Q_NULLPTR);
    virtual ~PageStereoResult ();

    virtual void initializePage () override;

    virtual void setVisible (bool visible) override;

protected:
    void exportCalibration ();

    void displayTestImagePair (const QString &filenameLeft, const QString &filenameRight);

protected:
    QString fieldPrefix;

    CameraParametersWidget *widgetLeftCameraParameters;
    CameraParametersWidget *widgetRightCameraParameters;

    Widgets::ImagePairDisplayWidget *widgetImage;

    QString customTestImageLeft;
    QString customTestImageRight;

    cv::Rect validRoi1, validRoi2;
    cv::Mat map11, map12, map21, map22;

private:
    QMetaObject::Connection customButtonConnection;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
