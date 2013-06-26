/*
 * OpenCV GPU Belief Propagation: plugin
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

#include "PluginFactory.h"
#include "StereoMethodBeliefPropagationGPU.h"


class Plugin : public PluginFactory
{
    PluginType getPluginType () const {
        return PluginStereoMethod;
    }
    
    QString getShortName () const {
        return "BP_GPU";
    }
    
    QString getDescription () const {
        return "OpenCV GPU Belief Propagation";
    }
    
    QObject *createObject (QObject *parent = 0) const {
        return new StereoMethodBeliefPropagationGPU(parent);
    }
};

Q_EXPORT_PLUGIN2(opencv_gpu_bp, Plugin)
