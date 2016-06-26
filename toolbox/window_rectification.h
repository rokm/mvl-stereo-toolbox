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

namespace Widgets {
class ImagePairDisplayWidget;
} // Widgets

namespace GUI {


class CalibrationWizard;
class RectificationSettingsDialog;

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
    void modifyRectificationSettings ();

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
    QPushButton *pushButtonRectificationSettings;
    QPushButton *pushButtonSaveImages;

    QComboBox *comboBoxVisualizationMethod;

    Widgets::ImagePairDisplayWidget *displayPair;

    QStatusBar *statusBar;
    QCheckBox *checkBoxRectifyImages;

    // Rectification settings dialog
    RectificationSettingsDialog *dialogSettings;

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

class RectificationSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    RectificationSettingsDialog (QWidget * = 0);
    virtual ~RectificationSettingsDialog();

    void setAlpha (float);
    float getAlpha () const;

    void setZeroDisparity (bool);
    bool getZeroDisparity () const;

protected:
    QDoubleSpinBox *spinBoxAlpha;
    QCheckBox *checkBoxZeroDisparity;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
