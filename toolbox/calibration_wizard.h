/*
 * MVL Stereo Toolbox: calibration wizard
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

#ifndef CALIBRATION_WIZARD_H
#define CALIBRATION_WIZARD_H

#include <QtWidgets>
#include <QtConcurrent>

#include <stereo-pipeline/calibration_pattern.h>

#include <opencv2/core.hpp>


// Metatype declarations for OpenCV types, so we can pass them between
// wizard pages
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point2f> >);
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point3f> >);
Q_DECLARE_METATYPE(cv::Size);
Q_DECLARE_METATYPE(cv::Mat);


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class CalibrationPattern;
} // Pipeline

namespace Widgets {
    class CalibrationPatternDisplayWidget;
    class ImageDisplayWidget;
    class ImagePairDisplayWidget;
} // Widgets

namespace GUI {


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

    Q_PROPERTY(QStringList images READ getImages WRITE setImages NOTIFY imagesChanged);
    Q_PROPERTY(int patternType READ getPatternType WRITE setPatternType);

public:
    CalibrationWizardPageImages (const QString &, QWidget * = 0);
    virtual ~CalibrationWizardPageImages ();

protected:
    void addImages ();
    void clearImages ();

    virtual QStringList getImages () const;
    void setImages (const QStringList &);

    void appendImages (const QStringList &);

    int getPatternType () const;
    void setPatternType (int);

signals:
    void imagesChanged ();

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
    Q_OBJECT
    Q_PROPERTY(int imagesOrder READ getImagesOrder WRITE setImagesOrder);

public:
    CalibrationWizardPageStereoImages (QWidget * = 0);
    virtual ~CalibrationWizardPageStereoImages ();

    virtual int nextId () const;
    virtual bool isComplete () const;

    virtual void initializePage ();

    virtual QStringList getImages () const;

    enum {
        Interleaved,
        Grouped
    } ImagesOrder;

    int getImagesOrder () const;
    void setImagesOrder (int order);

protected:
    void updateImageEntries ();

protected:
    QRadioButton *radioButtonInterleaved;
    QRadioButton *radioButtonGrouped;
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

    virtual void initializePage ();

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
    virtual bool validatePage ();

    const std::vector<std::vector<cv::Point2f> > &getPatternImagePoints () const;
    const std::vector<std::vector<cv::Point3f> > &getPatternWorldPoints () const;
    const cv::Size &getImageSize () const;

protected:
    void startProcessing ();
    void acceptPattern ();
    void discardPattern ();
    void processImage ();

    void doAutomaticProcessing ();

protected:
    QString fieldPrefix;

    Widgets::CalibrationPatternDisplayWidget *displayImage;

    QLabel *labelStatus;

    QPushButton *pushButtonAuto;
    QPushButton *pushButtonAccept;
    QPushButton *pushButtonDiscard;

    // Processing part
    bool processImagePairs;

    //
    QStringList images;
    int imageCounter;
    bool patternFound;
    bool autoProcess;

    Pipeline::CalibrationPattern calibrationPattern;
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
// Camera parameters widget
class CameraParametersWidget : public QGroupBox
{
    Q_OBJECT

public:
    CameraParametersWidget (const QString & = "Parameters", QWidget * = 0);
    virtual ~CameraParametersWidget ();

    double getFocalLengthX () const;
    void setFocalLengthX (double);

    double getFocalLengthY () const;
    void setFocalLengthY (double);

    double getPrincipalPointX () const;
    void setPrincipalPointX (double);

    double getPrincipalPointY () const;
    void setPrincipalPointY (double);

    double getDistortionK1 () const;
    void setDistortionK1 (double);

    double getDistortionK2 () const;
    void setDistortionK2 (double);

    double getDistortionK3 () const;
    void setDistortionK3 (double);

    double getDistortionP1 () const;
    void setDistortionP1 (double);

    double getDistortionP2 () const;
    void setDistortionP2 (double);

    double getDistortionK4 () const;
    void setDistortionK4 (double);

    double getDistortionK5 () const;
    void setDistortionK5 (double);

    double getDistortionK6 () const;
    void setDistortionK6 (double);

    void setCameraMatrix (const cv::Mat &, const std::vector<double> &);
    cv::Mat getCameraMatrix () const;
    std::vector<double> getDistCoeffs () const;

    void setDisplayMode (bool);

protected:
    QDoubleSpinBox *spinBoxFx;
    QDoubleSpinBox *spinBoxFy;
    QDoubleSpinBox *spinBoxCx;
    QDoubleSpinBox *spinBoxCy;
    QDoubleSpinBox *spinBoxK1;
    QDoubleSpinBox *spinBoxK2;
    QDoubleSpinBox *spinBoxP1;
    QDoubleSpinBox *spinBoxP2;
    QDoubleSpinBox *spinBoxK3;
    QDoubleSpinBox *spinBoxK4;
    QDoubleSpinBox *spinBoxK5;
    QDoubleSpinBox *spinBoxK6;
};

// Calibration flags widget
class CalibrationFlagsWidget : public QGroupBox
{
    Q_OBJECT

public:
    CalibrationFlagsWidget (const QString & = "Flags", QWidget * = 0);
    ~CalibrationFlagsWidget ();

    virtual int getFlags () const;
    virtual void setFlags (int);

protected:
    QCheckBox *checkBoxUseIntrinsicGuess;
    QCheckBox *checkBoxFixPrincipalPoint;
    QCheckBox *checkBoxFixAspectRatio;
    QCheckBox *checkBoxZeroTangentDist;
    QCheckBox *checkBoxRationalModel;
    QCheckBox *checkBoxFixK1;
    QCheckBox *checkBoxFixK2;
    QCheckBox *checkBoxFixK3;
    QCheckBox *checkBoxFixK4;
    QCheckBox *checkBoxFixK5;
    QCheckBox *checkBoxFixK6;
};

// Stereo calibration flags widget
class StereoCalibrationFlagsWidget : public CalibrationFlagsWidget
{
    Q_OBJECT

public:
    StereoCalibrationFlagsWidget (const QString & = "Flags", QWidget * = 0);
    ~StereoCalibrationFlagsWidget ();

    virtual int getFlags () const;
    virtual void setFlags (int);

protected:
    QCheckBox *checkBoxFixIntrinsic;
    QCheckBox *checkBoxFixFocalLength;
    QCheckBox *checkBoxSameFocalLength;
};



// Common class for left/right camera
class CalibrationWizardPageCalibration : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(cv::Mat cameraMatrix READ getCameraMatrix);
    Q_PROPERTY(cv::Mat distCoeffs READ getDistCoeffs);
    Q_PROPERTY(int calibrationFlags READ getCalibrationFlags WRITE setCalibrationFlags);

public:
    CalibrationWizardPageCalibration (const QString &, QWidget * = 0);
    virtual ~CalibrationWizardPageCalibration ();

    virtual void initializePage ();
    virtual bool isComplete () const;
    virtual void setVisible (bool);

    cv::Mat getCameraMatrix () const;
    cv::Mat getDistCoeffs () const;

    void setCalibrationFlags (int);
    int getCalibrationFlags (void) const;

protected:
    void calibrationBegin ();
    void calibrationFinished ();

    bool calibrationFunction ();

signals:
    void error (QString);

protected:
    QString fieldPrefix;

    CameraParametersWidget *boxCameraParameters;
    CalibrationFlagsWidget *boxCalibrationFlags;

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

    virtual void initializePage ();
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
    virtual bool isComplete () const;
    virtual void setVisible (bool);

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
    void error (QString);

protected:
    QString fieldPrefix;

    CameraParametersWidget *boxLeftCameraParameters;
    CameraParametersWidget *boxRightCameraParameters;
    StereoCalibrationFlagsWidget *boxCalibrationFlags;

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

    CameraParametersWidget *boxCameraParameters;
    Widgets::ImageDisplayWidget *displayImage;
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

    virtual void setVisible (bool);

protected:
    void exportCalibration ();

protected:
    QString fieldPrefix;

    QMetaObject::Connection customButtonConnection;

    CameraParametersWidget *boxLeftCameraParameters;
    CameraParametersWidget *boxRightCameraParameters;

    Widgets::ImagePairDisplayWidget *displayImage;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
