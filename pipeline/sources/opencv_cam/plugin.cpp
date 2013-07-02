/*
 * OpenCV Camera Image Pair Source: plugin
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
#include "source.h"

using namespace SourceOpenCvCam;


class Plugin : public PluginFactory
{
    PluginType getPluginType () const {
        return PluginImagePairSource;
    }
    
    QString getShortName () const {
        return "OpenCV Cam";
    }
    
    QString getDescription () const {
        return "OpenCV Camera Source";
    }
    
    QObject *createObject (QObject *parent = 0) const {
        return new Source(parent);
    }
};

Q_EXPORT_PLUGIN2(opencv_cam, Plugin)
