/*
 * MVL Stereo Toolbox: calibration wizard: result page
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

#include "page_result.h"
#include "wizard.h"
#include "camera_parameters_widget.h"

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/rectification.h>
#include <stereo-widgets/image_display_widget.h>
#include <stereo-widgets/image_pair_display_widget.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                  Clickable image display widgets                  *
// *********************************************************************
// Image display widget
class ClickableImageDisplayWidget : public Widgets::ImageDisplayWidget
{
    Q_OBJECT

public:
    ClickableImageDisplayWidget (const QString &text = QString(), QWidget *parent = Q_NULLPTR);
    virtual ~ClickableImageDisplayWidget ();

protected:
    virtual void mouseDoubleClickEvent (QMouseEvent *event) override;

signals:
    void doubleClicked ();
};


ClickableImageDisplayWidget::ClickableImageDisplayWidget (const QString &text, QWidget *parent)
    : Widgets::ImageDisplayWidget(text, parent)
{
}

ClickableImageDisplayWidget::~ClickableImageDisplayWidget ()
{
}

void ClickableImageDisplayWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked ();
    }
}


// Image pair display widget
class ClickableImagePairDisplayWidget : public Widgets::ImagePairDisplayWidget
{
    Q_OBJECT

public:
    ClickableImagePairDisplayWidget (const QString &text = QString(), QWidget *parent = Q_NULLPTR);
    virtual ~ClickableImagePairDisplayWidget ();

protected:
    virtual void mouseDoubleClickEvent (QMouseEvent *event) override;

signals:
    void doubleClicked ();
};

ClickableImagePairDisplayWidget::ClickableImagePairDisplayWidget (const QString &text, QWidget *parent)
    : Widgets::ImagePairDisplayWidget(text, parent)
{
}

ClickableImagePairDisplayWidget::~ClickableImagePairDisplayWidget ()
{
}

void ClickableImagePairDisplayWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked ();
    }
}


// Because we are keeping the above two classes confined to this source file...
#include "page_result.moc"


// *********************************************************************
// *                        Result page: common                        *
// *********************************************************************
PageResult::PageResult (const QString &fieldPrefix, Pipeline::Pipeline *pipeline, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefix),
      pipeline(pipeline)
{
    setSubTitle("Calibration result");

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    labelCaption = new QLabel(); // Text is set in InitializePage()
    labelCaption->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    labelCaption->setWordWrap(true);

    layout->addWidget(labelCaption);

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Camera parameters
    widgetCameraParameters = new CameraParametersWidget("Parameters", this);
    widgetCameraParameters->setDisplayMode(true);
    splitter->addWidget(widgetCameraParameters);

    // Undistorted image
    ClickableImageDisplayWidget *widgetImage = new ClickableImageDisplayWidget("Undistorted image", this);
    this->widgetImage = widgetImage; // Store pointer to Widgets::ImageDisplayWidget
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetImage->resize(400, 600);
    splitter->addWidget(widgetImage);

    // Image selection
    connect(widgetImage, &ClickableImageDisplayWidget::doubleClicked, this, &PageResult::displayCustomTestImage);
}

PageResult::~PageResult ()
{
}


void PageResult::displayCustomTestImage ()
{
    cv::Mat image;

    if (field(fieldPrefix + "LiveCapture").toBool()) {
        // Retrieve image from stereo pipeline
        image = getImageFromPipeline();
    } else {
        // Select image
        QString filename = QFileDialog::getOpenFileName(this, "Select test image", QString(), "Images (*.jpg *.png *.bmp *.tif *.ppm *.pgm)");
        if (filename.isEmpty()) {
            return;
        }

        // Load
        image = cv::imread(filename.toStdString(), -1);

        // No-op if loading failed
        if (image.empty()) {
            qWarning() << this << ": failed to load test image:" << filename;
            return;
        }
    }

    displayTestImage(image);
}

cv::Mat PageResult::getImageFromPipeline ()
{
    // Return left image
    return pipeline->getLeftImage();
}

void PageResult::displayTestImage (const cv::Mat &image)
{
    cv::Mat undistortedImage;

    // Undistort
    if (!image.empty()) {
        cv::undistort(image, undistortedImage, cameraMatrix, distCoeffs);
    }

    // Display undistorted image
    widgetImage->setImage(undistortedImage);
}


void PageResult::initializePage ()
{
    // Get camera calibration
    cameraMatrix = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
    distCoeffs = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();

    // Display parameters
    widgetCameraParameters->setCameraMatrix(cameraMatrix, distCoeffs);

    // Display last-processed calibration image
    cv::Mat image = field(fieldPrefix + "LastProcessedImage").value<cv::Mat>();
    if (image.empty()) {
        qWarning() << this << ": failed to retrieved last-processed image!";
    }
    displayTestImage(image);

    // Set caption
    if (field(fieldPrefix + "LiveCapture").toBool()) {
        labelCaption->setText("Congratulations! Camera's intrinsic parameters have been calibrated. Last-processed calibration image has been undistorted and is shown below for visual validation of calibration result. To load new image from live source, double-click on the image widget.");
    } else {
        labelCaption->setText("Congratulations! Camera's intrinsic parameters have been calibrated. Last-processed calibration image has been undistorted and is shown below for visual validation of calibration result. To load a custom test image, double-click on the image widget.");
    }
}


void PageResult::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Export"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageResult::exportCalibration);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}

void PageResult::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save camera calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            ext = "yml";
            fileName += "." + ext;
        }

        // Get parameters
        cv::Mat cameraMatrix = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
        cv::Mat distCoeffs = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();
        cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

        // Store
        cv::FileStorage storage(fileName.toStdString(), cv::FileStorage::WRITE);
        if (storage.isOpened()) {
            storage << "DataType" << "CameraCalibration";
            storage << "imageSize" << imageSize;
            storage << "cameraMatrix" << cameraMatrix;
            storage << "distCoeffs" << distCoeffs;
        } else {
            QMessageBox::warning(this, "Error", QString("Failed to export calibration to '%1'").arg(fileName));
        }
    }
}


// *********************************************************************
// *                    Result page: single camera                     *
// *********************************************************************
PageSingleCameraResult::PageSingleCameraResult (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageResult("SingleCamera", pipeline, parent)
{
    setTitle("Single camera calibration");
}

PageSingleCameraResult::~PageSingleCameraResult ()
{
}

int PageSingleCameraResult::nextId () const
{
    return -1;
}


// *********************************************************************
// *                     Result page: left camera                      *
// *********************************************************************
PageLeftCameraResult::PageLeftCameraResult (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageResult("LeftCamera", pipeline, parent)
{
    setTitle("Left camera calibration");
}

PageLeftCameraResult::~PageLeftCameraResult ()
{
}

int PageLeftCameraResult::nextId () const
{
    return Wizard::PageId::RightCameraImagesId;
}


// *********************************************************************
// *                     Result page: right camera                     *
// *********************************************************************
PageRightCameraResult::PageRightCameraResult (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageResult("RightCamera", pipeline, parent)
{
    setTitle("Right camera calibration");
}

PageRightCameraResult::~PageRightCameraResult ()
{
}

int PageRightCameraResult::nextId () const
{
    return Wizard::PageId::StereoImagesId;
}


cv::Mat PageRightCameraResult::getImageFromPipeline ()
{
    // Return right image
    return pipeline->getRightImage();
}


// *********************************************************************
// *                        Result page: stereo                        *
// *********************************************************************
PageStereoResult::PageStereoResult (Pipeline::Pipeline *pipeline, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix("Stereo"),
      pipeline(pipeline)
{
    setTitle("Stereo calibration");
    setSubTitle("Calibration result");

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    labelCaption = new QLabel(); // Text is set in InitializePage()
    labelCaption->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    labelCaption->setWordWrap(true);

    layout->addWidget(labelCaption);

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Undistorted image
    ClickableImagePairDisplayWidget *widgetImage = new ClickableImagePairDisplayWidget("Rectified image pair", this);
    this->widgetImage = widgetImage;
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetImage->resize(400, 600);
    splitter->addWidget(widgetImage);

    connect(widgetImage, &ClickableImagePairDisplayWidget::doubleClicked, this, &PageStereoResult::displayCustomTestImagePair);

    // Rectification parameters
    QGroupBox *groupBoxRectification = new QGroupBox("Rectification parameters", this);
    QHBoxLayout *rectificationLayout = new QHBoxLayout(groupBoxRectification);

    checkBoxZeroDisparity = new QCheckBox("CALIB_ZERO_DISPARITY", this);

    rectificationLayout->addWidget(checkBoxZeroDisparity);

    rectificationLayout->addStretch(1);

    spinBoxAlpha = new QDoubleSpinBox(this);
    spinBoxAlpha->setRange(-1, 1);
    spinBoxAlpha->setDecimals(2);
    spinBoxAlpha->setSingleStep(0.10);

    rectificationLayout->addWidget(new QLabel("Alpha: ", this));
    rectificationLayout->addWidget(spinBoxAlpha);

    splitter->addWidget(groupBoxRectification);

    auto lambdaUpdateRectification = [this] () {
        initializeRectificationMaps();
        displayTestImagePair();
    };
    connect(spinBoxAlpha, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, lambdaUpdateRectification);
    connect(checkBoxZeroDisparity, &QCheckBox::toggled, this, lambdaUpdateRectification);

    // Scroll area; camera parameters
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);

    QWidget *parametersWidget = new QWidget(scrollArea);
    QHBoxLayout *boxLayout = new QHBoxLayout(parametersWidget);
    parametersWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boxLayout->setContentsMargins(0, 0, 0, 0);

    // Left camera parameters
    widgetLeftCameraParameters = new CameraParametersWidget("Left camera", this);
    widgetLeftCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetLeftCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(widgetLeftCameraParameters);

    //boxLayout->addStretch();

    // Right camera parameters
    widgetRightCameraParameters = new CameraParametersWidget("Right camera", this);
    widgetRightCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetRightCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(widgetRightCameraParameters);

    scrollArea->setWidget(parametersWidget);
    splitter->addWidget(scrollArea);

    // Settable fields
    registerField(fieldPrefix + "RectificationAlpha", spinBoxAlpha, "value");
    registerField(fieldPrefix + "RectificationZeroDisparity", checkBoxZeroDisparity);
}

PageStereoResult::~PageStereoResult ()
{
}

int PageStereoResult::nextId () const
{
    return -1;
}


void PageStereoResult::initializeRectificationMaps ()
{
    // Get stereo calibration
    cv::Mat M1 = field(fieldPrefix + "CameraMatrix1").value<cv::Mat>();
    cv::Mat D1 = field(fieldPrefix + "DistCoeffs1").value<cv::Mat>();
    cv::Mat M2 = field(fieldPrefix + "CameraMatrix2").value<cv::Mat>();
    cv::Mat D2 = field(fieldPrefix + "DistCoeffs2").value<cv::Mat>();
    cv::Mat T = field(fieldPrefix + "T").value<cv::Mat>();
    cv::Mat R = field(fieldPrefix + "R").value<cv::Mat>();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    if (M1.empty()) {
        // This catches the call triggered by initial setting of
        // rectification parameters when the wizard is started...
        return;
    }

    // Get rectification settings
    double alpha = field(fieldPrefix + "RectificationAlpha").value<double>();
    bool zeroDisparity = field(fieldPrefix + "RectificationZeroDisparity").value<bool>();

    // Initialize stereo rectification
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;

    cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, zeroDisparity ? cv::CALIB_ZERO_DISPARITY : 0, alpha, imageSize, &validRoi1, &validRoi2);

    cv::initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    cv::initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);
}

void PageStereoResult::displayCustomTestImagePair ()
{
    cv::Mat imageLeft, imageRight;

    if (field(fieldPrefix + "LiveCapture").toBool()) {
        // Retieve image from stereo pipeline
        pipeline->getImages(imageLeft, imageRight);
    } else {
        // Select image pair
        QStringList filenames = QFileDialog::getOpenFileNames(this, "Select test image pair", QString(), "Images (*.jpg *.png *.bmp *.tif *.ppm *.pgm)");
        if (filenames.isEmpty()) {
            return;
        }

        if (filenames.size() != 2) {
            QMessageBox::warning(this, "Error", QString("Exactly two images need to be selected!"));
            return;
        }

        // Load image pair
        imageLeft = cv::imread(filenames[0].toStdString(), -1);
        imageRight = cv::imread(filenames[1].toStdString(), -1);

        if (imageLeft.empty()) {
            qWarning() << this << ": failed to load left test image:" << filenames[0];
            return;
        }

        if (imageRight.empty()) {
            qWarning() << this << ": failed to load right test image:" << filenames[1];
            return;
        }
    }

    displayTestImagePair(imageLeft, imageRight);
}

void PageStereoResult::displayTestImagePair (const cv::Mat &imageLeft, const cv::Mat &imageRight)
{
    if (imageLeft.empty() || imageRight.empty()) {
        return;
    }

    // Store images for alpha-factor or zero-calibration flag adjustment
    // update
    image1 = imageLeft;
    image2 = imageRight;

    displayTestImagePair();
}

void PageStereoResult::displayTestImagePair ()
{
    // Two simple remaps using look-up tables
    cv::Mat rectifiedImage1, rectifiedImage2;

    cv::remap(image1, rectifiedImage1, map11, map12, cv::INTER_LINEAR);
    cv::remap(image2, rectifiedImage2, map21, map22, cv::INTER_LINEAR);

    // Display undistorted image
    widgetImage->setImagePair(rectifiedImage1, rectifiedImage2);
    widgetImage->setImagePairROI(validRoi1, validRoi2);
}

void PageStereoResult::initializePage ()
{
    // Get stereo calibration
    cv::Mat M1 = field(fieldPrefix + "CameraMatrix1").value<cv::Mat>();
    cv::Mat D1 = field(fieldPrefix + "DistCoeffs1").value<cv::Mat>();
    cv::Mat M2 = field(fieldPrefix + "CameraMatrix2").value<cv::Mat>();
    cv::Mat D2 = field(fieldPrefix + "DistCoeffs2").value<cv::Mat>();

    // Display parameters
    widgetLeftCameraParameters->setCameraMatrix(M1, D1);
    widgetRightCameraParameters->setCameraMatrix(M2, D2);

    // Initialize stereo rectification
    initializeRectificationMaps();

    // Display last-processed calibration image pair
    cv::Mat imageLeft = field(fieldPrefix + "LastProcessedImageLeft").value<cv::Mat>();
    cv::Mat imageRight = field(fieldPrefix + "LastProcessedImageRight").value<cv::Mat>();
    if (imageLeft.empty() || imageRight.empty()) {
        qWarning() << this << ": failed to retrieved last-processed image pair!";
    }
    displayTestImagePair(imageLeft, imageRight);

    // Set caption
    if (field(fieldPrefix + "LiveCapture").toBool()) {
        labelCaption->setText("Congratulations! Stereo parameters have been calibrated. Last-processed calibration image pair has been rectified and is shown below for visual validation of calibration result. To load a new pair from the live source, double-click on the image widget.");
    } else {
        labelCaption->setText("Congratulations! Stereo parameters have been calibrated. Last-processed calibration image pair has been rectified and is shown below for visual validation of calibration result. To load a custom test image pair, double-click on the image widget.");
    }
}

void PageStereoResult::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Export"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageStereoResult::exportCalibration);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}


void PageStereoResult::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            ext = "yml";
            fileName += "." + ext;
        }

        try {
            QString fieldPrefix = "Stereo";

            // Export
            Pipeline::Rectification::exportStereoCalibration(fileName,
                field(fieldPrefix + "CameraMatrix1").value<cv::Mat>(),
                field(fieldPrefix + "DistCoeffs1").value<cv::Mat>(),
                field(fieldPrefix + "CameraMatrix2").value<cv::Mat>(),
                field(fieldPrefix + "DistCoeffs2").value<cv::Mat>(),
                field(fieldPrefix + "R").value<cv::Mat>(),
                field(fieldPrefix + "T").value<cv::Mat>(),
                field(fieldPrefix + "ImageSize").value<cv::Size>(),
                field(fieldPrefix + "RectificationZeroDisparity").value<bool>(),
                field(fieldPrefix + "RectificationAlpha").value<double>()
            );
        } catch (QString &e) {
            QMessageBox::warning(this, "Error", "Failed to export calibration: " + e);
        }
    }
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
