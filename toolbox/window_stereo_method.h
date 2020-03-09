/*
 * MVL Stereo Toolbox: stereo method window
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_STEREO_METHOD_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_STEREO_METHOD_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class Pipeline;
class StereoMethod;
class DisparityVisualization;
} // Pipeline

namespace Widgets {
class DisparityDisplayWidget;
} // Widgets

namespace GUI {


class WindowStereoMethod : public QWidget
{
    Q_OBJECT

public:
    WindowStereoMethod (Pipeline::Pipeline *pipeline, QList<QObject *> &methods, QWidget *parent = nullptr);
    virtual ~WindowStereoMethod ();

protected:
    void setMethod (int idx);

    void saveImage ();

    void importParameters ();
    void exportParameters ();

    void fillVisualizationMethods ();

    void updateStatusBar ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    QList<QObject *> methods;
    Pipeline::DisparityVisualization *visualization;

    // Status bar info
    struct {
        bool valid;
        int width;
        int height;
        int depth;
    } disparityInfo;
    int numDroppedFrames;
    float estimatedFps;

    // GUI
    QPushButton *pushButtonExportParameters;
    QPushButton *pushButtonImportParameters;
    QComboBox *comboBoxVisualizationMethod;
    QPushButton *pushButtonSaveImage;

    Widgets::DisparityDisplayWidget *displayDisparityImage;

    QLabel *labelDisparity;
    QStatusBar *statusBar;

    QString lastSavedFile;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
