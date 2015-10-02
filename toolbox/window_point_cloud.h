/*
 * MVL Stereo Toolbox: point cloud window
 * Copyright (C) 2015 Rok Mandeljc
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

#ifndef WINDOW_POINT_CLOUD_H
#define WINDOW_POINT_CLOUD_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class Pipeline;
} // Pipeline

namespace GUI {


class WindowPointCloud : public QWidget
{
    Q_OBJECT

public:
    WindowPointCloud (Pipeline::Pipeline *, QWidget * = 0);
    virtual ~WindowPointCloud ();

protected:
    // Pipeline
    Pipeline::Pipeline *pipeline;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
