/*
 * MVL Stereo Toolbox: reprojection window
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_REPROJECTION_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__WINDOW_REPROJECTION_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class Reprojection;
class Pipeline;
} // Pipeline

namespace Widgets {
class ReprojectionDisplayWidget;
} // Widgets

namespace GUI {


class WindowReprojection : public QWidget
{
    Q_OBJECT

public:
    WindowReprojection (Pipeline::Pipeline *pipeline, QWidget *parent = Q_NULLPTR);
    virtual ~WindowReprojection ();

protected:
    void saveReprojectionResult ();

    void fillReprojectionMethods ();

    void updateStatusBar ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    Pipeline::Reprojection *reprojection;

    struct {
        bool valid;
        int width;
        int height;
    } pointsInfo;
    int numDroppedFrames;
    float estimatedFps;

    // GUI
    QComboBox *comboBoxImage;
    QComboBox *comboBoxReprojectionMethod;
    QPushButton *pushButtonSaveReprojection;

    Widgets::ReprojectionDisplayWidget *displayReprojectedImage;

    QLabel *labelCoordinates;
    QStatusBar *statusBar;

    QString lastSavedFile;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
