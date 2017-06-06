/*
 * MVL Stereo Toolbox: image pair source window
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

#ifndef WINDOW_IMAGE_PAIR_SOURCE_H
#define WINDOW_IMAGE_PAIR_SOURCE_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class Pipeline;
class ImagePairSource;
} // Pipeline

namespace Widgets {
    class ImageDisplayWidget;
} // Widgets

namespace GUI {


class WindowImagePairSource : public QWidget
{
    Q_OBJECT

public:
    WindowImagePairSource (Pipeline::Pipeline *, QList<QObject *> &, QWidget * = 0);
    virtual ~WindowImagePairSource ();

protected:
    void setSource (int);

    void saveImages ();
    void snapshotImages ();

    void selectSnapshotFilename ();

    void updateStatusBar ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
    QList<QObject *> sources;

    struct {
        bool valid;
        int width;
        int height;
        int depth;
    } leftInfo, rightInfo;
    int numDroppedFrames;
    float estimatedFps;

    QString snapshotBaseName;

    // GUI
    Widgets::ImageDisplayWidget *displayImageLeft;
    Widgets::ImageDisplayWidget *displayImageRight;

    QStatusBar *statusBar;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
