/*
 * MVL Stereo Toolbox: rectification GUI
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

#ifndef GUI_RECTIFICATION_H
#define GUI_RECTIFICATION_H

#include <QtCore>
#include <QtGui>

#include "StereoRectification.h"


class CalibrationWizard;
class ImagePairDisplayWidget;
class RoiDialog;
class StereoPipeline;

class GuiRectification : public QWidget
{
    Q_OBJECT

public:
    GuiRectification (StereoPipeline *, StereoRectification *, QWidget * = 0);
    virtual ~GuiRectification ();

protected slots:
    void runCalibrationWizard ();
    void importCalibration ();
    void exportCalibration ();
    void modifyRoi ();
    void clearCalibration ();

    void saveImages ();

    void updateImage ();
    void updateState ();
    void updateRoi ();

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

    // ROI dialog
    RoiDialog *dialogRoi;

    // Wizard
    CalibrationWizard *wizard;
};

class RoiDialog : public QDialog
{
    Q_OBJECT

public:
    RoiDialog (QWidget * = 0);
    virtual ~RoiDialog ();

    cv::Size getRoiSize () const;
    void setRoiSize (const cv::Size &);

protected:
    QSpinBox *spinBoxWidth;
    QSpinBox *spinBoxHeight;
};

#endif
