/*
 * MVL Stereo Toolbox: rectification window
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

#ifndef WINDOW_RECTIFICATION_H
#define WINDOW_RECTIFICATION_H

#include <QtWidgets>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class Pipeline;
class Rectification;
} // Pipeline

namespace GUI {

class CalibrationWizard;
class ImagePairDisplayWidget;
class RoiDialog;

class WindowRectification : public QWidget
{
    Q_OBJECT

public:
    WindowRectification (Pipeline::Pipeline *, Pipeline::Rectification *, QWidget * = 0);
    virtual ~WindowRectification ();

protected:
    void runCalibrationWizard ();
    void importCalibration ();
    void exportCalibration ();
    void clearCalibration ();
    void modifyRoi ();

    void saveImages ();

    void updateImage ();
    void updateState ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    Pipeline::Rectification *rectification;

    // GUI
    QPushButton *pushButtonWizard;
    QPushButton *pushButtonImport;
    QPushButton *pushButtonExport;
    QPushButton *pushButtonClear;
    QPushButton *pushButtonRoi;
    QPushButton *pushButtonSaveImages;

    QComboBox *comboBoxVisualizationMethod;

    ImagePairDisplayWidget *displayPair;

    QStatusBar *statusBar;
    QCheckBox *checkBoxRectifyImages;

    // ROI dialog
    RoiDialog *dialogRoi;

    // Wizard
    CalibrationWizard *wizard;

    // Cached calibration file filename
    QString lastCalibrationFilename;

    // Visualization type
    enum {
        VisualizationImagePair = 0,
        VisualizationAnaglyph = 1,
    };
    cv::Mat anaglyphImage;
};

class RoiDialog : public QDialog
{
    Q_OBJECT

public:
    RoiDialog (QWidget * = 0);
    virtual ~RoiDialog ();

    void setImageSizeAndRoi (const cv::Size &, const cv::Rect &);

    cv::Rect getRoi () const;

protected:
    void refreshDialog ();

protected:
    QCheckBox *checkBoxEnabled;
    QCheckBox *checkBoxCenter;
    QSpinBox *spinBoxX;
    QSpinBox *spinBoxY;
    QSpinBox *spinBoxW;
    QSpinBox *spinBoxH;

    cv::Size imageSize;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
