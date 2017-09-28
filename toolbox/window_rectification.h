/*
 * MVL Stereo Toolbox: rectification window
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_RECTIFICATION_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_RECTIFICATION_H

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


namespace CalibrationWizard {
class Wizard;
} // CalibrationWizard


class RectificationSettingsDialog;

class WindowRectification : public QWidget
{
    Q_OBJECT

public:
    WindowRectification (Pipeline::Pipeline *pipeline, QWidget *parent = Q_NULLPTR);
    virtual ~WindowRectification ();

protected:
    void runCalibrationWizard ();
    void importCalibration ();
    void exportCalibration ();
    void clearCalibration ();
    void modifyRectificationSettings ();

    void saveImages ();

    void updateImage ();
    void updateStatusBar ();
    void updateButtonsState ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    Pipeline::Rectification *rectification;

    int numDroppedFrames;
    float estimatedFps;

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
    CalibrationWizard::Wizard *wizard;

    // Cached calibration file filename
    QString lastCalibrationFilename;

    // Visualization type
    enum VisualizationType {
        VisualizationImagePair = 0,
        VisualizationAnaglyph = 1,
    };
    cv::Mat anaglyphImage;
};

class RectificationSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    RectificationSettingsDialog (QWidget *parent = Q_NULLPTR);
    virtual ~RectificationSettingsDialog();

    void setAlpha (float alpha);
    float getAlpha () const;

    void setZeroDisparity (bool enable);
    bool getZeroDisparity () const;

protected:
    QDoubleSpinBox *spinBoxAlpha;
    QCheckBox *checkBoxZeroDisparity;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
