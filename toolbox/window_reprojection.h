/*
 * MVL Stereo Toolbox: reprojection window
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

#ifndef WINDOW_REPROJECTION_H
#define WINDOW_REPROJECTION_H

#include <QtWidgets>


class StereoReprojection;
class StereoPipeline;
class ReprojectedImageDisplayWidget;

class WindowReprojection : public QWidget
{
    Q_OBJECT

public:
    WindowReprojection (StereoPipeline *, StereoReprojection *, QWidget * = 0);
    virtual ~WindowReprojection ();

protected slots:
    void updateDisplayBackground ();
    void updateDisplayValues ();

    void displayCoordinates (const QVector3D &);

    void reprojectionMethodChanged (int);
    void updateReprojectionMethod (int);

    void saveReprojectionResult ();

protected:
    void fillReprojectionMethods ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    StereoReprojection *reprojection;

    // GUI
    QComboBox *comboBoxImage;
    QComboBox *comboBoxReprojectionMethod;
    QPushButton *pushButtonSaveReprojection;

    ReprojectedImageDisplayWidget *displayReprojectedImage;

    QLabel *labelCoordinates;
    QStatusBar *statusBar;

    QString lastSavedFile;
};

#endif
