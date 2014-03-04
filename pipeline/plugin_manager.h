/*
 * Stereo Pipeline: plugin manager
 * Copyright (C) 2014 Rok Mandeljc
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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "mvl_stereo_pipeline_export.h"

#include <QtCore>

class PluginFactory;

class MVL_STEREO_PIPELINE_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    PluginManager (QObject * = 0);
    virtual ~PluginManager ();

    void setPluginDirectory (const QString & = QString());
    QString getPluginDirectory () const;

    const QList<QObject *> getAvailablePlugins () const;

protected:
    QDir pluginDirectory;
    QList<QObject *> plugins;
};

#endif
