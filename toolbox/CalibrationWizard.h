/*
 * MVL Stereo Toolbox: calibration wizard
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef CALIBRATION_WIZARD_H
#define CALIBRATION_WIZARD_H

#include <QtGui>

#include "StereoCalibrationPattern.h"

#include <opencv2/highgui/highgui.hpp>


class CalibrationPattern;
class CalibrationPatternDisplayWidget;
class ImageDisplayWidget;
class ImagePairDisplayWidget;

// Metatype declarations for OpenCV types, so we can pass them between
// wizard pages
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point2f> >);
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point3f> >);
Q_DECLARE_METATYPE(cv::Size);
Q_DECLARE_METATYPE(cv::Mat);


// *********************************************************************
// *                               Wizard                              *
// *********************************************************************
class CalibrationWizard : public QWizard
{
    Q_OBJECT
    
public:
    CalibrationWizard (QWidget * = 0);
    virtual ~CalibrationWizard ();

    enum {
        PageIntroduction,
        PageLeftCameraImages,
        PageLeftCameraDetection,
        PageLeftCameraCalibration,
        PageLeftCameraResult,
        PageRightCameraImages,
        PageRightCameraDetection,
        PageRightCameraCalibration,
        PageRightCameraResult,
        PageStereoImages,
        PageStereoDetection,
        PageStereoCalibration,
        PageStereoResult,
    };    
};


// *********************************************************************
// *                         Page: introduction                        *
// *********************************************************************
class CalibrationWizardPageIntroduction : public QWizardPage
{
    Q_OBJECT

public:
    CalibrationWizardPageIntroduction (QWidget * = 0);
    virtual ~CalibrationWizardPageIntroduction ();

    virtual int nextId () const;

protected:
    QRadioButton *radioButtonJointCalibration;
    QRadioButton *radioButtonDecoupledCalibration;
};


// *********************************************************************
// *                        Page: image selection                      *
// *********************************************************************
// Common class
class CalibrationWizardPageImages : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QStringList images READ getImages);
    Q_PROPERTY(int patternType READ getPatternType);

public:
    CalibrationWizardPageImages (const QString &, QWidget * = 0);
    virtual ~CalibrationWizardPageImages ();

protected slots:
    void addImages ();
    void clearImages ();

protected:
    QStringList getImages () const;
    int getPatternType () const;

protected:
    QString fieldPrefix;

    // Image sequence
    QPushButton *pushButtonAddImages;
    QPushButton *pushButtonClearImages;
    QListWidget *listWidgetImages;

    // Pattern
    QSpinBox *spinBoxPatternWidth;
    QSpinBox *spinBoxPatternHeight;
    QDoubleSpinBox *spinBoxElementSize;
    QComboBox *comboBoxPatternType;
    QSpinBox *spinBoxScaleLevels;
    QDoubleSpinBox *spinBoxScaleIncrement;
};

// Stereo
class CalibrationWizardPageStereoImages : public CalibrationWizardPageImages
{
public:
    CalibrationWizardPageStereoImages (QWidget * = 0);
    virtual ~CalibrationWizardPageStereoImages ();

    virtual int nextId () const;
    virtual bool isComplete () const;
};

// Left camera
class CalibrationWizardPageLeftCameraImages : public CalibrationWizardPageImages
{
public:
    CalibrationWizardPageLeftCameraImages (QWidget * = 0);
    virtual ~CalibrationWizardPageLeftCameraImages ();

    virtual int nextId () const;
    virtual bool isComplete () const;
};

// Right camera
class CalibrationWizardPageRightCameraImages : public CalibrationWizardPageImages
{
public:
    CalibrationWizardPageRightCameraImages (QWidget * = 0);
    virtual ~CalibrationWizardPageRightCameraImages ();

    virtual int nextId () const;
    virtual bool isComplete () const;
};


// *********************************************************************
// *                       Page: pattern detection                     *
// *********************************************************************
// Common class
class CalibrationWizardPageDetection : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(std::vector<std::vector<cv::Point2f> > patternImagePoints READ getPatternImagePoints);
    Q_PROPERTY(std::vector<std::vector<cv::Point3f> > patternWorldPoints READ getPatternWorldPoints);
    Q_PROPERTY(cv::Size imageSize READ getImageSize);
    
public:
    CalibrationWizardPageDetection (const QString &, bool, QWidget * = 0);
    virtual ~CalibrationWizardPageDetection ();

    virtual void initializePage ();

    virtual bool isComplete () const;

    const std::vector<std::vector<cv::Point2f> > &getPatternImagePoints () const;
    const std::vector<std::vector<cv::Point3f> > &getPatternWorldPoints () const;
    const cv::Size &getImageSize () const;

protected slots:
    void acceptPattern ();
    void discardPattern ();
    void processNextImage ();

protected:
    QString fieldPrefix;

    CalibrationPatternDisplayWidget *displayImage;

    QLabel *labelStatus;

    QDialogButtonBox *buttonBox;
    QPushButton *pushButtonAccept;
    QPushButton *pushButtonDiscard;

    // Processing part
    bool processImagePairs;
    
    QStringList images;
    int imageCounter;

    StereoCalibrationPattern calibrationPattern;
    std::vector<cv::Point2f> currentImagePoints;

    // These are what we will pass on...
    cv::Size imageSize;
    std::vector<std::vector<cv::Point2f> > patternImagePoints;
    std::vector<std::vector<cv::Point3f> > patternWorldPoints;
};

// Stereo
class CalibrationWizardPageStereoDetection : public CalibrationWizardPageDetection
{
public:
    CalibrationWizardPageStereoDetection (QWidget * = 0);
    virtual ~CalibrationWizardPageStereoDetection ();

    virtual int nextId () const;
};

// Left camera
class CalibrationWizardPageLeftCameraDetection : public CalibrationWizardPageDetection
{
public:
    CalibrationWizardPageLeftCameraDetection (QWidget * = 0);
    virtual ~CalibrationWizardPageLeftCameraDetection ();

    virtual int nextId () const;
};

// Right camera
class CalibrationWizardPageRightCameraDetection : public CalibrationWizardPageDetection
{
public:
    CalibrationWizardPageRightCameraDetection (QWidget * = 0);
    virtual ~CalibrationWizardPageRightCameraDetection ();

    virtual int nextId () const;
};

// *********************************************************************
// *                         Page: calibration                         *
// *********************************************************************
// Common class for left/right camera
class CalibrationWizardPageCalibration : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(cv::Mat cameraMatrix READ getCameraMatrix);
    Q_PROPERTY(cv::Mat distCoeffs READ getDistCoeffs);

public:
    CalibrationWizardPageCalibration (const QString &, QWidget * = 0);
    virtual ~CalibrationWizardPageCalibration ();

    virtual void initializePage ();
    virtual void cleanupPage ();

    virtual bool validatePage ();

    cv::Mat getCameraMatrix () const;
    cv::Mat getDistCoeffs () const;

protected:
    QString fieldPrefix;

    QString oldNextButtonText;

    // Data
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
};

// Left camera
class CalibrationWizardPageLeftCameraCalibration : public CalibrationWizardPageCalibration
{
public:
    CalibrationWizardPageLeftCameraCalibration (QWidget * = 0);
    virtual ~CalibrationWizardPageLeftCameraCalibration ();

    virtual int nextId () const;
};

// Right camera
class CalibrationWizardPageRightCameraCalibration : public CalibrationWizardPageCalibration
{
public:
    CalibrationWizardPageRightCameraCalibration (QWidget * = 0);
    virtual ~CalibrationWizardPageRightCameraCalibration ();

    virtual int nextId () const;
};

// Stereo
class CalibrationWizardPageStereoCalibration : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(cv::Mat cameraMatrix1 READ getCameraMatrix1);
    Q_PROPERTY(cv::Mat distCoeffs1 READ getDistCoeffs1);
    Q_PROPERTY(cv::Mat cameraMatrix2 READ getCameraMatrix2);
    Q_PROPERTY(cv::Mat distCoeffs2 READ getDistCoeffs2);
    Q_PROPERTY(cv::Mat R READ getR);
    Q_PROPERTY(cv::Mat T READ getT);

public:
    CalibrationWizardPageStereoCalibration (QWidget * = 0);
    virtual ~CalibrationWizardPageStereoCalibration ();

    virtual void initializePage ();
    virtual void cleanupPage ();

    virtual bool validatePage ();

    const cv::Mat &getCameraMatrix1 () const;
    const cv::Mat &getDistCoeffs1 () const;
    const cv::Mat &getCameraMatrix2 () const;
    const cv::Mat &getDistCoeffs2 () const;
    const cv::Mat &getR () const;
    const cv::Mat &getT () const;
    const cv::Size &getImageSize () const;

protected:
    QString fieldPrefix;

    QString oldNextButtonText;

    // Data
    cv::Mat cameraMatrix1;
    cv::Mat distCoeffs1;
    cv::Mat cameraMatrix2;
    cv::Mat distCoeffs2;
    cv::Mat R, T;
    cv::Size imageSize;
};


// *********************************************************************
// *                            Page: result                           *
// *********************************************************************
// Common class for left/right camera
class CalibrationWizardPageResult : public QWizardPage
{
    Q_OBJECT

public:
    CalibrationWizardPageResult (const QString &, QWidget * = 0);
    virtual ~CalibrationWizardPageResult ();

    virtual void initializePage ();

protected:
    QString fieldPrefix;

    ImageDisplayWidget *displayImage;
};

// Left camera
class CalibrationWizardPageLeftCameraResult : public CalibrationWizardPageResult
{
public:
    CalibrationWizardPageLeftCameraResult (QWidget * = 0);
    virtual ~CalibrationWizardPageLeftCameraResult ();

    virtual int nextId () const;
};

// Right camera
class CalibrationWizardPageRightCameraResult : public CalibrationWizardPageResult
{
public:
    CalibrationWizardPageRightCameraResult (QWidget * = 0);
    virtual ~CalibrationWizardPageRightCameraResult ();

    virtual int nextId () const;
};

// Stereo
class CalibrationWizardPageStereoResult : public QWizardPage
{
    Q_OBJECT

public:
    CalibrationWizardPageStereoResult (QWidget * = 0);
    virtual ~CalibrationWizardPageStereoResult ();

    virtual void initializePage ();

protected:
    QString fieldPrefix;

    ImagePairDisplayWidget *displayImage;
};

#endif
