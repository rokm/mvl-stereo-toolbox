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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__PLUGIN_MANAGER_H
#define MVL_STEREO_TOOLBOX__PIPELINE__PLUGIN_MANAGER_H

#include <stereo-pipeline/export.h>

#include <QtCore>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class PluginManagerPrivate;

class MVL_STEREO_PIPELINE_EXPORT PluginManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginManager)
    Q_DECLARE_PRIVATE(PluginManager)
    QScopedPointer<PluginManagerPrivate> const d_ptr;

public:
    PluginManager (QObject *parent = nullptr);
    virtual ~PluginManager ();

    void setPluginDirectory (const QString &path = QString());
    QString getPluginDirectory () const;

    const QList<QObject *> getAvailablePlugins () const;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
