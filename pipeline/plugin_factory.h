/*
 * Stereo Pipeline: plugin factory interface
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

#ifndef PLUGIN_FACTORY_H
#define PLUGIN_FACTORY_H

#include "mvl_stereo_pipeline_export.h"

#include <QtPlugin>


class MVL_STEREO_PIPELINE_EXPORT PluginFactory
{
public:
    enum PluginType {
        PluginStereoMethod,
        PluginImagePairSource,
    };

    virtual PluginType getPluginType () const = 0;
    
    virtual QString getShortName () const = 0;
    virtual QString getDescription () const = 0;
    
    virtual QObject *createObject (QObject * = 0) const = 0;    
};

Q_DECLARE_INTERFACE(PluginFactory, "MVL_Stereo_Toolbox.PluginFactory/1.0")

#endif
