/*
 * OpenCV GPU Dense Optical Flow - Dual TVL1: plugin
 * Copyright (C) 2013 Rok Mandeljc
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

#include <plugin_factory.h>
#include "method.h"

using namespace StereoMethodFlowDualTVL1Gpu;


class Plugin : public QObject, PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mvl-stereo-toolbox.Plugin.StereoMethod.OpenCV_Flow_DualTVL1_GPU")
    Q_INTERFACES(PluginFactory)
    
    PluginType getPluginType () const {
        return PluginStereoMethod;
    }
    
    QString getShortName () const {
        return "Flow_DualTVL1_GPU";
    }
    
    QString getDescription () const {
        return "OpenCV GPU Optical Flow - Dual TVL1";
    }
    
    QObject *createObject (QObject *parent = 0) const {
        return new Method(parent);
    }
};

// Because we have Q_OBJECT in source file
#include "plugin.moc"
