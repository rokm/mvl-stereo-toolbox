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

#include <stereo-pipeline/image_pair_source.h>
#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/plugin_factory.h>
#include <stereo-pipeline/plugin_manager.h>
#include <stereo-pipeline/rectification.h>
#include <stereo-pipeline/reprojection.h>
#include <stereo-pipeline/stereo_method.h>

#include "window_image_pair_source.h"
#include "window_rectification.h"
#include "window_reprojection.h"
#include "window_stereo_method.h"
#include "window_point_cloud.h"


namespace MVL {
namespace StereoToolbox {
namespace GUI {


Toolbox::Toolbox (QWidget *parent)
    : QWidget(parent)
{
    // Resize window
    resize(200, 100);
    setWindowTitle("MVL Stereo Toolbox");

    // Plugin manager
    plugin_manager = new Pipeline::PluginManager(this);

    // Create pipeline
    pipeline = new Pipeline::Pipeline(this);

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

    // Create windows; set Qt::WA_QuitOnClose to false so that closing
    // the main window will close the application (these windows are
    // unparented!)
    windowImagePairSource = new WindowImagePairSource(pipeline, imagePairSources);
    windowImagePairSource->setAttribute(Qt::WA_QuitOnClose, false);

    windowRectification = new WindowRectification(pipeline);
    windowRectification->setAttribute(Qt::WA_QuitOnClose, false);

    windowStereoMethod = new WindowStereoMethod(pipeline, stereoMethods);
    windowStereoMethod->setAttribute(Qt::WA_QuitOnClose, false);

    windowReprojection = new WindowReprojection(pipeline);
    windowReprojection->setAttribute(Qt::WA_QuitOnClose, false);

    windowPointCloud = new WindowPointCloud(pipeline);
    windowPointCloud->setAttribute(Qt::WA_QuitOnClose, false);

    // Create GUI
    createGui();
}

Toolbox::~Toolbox ()
{
    delete windowImagePairSource;
    delete windowRectification;
    delete windowStereoMethod;
    delete windowReprojection;
    delete windowPointCloud;
}

void Toolbox::createGui ()
{
    QGridLayout *layout = new QGridLayout(this);
    QFrame *line;
    int row = 0;

    // Status text
    statusLabel = new QLabel("Status:", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    connect(pipeline, &Pipeline::Pipeline::error, this, &Toolbox::displayError);
    connect(pipeline, &Pipeline::Pipeline::processingCompleted, this, &Toolbox::clearError);

    layout->addWidget(statusLabel, row, 0, 1, 2);
    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, row, 0, 1, 2);
    row++;

    // Image pair source
    pushButtonImagePairSource = new QPushButton("Image pair source", this);
    connect(pushButtonImagePairSource, &QPushButton::clicked, this, [this] () {
        showWindowOnTop(windowImagePairSource);
    });

    pushButtonImagePairSourceActive = new QPushButton("Active", this);
    pushButtonImagePairSourceActive->setCheckable(true);

    pushButtonImagePairSourceActive->setChecked(pipeline->getImagePairSourceState());
    connect(pushButtonImagePairSourceActive, &QPushButton::toggled, pipeline, &Pipeline::Pipeline::setImagePairSourceState);
    connect(pipeline, &Pipeline::Pipeline::imagePairSourceStateChanged, this, [this] (bool active) {
        setActiveButtonState(pushButtonImagePairSourceActive, active);
    });

    layout->addWidget(pushButtonImagePairSource, row, 0);
    layout->addWidget(pushButtonImagePairSourceActive, row, 1);
    row++;

    // Rectification
    pushButtonRectification = new QPushButton("Rectification", this);
    connect(pushButtonRectification, &QPushButton::clicked, this, [this] () {
        showWindowOnTop(windowRectification);
    });

    pushButtonRectificationActive = new QPushButton("Active", this);
    pushButtonRectificationActive->setCheckable(true);

    pushButtonRectificationActive->setChecked(pipeline->getRectificationState());
    connect(pushButtonRectificationActive, &QPushButton::toggled, pipeline, &Pipeline::Pipeline::setRectificationState);
    connect(pipeline, &Pipeline::Pipeline::rectificationStateChanged, this, [this] (bool active) {
        setActiveButtonState(pushButtonRectificationActive, active);
    });

    layout->addWidget(pushButtonRectification, row, 0);
    layout->addWidget(pushButtonRectificationActive, row, 1);
    row++;

    // Stereo method
    pushButtonStereoMethod = new QPushButton("Stereo method", this);
    connect(pushButtonStereoMethod, &QPushButton::clicked, this, [this] () {
        showWindowOnTop(windowStereoMethod);
    });

    pushButtonStereoMethodActive = new QPushButton("Active", this);
    pushButtonStereoMethodActive->setCheckable(true);

    pushButtonStereoMethodActive->setChecked(pipeline->getStereoMethodState());
    connect(pushButtonStereoMethodActive, &QPushButton::toggled, pipeline, &Pipeline::Pipeline::setStereoMethodState);
    connect(pipeline, &Pipeline::Pipeline::stereoMethodStateChanged, this, [this] (bool active) {
        setActiveButtonState(pushButtonStereoMethodActive, active);
    });

    layout->addWidget(pushButtonStereoMethod, row, 0);
    layout->addWidget(pushButtonStereoMethodActive, row, 1);
    row++;

    // Reprojection
    pushButtonReprojection = new QPushButton("Reprojection", this);
    connect(pushButtonReprojection, &QPushButton::clicked, this, [this] () {
        showWindowOnTop(windowReprojection);
    });

    pushButtonReprojectionActive = new QPushButton("Active", this);
    pushButtonReprojectionActive->setCheckable(true);

    pushButtonReprojectionActive->setChecked(pipeline->getReprojectionState());
    connect(pushButtonReprojectionActive, &QPushButton::toggled, pipeline, &Pipeline::Pipeline::setReprojectionState);
    connect(pipeline, &Pipeline::Pipeline::reprojectionStateChanged, this, [this] (bool active) {
        setActiveButtonState(pushButtonReprojectionActive, active);
    });

    layout->addWidget(pushButtonReprojection, row, 0);
    layout->addWidget(pushButtonReprojectionActive, row, 1);
    row++;

    // Point cloud
    pushButtonPointCloud = new QPushButton("Point cloud", this);
    connect(pushButtonPointCloud, &QPushButton::clicked, this, [this] () {
        showWindowOnTop(windowPointCloud);
    });


    layout->addWidget(pushButtonPointCloud, row, 0);
    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, row, 0, 1, 2);
    row++;

