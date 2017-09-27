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

protected:
    QString fieldPrefix;

    CameraParametersWidget *widgetCameraParameters;
    Widgets::ImageDisplayWidget *widgetImage;

    QMetaObject::Connection customButtonConnection;
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

protected:
    QString fieldPrefix;

    QMetaObject::Connection customButtonConnection;

    CameraParametersWidget *widgetLeftCameraParameters;
    CameraParametersWidget *widgetRightCameraParameters;

    Widgets::ImagePairDisplayWidget *widgetImage;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
