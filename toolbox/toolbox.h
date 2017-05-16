/*
 * MVL Stereo Toolbox: main toolbox object and window
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

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {

namespace Pipeline {
class ImagePairSource;
class Pipeline;
class PluginManager;
class StereoMethod;
} // Pipeline

namespace GUI {


class WindowImagePairSource;
class WindowRectification;
class WindowReprojection;
class WindowStereoMethod;
class WindowPointCloud;

class Toolbox : public QWidget
{
    Q_OBJECT

public:
    Toolbox ();
    virtual ~Toolbox ();

protected:
    void createGui ();
    void loadPlugins ();

    void displayError (int, const QString &);
    void clearError ();

    void showWindowOnTop (QWidget *);
    void setActiveButtonState (QPushButton *, bool);

protected:
    WindowImagePairSource *windowImagePairSource;
    WindowRectification *windowRectification;
    WindowStereoMethod *windowStereoMethod;
    WindowReprojection *windowReprojection;
    WindowPointCloud *windowPointCloud;

    QPushButton *pushButtonImagePairSource;
    QPushButton *pushButtonImagePairSourceActive;
    QPushButton *pushButtonRectification;
    QPushButton *pushButtonRectificationActive;
    QPushButton *pushButtonStereoMethod;
    QPushButton *pushButtonStereoMethodActive;
    QPushButton *pushButtonReprojection;
    QPushButton *pushButtonReprojectionActive;

    QPushButton *pushButtonPointCloud;

    QLabel *statusLabel;

    Pipeline::Pipeline *pipeline;

    Pipeline::PluginManager *plugin_manager;
    QList<QObject *> imagePairSources;
    QList<Pipeline::StereoMethod *> stereoMethods;
};


} // GUI
} // StereoToolbox
} // MVL


#endif
