/*
 * MVL Stereo Toolbox: rectification GUI
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

#ifndef GUI_RECTIFICATION_H
#define GUI_RECTIFICATION_H

#include <QtCore>
#include <QtGui>

#include "StereoRectification.h"


class CalibrationWizard;
class ImagePairDisplayWidget;
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
    void clearCalibration ();

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
    QPushButton *pushButtonSaveImages;
    
    ImagePairDisplayWidget *displayPair;

    QStatusBar *statusBar;

    // Wizard
    CalibrationWizard *wizard;
};

#endif
