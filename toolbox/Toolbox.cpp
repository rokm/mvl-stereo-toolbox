/*
 * MVL Stereo Toolbox: main toolbox object and window
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "Toolbox.h"

#include "ImageSource.h"
#include "StereoCalibration.h"
#include "StereoMethod.h"
#include "StereoPipeline.h"

#include "GuiImageSource.h"
#include "GuiCalibration.h"
#include "GuiStereoMethod.h"


Toolbox::Toolbox ()
    : QWidget()
{
    // Create pipeline
    pipeline = new StereoPipeline(this);
    pipeline->setUseStereoMethodThread(true);

    calibration = new StereoCalibration(pipeline);
    pipeline->setCalibration(calibration);

    // Load image source and method plugins; look for path stored in
    // MVL_STEREO_TOOLBOX_PLUGIN_DIR, otherwise use application path
    QDir pluginRoot;
    QByteArray pluginEnvVariable = qgetenv ("MVL_STEREO_TOOLBOX_PLUGIN_DIR");
    if (!pluginEnvVariable.isEmpty()) {
        pluginRoot = QDir(pluginEnvVariable);
    } else {
        pluginRoot = QDir(qApp->applicationDirPath());
    }

    loadSources(pluginRoot, imageSources);
    loadMethods(pluginRoot, stereoMethods);

    // Create windows
    windowImageSource = new GuiImageSource(pipeline, imageSources, this);
    windowCalibration = new GuiCalibration(pipeline, calibration, this);
    windowStereoMethod = new GuiStereoMethod(pipeline, stereoMethods, this);

    // Create GUI
    createGui();
}

Toolbox::~Toolbox ()
{
}

void Toolbox::createGui ()
{
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    // Image source
    pushButtonImageSource = new QPushButton("Image source", this);
    connect(pushButtonImageSource, SIGNAL(released()), this, SLOT(showWindowImageSource()));
    
    pushButtonImageSourceActive = new QPushButton("Active", this);
    pushButtonImageSourceActive->setCheckable(true);

    pushButtonImageSourceActive->setChecked(pipeline->getImageSourceState());
    connect(pushButtonImageSourceActive, SIGNAL(toggled(bool)), pipeline, SLOT(setImageSourceState(bool)));
    connect(pipeline, SIGNAL(imageSourceStateChanged(bool)), this, SLOT(setPushButtonImageSourceActiveState(bool)));
    
    layout->addWidget(pushButtonImageSource, 0, 0);
    layout->addWidget(pushButtonImageSourceActive, 0, 1);

    // Calibration
    pushButtonCalibration = new QPushButton("Calibration", this);
    connect(pushButtonCalibration, SIGNAL(released()), this, SLOT(showWindowCalibration()));

    pushButtonCalibrationActive = new QPushButton("Active", this);
    pushButtonCalibrationActive->setCheckable(true);

    pushButtonCalibrationActive->setChecked(pipeline->getCalibrationState());
    connect(pushButtonCalibrationActive, SIGNAL(toggled(bool)), pipeline, SLOT(setCalibrationState(bool)));
    connect(pipeline, SIGNAL(calibrationStateChanged(bool)), this, SLOT(setPushButtonCalibrationActiveState(bool)));
        
    layout->addWidget(pushButtonCalibration, 1, 0);
    layout->addWidget(pushButtonCalibrationActive, 1, 1);

    // Stereo method
    pushButtonStereoMethod = new QPushButton("Stereo method", this);
    connect(pushButtonStereoMethod, SIGNAL(released()), this, SLOT(showWindowStereoMethod()));

    pushButtonStereoMethodActive = new QPushButton("Active", this);
    pushButtonStereoMethodActive->setCheckable(true);

    pushButtonStereoMethodActive->setChecked(pipeline->getStereoMethodState());
    connect(pushButtonStereoMethodActive, SIGNAL(toggled(bool)), pipeline, SLOT(setStereoMethodState(bool)));
    connect(pipeline, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(setPushButtonStereoMethodActiveState(bool)));
        
    layout->addWidget(pushButtonStereoMethod, 2, 0);
    layout->addWidget(pushButtonStereoMethodActive, 2, 1);

    // Separator
    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, 3, 0, 1, 2);

    // Exit
    QPushButton *pushButtonExit = new QPushButton("Exit", this);
    connect(pushButtonExit, SIGNAL(released()), qApp, SLOT(quit()));

    layout->addWidget(pushButtonExit, 4, 0, 1, 2);
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

void Toolbox::showWindowImageSource ()
{
    showWindowOnTop(windowImageSource);
}

void Toolbox::showWindowCalibration ()
{
    showWindowOnTop(windowCalibration);

}

void Toolbox::showWindowStereoMethod ()
{
    showWindowOnTop(windowStereoMethod);
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

void Toolbox::setPushButtonImageSourceActiveState (bool active)
{
    setActiveButtonState(pushButtonImageSourceActive, active);
}

void Toolbox::setPushButtonCalibrationActiveState (bool active)
{
    setActiveButtonState(pushButtonCalibrationActive, active);
}

void Toolbox::setPushButtonStereomethodActiveState (bool active)
{
    setActiveButtonState(pushButtonStereoMethodActive, active);
}


// *********************************************************************
// *                          Plugin loading                           *
// *********************************************************************
static void recursiveDirectoryScan (QDir dir, QStringList &files)
{
    // List all files in current directory
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (QString fileName, dir.entryList()) {
        files.append(dir.absoluteFilePath(fileName));
    }

    // List all directories and recursively scan them
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (QString dirName, dir.entryList()) {
        recursiveDirectoryScan(dir.absoluteFilePath(dirName), files);
    }
}


void Toolbox::loadSources (QDir &rootDir, QList<ImageSource *> &sources)
{
    QStringList files;

    // Recursively scan "sources" directory for plugin files
    recursiveDirectoryScan(rootDir.absoluteFilePath("sources"), files);
    
    foreach (QString fileName, files) {
        // Make sure it is a library
        if (!QLibrary::isLibrary(fileName)) {
            continue;
        }

        // Load plugin
        qDebug() << "Trying to load plugin:" << fileName;
        
        QPluginLoader loader(fileName);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

        QObject *plugin = loader.instance();
        if (plugin) {
            qDebug() << "Plugin successfully loaded!";
            ImageSource *source = qobject_cast<ImageSource *>(plugin);
            if (source) {
                sources.append(source);
            } else {
                qDebug() << "Failed to cast plugged object to ImageSource class!";
            }
        } else {
            qDebug() << "Failed to load plugin:" << loader.errorString();            
        }
    }
}

void Toolbox::loadMethods (QDir &rootDir, QList<StereoMethod *> &methods)
{
    QStringList files;

    // Recursively scan "methods" directory for plugin files
    recursiveDirectoryScan(rootDir.absoluteFilePath("methods"), files);
    
    foreach (QString fileName, files) {
        // Make sure it is a library
        if (!QLibrary::isLibrary(fileName)) {
            continue;
        }

        // Load plugin
        qDebug() << "Trying to load plugin:" << fileName;
        
        QPluginLoader loader(fileName);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
        
        QObject *plugin = loader.instance();
        if (plugin) {
            qDebug() << "Plugin successfully loaded!";
            StereoMethod *method = qobject_cast<StereoMethod *>(plugin);
            if (method) {
                methods.append(method);
            } else {
                qDebug() << "Failed to cast plugged object to StereoMethod class!";
            }
        } else {
            qDebug() << "Failed to load plugin:" << loader.errorString();            
        }
    }
}
