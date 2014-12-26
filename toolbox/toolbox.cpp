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

#include "toolbox.h"

#include <image_pair_source.h>
#include <plugin_factory.h>
#include <plugin_manager.h>
#include <stereo_method.h>
#include <stereo_pipeline.h>
#include <stereo_rectification.h>
#include <stereo_reprojection.h>

#include "window_image_pair_source.h"
#include "window_rectification.h"
#include "window_reprojection.h"
#include "window_stereo_method.h"


namespace MVL {
namespace StereoToolbox {
namespace GUI {


Toolbox::Toolbox ()
    : QWidget()
{
    // Resize window
    resize(200, 100);
    setWindowTitle("MVL Stereo Toolbox");

    // Plugin manager
    plugin_manager = new Pipeline::PluginManager(this);

    // Create pipeline
    pipeline = new Pipeline::StereoPipeline(this);
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
    QFrame *line;
    int row = 0;

    // Status text
    statusLabel = new QLabel("Status:", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    connect(pipeline, SIGNAL(error(const QString)), this, SLOT(displayError(const QString)));
    connect(pipeline, SIGNAL(processingCompleted()), this, SLOT(clearError()));

    layout->addWidget(statusLabel, row, 0, 1, 2);
    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, row, 0, 1, 2);
    row++;

    // Image pair source
    pushButtonImagePairSource = new QPushButton("Image pair source", this);
    connect(pushButtonImagePairSource, SIGNAL(clicked()), this, SLOT(showWindowImagePairSource()));

    pushButtonImagePairSourceActive = new QPushButton("Active", this);
    pushButtonImagePairSourceActive->setCheckable(true);

    pushButtonImagePairSourceActive->setChecked(pipeline->getImagePairSourceState());
    connect(pushButtonImagePairSourceActive, SIGNAL(toggled(bool)), pipeline, SLOT(setImagePairSourceState(bool)));
    connect(pipeline, SIGNAL(imagePairSourceStateChanged(bool)), this, SLOT(setPushButtonImagePairSourceActiveState(bool)));

    layout->addWidget(pushButtonImagePairSource, row, 0);
    layout->addWidget(pushButtonImagePairSourceActive, row, 1);
    row++;

    // Rectification
    pushButtonRectification = new QPushButton("Rectification", this);
    connect(pushButtonRectification, SIGNAL(clicked()), this, SLOT(showWindowRectification()));

    pushButtonRectificationActive = new QPushButton("Active", this);
    pushButtonRectificationActive->setCheckable(true);

    pushButtonRectificationActive->setChecked(pipeline->getRectificationState());
    connect(pushButtonRectificationActive, SIGNAL(toggled(bool)), pipeline, SLOT(setRectificationState(bool)));
    connect(pipeline, SIGNAL(rectificationStateChanged(bool)), this, SLOT(setPushButtonRectificationActiveState(bool)));

    layout->addWidget(pushButtonRectification, row, 0);
    layout->addWidget(pushButtonRectificationActive, row, 1);
    row++;

    // Stereo method
    pushButtonStereoMethod = new QPushButton("Stereo method", this);
    connect(pushButtonStereoMethod, SIGNAL(clicked()), this, SLOT(showWindowStereoMethod()));

    pushButtonStereoMethodActive = new QPushButton("Active", this);
    pushButtonStereoMethodActive->setCheckable(true);

    pushButtonStereoMethodActive->setChecked(pipeline->getStereoMethodState());
    connect(pushButtonStereoMethodActive, SIGNAL(toggled(bool)), pipeline, SLOT(setStereoMethodState(bool)));
    connect(pipeline, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(setPushButtonStereoMethodActiveState(bool)));

    layout->addWidget(pushButtonStereoMethod, row, 0);
    layout->addWidget(pushButtonStereoMethodActive, row, 1);
    row++;

    // Reprojection/point cloud
    pushButtonReprojection = new QPushButton("Reprojection", this);
    connect(pushButtonReprojection, SIGNAL(clicked()), this, SLOT(showWindowReprojection()));

    pushButtonReprojectionActive = new QPushButton("Active", this);
    pushButtonReprojectionActive->setCheckable(true);

    pushButtonReprojectionActive->setChecked(pipeline->getReprojectionState());
    connect(pushButtonReprojectionActive, SIGNAL(toggled(bool)), pipeline, SLOT(setReprojectionState(bool)));
    connect(pipeline, SIGNAL(reprojectionStateChanged(bool)), this, SLOT(setPushButtonReprojectionActiveState(bool)));

    layout->addWidget(pushButtonReprojection, row, 0);
    layout->addWidget(pushButtonReprojectionActive, row, 1);
    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, row, 0, 1, 2);
    row++;

    // Exit
    QPushButton *pushButtonExit = new QPushButton("Exit", this);
    connect(pushButtonExit, SIGNAL(clicked()), qApp, SLOT(quit()));

    layout->addWidget(pushButtonExit, row, 0, 1, 2);
    row++;
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
// *                         Status message                            *
// *********************************************************************
void Toolbox::displayError (const QString error)
{
    statusLabel->setText("Status: ERROR");
    statusLabel->setToolTip(error);
}

void Toolbox::clearError ()
{
    statusLabel->setText("Status: operational");
    statusLabel->setToolTip("");
}


// *********************************************************************
// *                          Plugin loading                           *
// *********************************************************************
void Toolbox::loadPlugins ()
{
    foreach (QObject *plugin, plugin_manager->getAvailablePlugins()) {
        Pipeline::PluginFactory *factory = qobject_cast<Pipeline::PluginFactory *>(plugin);
        QObject *object = NULL;

        // Create object
        try {
            object = factory->createObject(this);
        } catch (...) {
            continue;
        }

        // Insert object into corresponding list
        if (object) {
            switch (factory->getPluginType()) {
                case Pipeline::PluginFactory::PluginStereoMethod: {
                    stereoMethods.append(qobject_cast<Pipeline::StereoMethod *>(object));
                    break;
                }
                case Pipeline::PluginFactory::PluginImagePairSource: {
                    imagePairSources.append(qobject_cast<Pipeline::ImagePairSource *>(object));
                    break;
                }
                default: {
                    // Unhandled object; delete it
                    object->deleteLater();
                    break;
                }
            }
        }
    }
}


} // GUI
} // StereoToolbox
} // MVL
