/*
 * Stereo Pipeline: plugin manager
 * Copyright (C) 2014-2017 Rok Mandeljc
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


#include "plugin_manager_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


PluginManagerPrivate::PluginManagerPrivate (PluginManager *parent)
    : q_ptr(parent)
{
}



PluginManager::PluginManager (QObject *parent)
    : QObject(parent), d_ptr(new PluginManagerPrivate(this))
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
    for (const QString &fileName : dir.entryList()) {
        files.append(dir.absoluteFilePath(fileName));
    }

    // List all directories and recursively scan them
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QString &dirName : dir.entryList()) {
        recursiveDirectoryScan(dir.absoluteFilePath(dirName), files);
    }
}

void PluginManager::setPluginDirectory (const QString &path)
{
    Q_D(PluginManager);

    // Clear old plugins
    for (QObject *plugin : d->plugins) {
        delete plugin;
    }
    d->plugins.clear();

    // If path is not provided, use default, which can be overriden
    // by environment variable
    if (!path.isEmpty()) {
        d->pluginDirectory = QDir(path);
    } else {
        QByteArray pluginEnvVariable = qgetenv ("MVL_STEREO_TOOLBOX_PLUGIN_DIR");
        if (!pluginEnvVariable.isEmpty()) {
            d->pluginDirectory = QDir(pluginEnvVariable);
        } else {
            d->pluginDirectory = QDir(MVL_STEREO_PIPELINE_PLUGIN_DIR);
        }
    }

    // Recursively scan for plugins
    QStringList files;
    recursiveDirectoryScan(d->pluginDirectory.absolutePath(), files);

    for (const QString &fileName : files) {
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
                d->plugins.append(plugin);
            } else {
                qWarning() << "Failed to cast plugged object to PluginFactory!";
                delete plugin;
            }
        } else {
            qWarning() << "Failed to load plugin:" << loader.errorString();
        }
    }
}

QString PluginManager::getPluginDirectory () const
{
    Q_D(const PluginManager);
    return d->pluginDirectory.absolutePath();
}

const QList<QObject *> PluginManager::getAvailablePlugins () const
{
    Q_D(const PluginManager);
    return d->plugins;
}


} // Pipeline
} // StereoToolbox
} // MVL
