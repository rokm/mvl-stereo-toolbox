/*
 * MVL Stereo Toolbox: stereo method window
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

#ifndef WINDOW_STEREO_METHOD_H
#define WINDOW_STEREO_METHOD_H

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
    WindowStereoMethod (Pipeline::Pipeline *, QList<QObject *> &, QWidget * = 0);
    virtual ~WindowStereoMethod ();

protected:
    void setMethod (int);

    void saveImage ();

    void importParameters ();
    void exportParameters ();

    void fillVisualizationMethods ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    QList<QObject *> methods;
    Pipeline::DisparityVisualization *visualization;

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
