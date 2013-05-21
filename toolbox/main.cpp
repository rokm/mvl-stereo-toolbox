/*
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
 
#include "GuiImageSource.h"
#include "GuiCalibration.h"
#include "GuiStereoMethod.h"

#include "StereoPipeline.h"
#include "StereoCalibration.h"
#include "StereoMethod.h"
#include "ImageSource.h"


void recursiveDirectoryScan (QDir dir, QStringList &files)
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

void loadSources (QList<ImageSource *> &sources)
{
    QDir pluginsDir(qApp->applicationDirPath());
    QStringList files;

    // Recursively scan "sources" directory for plugin files
    recursiveDirectoryScan(pluginsDir.absoluteFilePath("sources"), files);
    
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

void loadMethods (QList<StereoMethod *> &methods)
{
    QDir pluginsDir(qApp->applicationDirPath());
    QStringList files;

    // Recursively scan "methods" directory for plugin files
    recursiveDirectoryScan(pluginsDir.absoluteFilePath("methods"), files);
    
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

int main (int argc, char **argv)
{
    QApplication app(argc, argv);
       
    qDebug() << "MVL StereoToolbox v.1.0, (C) 2013 Rok Mandeljc <rok.mandeljc@fe.uni-lj.si>";

    // *** Create pipeline ***
    StereoPipeline *pipeline = new StereoPipeline();

    // *** All available image sources ***
    QList<ImageSource *> sources;
    loadSources(sources);
    
    /*sources.append(new ImageSourceFile());
    sources.append(new ImageSourceDC1394());*/

    // *** Calibration ***
    StereoCalibration *calibration = new StereoCalibration();
    pipeline->setCalibration(calibration);

    // *** All available stereo methods ***
    QList<StereoMethod *> methods;
    loadMethods(methods);
    
    /*methods.append(new StereoMethodBlockMatching());
    methods.append(new StereoMethodSemiGlobalBlockMatching());
    methods.append(new StereoMethodVar());
    methods.append(new StereoMethodELAS());

    if (cv::gpu::getCudaEnabledDeviceCount()) {
        cv::gpu::setDevice(0);
        
        methods.append(new StereoMethodBlockMatchingGPU());
        methods.append(new StereoMethodBeliefPropagationGPU());
        methods.append(new StereoMethodConstantSpaceBeliefPropagationGPU());
    }*/

    // *** GUI ***
    GuiImageSource *guiImageSource = new GuiImageSource(pipeline, sources);
    guiImageSource->show();

    GuiCalibration *guiCalibration = new GuiCalibration(pipeline, calibration);
    guiCalibration->show();    

    GuiStereoMethod *guiStereoMethod = new GuiStereoMethod(pipeline, methods);
    guiStereoMethod->show();
    
    return app.exec();
}
