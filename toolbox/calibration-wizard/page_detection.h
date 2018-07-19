/*
 * MVL Stereo Toolbox: calibration wizard: pattern detection page
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_DETECTION_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_DETECTION_H


#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class CalibrationPattern;
} // Pipeline

namespace Widgets {
    class CalibrationPatternDisplayWidget;
} // Widgets

namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                   Pattern detection page: common                  *
// *********************************************************************
class PageDetection : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(std::vector<std::vector<cv::Point2f> > patternImagePoints READ getPatternImagePoints);
    Q_PROPERTY(std::vector<std::vector<cv::Point3f> > patternWorldPoints READ getPatternWorldPoints);
    Q_PROPERTY(cv::Size imageSize READ getImageSize WRITE setImageSize);

public:
    PageDetection (const QString &fieldPrefixString, QWidget *parent = Q_NULLPTR);
    virtual ~PageDetection ();

    virtual void initializePage () override;

    virtual bool isComplete () const override;
    virtual bool validatePage () override;

    const std::vector<std::vector<cv::Point2f> > &getPatternImagePoints () const;
    const std::vector<std::vector<cv::Point3f> > &getPatternWorldPoints () const;

    void setImageSize (const cv::Size &size);
    const cv::Size &getImageSize () const;

protected:
    void startProcessing ();

    virtual void acceptPattern () = 0;
    virtual void discardPattern () = 0;
    virtual void processImage () = 0;

    void doAutomaticProcessing ();

protected:
    QString fieldPrefix;

    QLabel *labelCaption;
    QLabel *labelStatus;

    QPushButton *pushButtonAuto;
    QPushButton *pushButtonAccept;
    QPushButton *pushButtonDiscard;

    //
    QStringList images;
    int imageCounter;
    bool patternFound;
    bool autoProcess;

    QTimer *autoProcessTimer;

    Pipeline::CalibrationPattern *calibrationPattern;

    // These are what we will pass on...
    cv::Size imageSize;
    std::vector<std::vector<cv::Point2f> > patternImagePoints;
    std::vector<std::vector<cv::Point3f> > patternWorldPoints;
};


// *********************************************************************
// *               Pattern detection page: single camera               *
// *********************************************************************
//
class PageSingleCameraDetection : public PageDetection
{
    Q_OBJECT

public:
    PageSingleCameraDetection (QWidget *parent = Q_NULLPTR);
    PageSingleCameraDetection (const QString &fieldPrefixString = "SingleCamera", QWidget *parent = Q_NULLPTR);

    virtual ~PageSingleCameraDetection ();

    virtual int nextId () const override;

    virtual void initializePage () override;

protected:
    virtual void acceptPattern () override;
    virtual void discardPattern () override;
    virtual void processImage () override;

protected:
    Widgets::CalibrationPatternDisplayWidget *widgetImage;

    std::vector<cv::Point2f> currentImagePoints;
};


// *********************************************************************
// *                Pattern detection page: left camera                *
// *********************************************************************
//
class PageLeftCameraDetection : public PageSingleCameraDetection
{
    Q_OBJECT

public:
    PageLeftCameraDetection (QWidget *parent = Q_NULLPTR);
    virtual ~PageLeftCameraDetection ();

    virtual int nextId () const override;
};


// *********************************************************************
// *                Pattern detection page: right camera               *
// *********************************************************************
class PageRightCameraDetection : public PageSingleCameraDetection
{
    Q_OBJECT

public:
    PageRightCameraDetection (QWidget *parent = Q_NULLPTR);
    virtual ~PageRightCameraDetection ();

    virtual int nextId () const override;
};


// *********************************************************************
// *                   Pattern detection page: stereo                  *
// *********************************************************************
class PageStereoDetection : public PageDetection
{
    Q_OBJECT

public:
    PageStereoDetection (QWidget *parent = Q_NULLPTR);
    virtual ~PageStereoDetection ();

    virtual int nextId () const override;

    virtual void initializePage () override;

protected:
    virtual void acceptPattern () override;
    virtual void discardPattern () override;
    virtual void processImage () override;

protected:
    Widgets::CalibrationPatternDisplayWidget *widgetImageLeft;
    Widgets::CalibrationPatternDisplayWidget *widgetImageRight;

    std::vector<cv::Point2f> currentImagePointsLeft;
    std::vector<cv::Point2f> currentImagePointsRight;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
