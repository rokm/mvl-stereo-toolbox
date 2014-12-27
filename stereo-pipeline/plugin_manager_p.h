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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__PLUGIN_MANAGER_P_H
#define MVL_STEREO_TOOLBOX__PIPELINE__PLUGIN_MANAGER_P_H


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class PluginManagerPrivate
{
    Q_DISABLE_COPY(PluginManagerPrivate)
    Q_DECLARE_PUBLIC(PluginManager)

    PluginManager * const q_ptr;

    PluginManagerPrivate (PluginManager *);

protected:
    QDir pluginDirectory;
    QList<QObject *> plugins;
};


PluginManagerPrivate::PluginManagerPrivate (PluginManager *manager)
    : q_ptr(manager)
{
}


} // Pipeline
} // StereoToolbox
} // MVL


#endif
