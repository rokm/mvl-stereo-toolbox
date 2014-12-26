/*
 * Stereo Pipeline: plugin manager
 * Copyright (C) 2014-2015 Rok Mandeljc
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

#include "plugin_manager.h"
#include "plugin_factory.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


PluginManager::PluginManager (QObject *parent)
    : QObject(parent)
{
    // Load plugins in default plugin path
    setPluginDirectory();
}

PluginManager::~PluginManager ()
{
}


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

void PluginManager::setPluginDirectory (const QString &newDirectory)
{
    // Clear old plugins
    foreach (QObject *plugin, plugins) {
        delete plugin;
    }
    plugins.clear();

    // If path is not provided, use default, which can be overriden
    // by environment variable
    if (!newDirectory.isEmpty()) {
        pluginDirectory = QDir(newDirectory);
    } else {
        QByteArray pluginEnvVariable = qgetenv ("MVL_STEREO_TOOLBOX_PLUGIN_DIR");
        if (!pluginEnvVariable.isEmpty()) {
            pluginDirectory = QDir(pluginEnvVariable);
        } else {
            pluginDirectory = QDir(MVL_STEREO_PIPELINE_PLUGIN_DIR);
        }
    }

    // Recursively scan for plugins
    QStringList files;
    recursiveDirectoryScan(pluginDirectory.absolutePath(), files);

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
            PluginFactory *factory = qobject_cast<PluginFactory *>(plugin);
            if (factory) {
                plugin->setParent(this);
                plugins.append(plugin);
            } else {
                qDebug() << "Failed to cast plugged object to PluginFactory!";
                delete plugin;
            }
        } else {
            qDebug() << "Failed to load plugin:" << loader.errorString();
        }
    }
}

QString PluginManager::getPluginDirectory () const
{
    return pluginDirectory.absolutePath();
}

const QList<QObject *> PluginManager::getAvailablePlugins () const
{
    return plugins;
}


} // Pipeline
} // StereoToolbox
} // MVL
