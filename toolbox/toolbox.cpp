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

#include "toolbox.h"

#include <image_pair_source.h>
#include <plugin_factory.h>
#include <stereo_method.h>
#include <stereo_pipeline.h>
#include <stereo_rectification.h>
#include <stereo_reprojection.h>

#include "window_image_pair_source.h"
#include "window_rectification.h"
#include "window_reprojection.h"
#include "window_stereo_method.h"


Toolbox::Toolbox ()
    : QWidget()
{
    // Create pipeline
    pipeline = new StereoPipeline(this);
    pipeline->setUseStereoMethodThread(true);

    // If available, initialize first GPU
    if (pipeline->getNumberOfGpuDevices()) {
        qDebug() << "";
        qDebug() << "Initializing GPU. This might take a while...";
        pipeline->setGpuDevice(0);
        qDebug() << "GPU initialized!";
        qDebug() << "";
    }

    // Load plugins
    loadPlugins();

    // Create windows
    windowImagePairSource = new WindowImagePairSource(pipeline, imagePairSources, this);
    windowRectification = new WindowRectification(pipeline, pipeline->getRectification(), this);
    windowStereoMethod = new WindowStereoMethod(pipeline, stereoMethods, this);
    windowReprojection = new WindowReprojection(pipeline, pipeline->getReprojection(), this);

    // Create GUI
    createGui();
}

Toolbox::~Toolbox ()
{
}

void Toolbox::createGui ()
{
    QGridLayout *layout = new QGridLayout(this);

    // Image pair source
    pushButtonImagePairSource = new QPushButton("Image pair source", this);
    connect(pushButtonImagePairSource, SIGNAL(clicked()), this, SLOT(showWindowImagePairSource()));
    
    pushButtonImagePairSourceActive = new QPushButton("Active", this);
    pushButtonImagePairSourceActive->setCheckable(true);

    pushButtonImagePairSourceActive->setChecked(pipeline->getImagePairSourceState());
    connect(pushButtonImagePairSourceActive, SIGNAL(toggled(bool)), pipeline, SLOT(setImagePairSourceState(bool)));
    connect(pipeline, SIGNAL(imagePairSourceStateChanged(bool)), this, SLOT(setPushButtonImagePairSourceActiveState(bool)));
    
    layout->addWidget(pushButtonImagePairSource, 0, 0);
    layout->addWidget(pushButtonImagePairSourceActive, 0, 1);

    // Rectification
    pushButtonRectification = new QPushButton("Rectification", this);
    connect(pushButtonRectification, SIGNAL(clicked()), this, SLOT(showWindowRectification()));

    pushButtonRectificationActive = new QPushButton("Active", this);
    pushButtonRectificationActive->setCheckable(true);

    pushButtonRectificationActive->setChecked(pipeline->getRectificationState());
    connect(pushButtonRectificationActive, SIGNAL(toggled(bool)), pipeline, SLOT(setRectificationState(bool)));
    connect(pipeline, SIGNAL(rectificationStateChanged(bool)), this, SLOT(setPushButtonRectificationActiveState(bool)));
        
    layout->addWidget(pushButtonRectification, 1, 0);
    layout->addWidget(pushButtonRectificationActive, 1, 1);

    // Stereo method
    pushButtonStereoMethod = new QPushButton("Stereo method", this);
    connect(pushButtonStereoMethod, SIGNAL(clicked()), this, SLOT(showWindowStereoMethod()));

    pushButtonStereoMethodActive = new QPushButton("Active", this);
    pushButtonStereoMethodActive->setCheckable(true);

    pushButtonStereoMethodActive->setChecked(pipeline->getStereoMethodState());
    connect(pushButtonStereoMethodActive, SIGNAL(toggled(bool)), pipeline, SLOT(setStereoMethodState(bool)));
    connect(pipeline, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(setPushButtonStereoMethodActiveState(bool)));
        
    layout->addWidget(pushButtonStereoMethod, 2, 0);
    layout->addWidget(pushButtonStereoMethodActive, 2, 1);

    // Reprojection/point cloud
    pushButtonReprojection = new QPushButton("Reprojection", this);
    connect(pushButtonReprojection, SIGNAL(clicked()), this, SLOT(showWindowReprojection()));

    pushButtonReprojectionActive = new QPushButton("Active", this);
    pushButtonReprojectionActive->setCheckable(true);

    pushButtonReprojectionActive->setChecked(pipeline->getReprojectionState());
    connect(pushButtonReprojectionActive, SIGNAL(toggled(bool)), pipeline, SLOT(setReprojectionState(bool)));
    connect(pipeline, SIGNAL(reprojectionStateChanged(bool)), this, SLOT(setPushButtonReprojectionActiveState(bool)));

    layout->addWidget(pushButtonReprojection, 3, 0);
    layout->addWidget(pushButtonReprojectionActive, 3, 1);

    // Separator
    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, 4, 0, 1, 2);

    // Exit
    QPushButton *pushButtonExit = new QPushButton("Exit", this);
    connect(pushButtonExit, SIGNAL(clicked()), qApp, SLOT(quit()));

    layout->addWidget(pushButtonExit, 5, 0, 1, 2);
}


// *********************************************************************
// *                          Window showing                           *
// *********************************************************************
static void showWindowOnTop (QWidget *window)
{
    window->show();
    window->activateWindow();
    window->raise();
}

void Toolbox::showWindowImagePairSource ()
{
    showWindowOnTop(windowImagePairSource);
}

void Toolbox::showWindowRectification ()
{
    showWindowOnTop(windowRectification);

}

void Toolbox::showWindowStereoMethod ()
{
    showWindowOnTop(windowStereoMethod);
}

void Toolbox::showWindowReprojection ()
{
    showWindowOnTop(windowReprojection);
}


// *********************************************************************
// *                      Active buttons states                        *
// *********************************************************************
static void setActiveButtonState (QPushButton *button, bool active)
{
    button->setChecked(active);
    if (active) {
        button->setText("Active");
    } else {
        button->setText("Inactive");
    }
}

void Toolbox::setPushButtonImagePairSourceActiveState (bool active)
{
    setActiveButtonState(pushButtonImagePairSourceActive, active);
}

void Toolbox::setPushButtonRectificationActiveState (bool active)
{
    setActiveButtonState(pushButtonRectificationActive, active);
}

void Toolbox::setPushButtonStereoMethodActiveState (bool active)
{
    setActiveButtonState(pushButtonStereoMethodActive, active);
}

void Toolbox::setPushButtonReprojectionActiveState (bool active)
{
    setActiveButtonState(pushButtonReprojectionActive, active);
}


// *********************************************************************
// *                          Plugin loading                           *
// *********************************************************************
void Toolbox::loadPlugins ()
{
    foreach (PluginFactory *plugin, pipeline->getAvailablePlugins()) {
        switch (plugin->getPluginType()) {
            case PluginFactory::PluginStereoMethod: {
                QObject *method = plugin->createObject(this);
                if (method) {
                    stereoMethods.append(qobject_cast<StereoMethod *>(method));
                }
                break;
            }
            case PluginFactory::PluginImagePairSource: {
                QObject *source = plugin->createObject(this);
                if (source) {
                    imagePairSources.append(qobject_cast<ImagePairSource *>(source));
                }
            }
            default: {
                break;
            }            
        }
    }
}
