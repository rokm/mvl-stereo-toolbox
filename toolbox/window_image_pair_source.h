/*
 * MVL Stereo Toolbox: image pair source window
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

#ifndef WINDOW_IMAGE_PAIR_SOURCE_H
#define WINDOW_IMAGE_PAIR_SOURCE_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class ImagePairSource;

class ImageDisplayWidget;

class WindowImagePairSource : public QWidget
{
    Q_OBJECT

public:
    WindowImagePairSource (StereoPipeline *, QList<ImagePairSource *> &, QWidget * = 0);
    virtual ~WindowImagePairSource ();

protected slots:
    void setSource (int);
    
    void updateImages ();

    void saveImages ();
    void snapshotImages ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    QList<ImagePairSource *> sources;

    QString snapshotBaseName;

    // GUI
    QPushButton *pushButtonSaveImages;
    QPushButton *pushButtonSnapshotImages;

    ImageDisplayWidget *displayImageLeft;
    ImageDisplayWidget *displayImageRight;

    QStatusBar *statusBar;
};

#endif