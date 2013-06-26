/*
 * MVL Stereo Toolbox: main toolbox object and window
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

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>


class ImagePairSource;
class StereoRectification;
class StereoMethod;
class StereoPipeline;
class StereoReprojection;

class GuiImagePairSource;
class GuiRectification;
class GuiReprojection;
class GuiStereoMethod;

class Toolbox : public QWidget
{
    Q_OBJECT

public:
    Toolbox ();
    virtual ~Toolbox ();

protected slots:
    void showWindowImagePairSource ();
    void showWindowRectification ();
    void showWindowStereoMethod ();
    void showWindowReprojection ();

    void setPushButtonImagePairSourceActiveState (bool);
    void setPushButtonRectificationActiveState (bool);
    void setPushButtonStereoMethodActiveState (bool);
    void setPushButtonReprojectionActiveState (bool);

protected:
    void createGui ();

    void loadPlugins ();
    
protected:
    GuiImagePairSource *windowImagePairSource;
    GuiRectification *windowRectification;
    GuiStereoMethod *windowStereoMethod;
    GuiReprojection *windowReprojection;

    QPushButton *pushButtonImagePairSource;
    QPushButton *pushButtonImagePairSourceActive;
    QPushButton *pushButtonRectification;
    QPushButton *pushButtonRectificationActive;
    QPushButton *pushButtonStereoMethod;
    QPushButton *pushButtonStereoMethodActive;
    QPushButton *pushButtonReprojection;
    QPushButton *pushButtonReprojectionActive;
    
    StereoPipeline *pipeline;

    QList<ImagePairSource *> imagePairSources;
    QList<StereoMethod *> stereoMethods;
};


#endif
