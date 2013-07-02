/*
 * MVL Stereo Toolbox: stereo method window
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

#ifndef WINDOW_STEREO_METHOD_H
#define WINDOW_STEREO_METHOD_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class StereoMethod;

class ImageDisplayWidget;

class WindowStereoMethod : public QWidget
{
    Q_OBJECT

public:
    WindowStereoMethod (StereoPipeline *, QList<StereoMethod *> &, QWidget * = 0);
    virtual ~WindowStereoMethod ();

protected slots:
    void setMethod (int);
    
    void updateImage ();

    void saveImage ();

    void importParameters ();
    void exportParameters ();

protected:
    enum {
        DisplayRawDisparity,
        DisplayColorGpuDisparity,
    };

    // Pipeline
    StereoPipeline *pipeline;
    QList<StereoMethod *> methods;

    // GUI
    QPushButton *pushButtonExportParameters;
    QPushButton *pushButtonImportParameters;
    QComboBox *comboBoxDisplayType;
    QPushButton *pushButtonSaveImage;
    
    ImageDisplayWidget *displayDisparityImage;

    QStatusBar *statusBar;
};

#endif