    // Exit
    QPushButton *pushButtonExit = new QPushButton("Exit", this);
    connect(pushButtonExit, &QPushButton::clicked, qApp, &QApplication::quit);

    layout->addWidget(pushButtonExit, row, 0, 1, 2);
    row++;
}


// *********************************************************************
// *                          Window showing                           *
// *********************************************************************
void Toolbox::showWindowOnTop (QWidget *window)
{
    window->show();
    window->activateWindow();
    window->raise();
}


// *********************************************************************
// *                      Active buttons states                        *
// *********************************************************************
void Toolbox::setActiveButtonState (QPushButton *button, bool active)
{
    button->blockSignals(true);
    button->setChecked(active);
    if (active) {
        button->setText("Active");
    } else {
        button->setText("Inactive");
    }
    button->blockSignals(false);
}


// *********************************************************************
// *                         Status message                            *
// *********************************************************************
void Toolbox::displayError (int errorType, const QString &message)
{
    statusLabel->setText("Status: ERROR");

    // If this is a general error, show it in the status bar; otherwise,
    // the corresponding sub window will pop up an error dialog
    if (errorType == Pipeline::Pipeline::ErrorGeneral) {
        statusLabel->setToolTip(message);
    }

    // Stop the image pair source, so that the user can resolve the
    // error
    /*Pipeline::ImagePairSource *imagePairSource = pipeline->getImagePairSource();
    if (imagePairSource) {
        imagePairSource->stopSource();
    }*/
    pipeline->setImagePairSourceState(false);
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
    for (QObject *plugin : plugin_manager->getAvailablePlugins()) {
        Pipeline::PluginFactory *factory = qobject_cast<Pipeline::PluginFactory *>(plugin);
        QObject *object = Q_NULLPTR;

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
                    stereoMethods.append(object);
                    break;
                }
                case Pipeline::PluginFactory::PluginImagePairSource: {
                    imagePairSources.append(object);
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
