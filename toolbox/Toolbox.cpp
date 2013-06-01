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
#include "StereoRectification.h"
#include "StereoMethod.h"
#include "StereoPipeline.h"

#include "GuiImageSource.h"
#include "GuiRectification.h"
#include "GuiStereoMethod.h"


Toolbox::Toolbox ()
    : QWidget()
{
    // Create pipeline
    pipeline = new StereoPipeline(this);
    pipeline->setUseStereoMethodThread(true);

    rectification = new StereoRectification(pipeline);
    pipeline->setRectification(rectification);

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
    windowRectification = new GuiRectification(pipeline, rectification, this);
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
    connect(pushButtonImageSource, SIGNAL(clicked()), this, SLOT(showWindowImageSource()));
    
    pushButtonImageSourceActive = new QPushButton("Active", this);
    pushButtonImageSourceActive->setCheckable(true);

    pushButtonImageSourceActive->setChecked(pipeline->getImageSourceState());
    connect(pushButtonImageSourceActive, SIGNAL(toggled(bool)), pipeline, SLOT(setImageSourceState(bool)));
    connect(pipeline, SIGNAL(imageSourceStateChanged(bool)), this, SLOT(setPushButtonImageSourceActiveState(bool)));
    
    layout->addWidget(pushButtonImageSource, 0, 0);
    layout->addWidget(pushButtonImageSourceActive, 0, 1);

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

    // Separator
    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, 3, 0, 1, 2);

    // Exit
    QPushButton *pushButtonExit = new QPushButton("Exit", this);
    connect(pushButtonExit, SIGNAL(clicked()), qApp, SLOT(quit()));

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

void Toolbox::showWindowRectification ()
{
    showWindowOnTop(windowRectification);

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

void Toolbox::setPushButtonRectificationActiveState (bool active)
{
    setActiveButtonState(pushButtonRectificationActive, active);
}

void Toolbox::setPushButtonStereoMethodActiveState (bool active)
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
        QPluginLoader loader(fileName);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

        QObject *plugin = loader.instance();
        if (plugin) {
            ImageSource *source = qobject_cast<ImageSource *>(plugin);
            if (source) {
                source->setParent(this);
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
        QPluginLoader loader(fileName);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
        
        QObject *plugin = loader.instance();
        if (plugin) {
            StereoMethod *method = qobject_cast<StereoMethod *>(plugin);
            if (method) {
                method->setParent(this);
                methods.append(method);
            } else {
                qDebug() << "Failed to cast plugged object to StereoMethod class!";
            }
        } else {
            qDebug() << "Failed to load plugin:" << loader.errorString();            
        }
    }
}
