/*
 * MVL Stereo Toolbox: rectification window
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

#ifndef WINDOW_RECTIFICATION_H
#define WINDOW_RECTIFICATION_H

#include <QtCore>
#include <QtGui>

#include <opencv2/core/core.hpp>


class CalibrationWizard;
class ImagePairDisplayWidget;
class RoiDialog;
class StereoPipeline;
class StereoRectification;

class WindowRectification : public QWidget
{
    Q_OBJECT

public:
    WindowRectification (StereoPipeline *, StereoRectification *, QWidget * = 0);
    virtual ~WindowRectification ();

protected slots:
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
    StereoPipeline *pipeline;
    StereoRectification *rectification;

    // GUI
    QPushButton *pushButtonWizard;
    QPushButton *pushButtonImport;
    QPushButton *pushButtonExport;
    QPushButton *pushButtonClear;
    QPushButton *pushButtonRoi;
    QPushButton *pushButtonSaveImages;
    
    ImagePairDisplayWidget *displayPair;

    QStatusBar *statusBar;
    QCheckBox *checkBoxRectifyImages;

    // ROI dialog
    RoiDialog *dialogRoi;

    // Wizard
    CalibrationWizard *wizard;

    // Cached calibration file filename
    QString lastCalibrationFilename;
};

class RoiDialog : public QDialog
{
    Q_OBJECT

public:
    RoiDialog (QWidget * = 0);
    virtual ~RoiDialog ();

    void setImageSizeAndRoi (const cv::Size &, const cv::Rect &);

    cv::Rect getRoi () const;

protected slots:
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

#endif
