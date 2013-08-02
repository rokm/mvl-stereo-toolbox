/*
 * OpenCV Semi-Global Block Matching: plugin
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

using namespace StereoMethodSemiGlobalBlockMatching;


class Plugin : public QObject, PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(PluginFactory)
    
    PluginType getPluginType () const {
        return PluginStereoMethod;
    }
    
    QString getShortName () const {
        return "SGBM";
    }
    
    QString getDescription () const {
        return "OpenCV Semi-Global Block Matching";
    }
    
    QObject *createObject (QObject *parent = 0) const {
        return new Method(parent);
    }
};

Q_EXPORT_PLUGIN2(opencv_sgbm, Plugin)

// Because we have Q_OBJECT in source file
#include "plugin.moc"
