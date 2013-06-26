/*
 * DC1394 Image Pair Source: plugin
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
#include "SourceDC1394.h"

class Plugin : public PluginFactory
{
    PluginType getPluginType () const {
        return PluginImagePairSource;
    }
    
    QString getShortName () const {
        return "DC1394";
    }
    
    QString getDescription () const {
        return "DC1394 Image Pair Source";
    }
    
    QObject *createObject (QObject *parent = 0) const {
        return new SourceDC1394(parent);
    }
};

Q_EXPORT_PLUGIN2(dc1394, Plugin)
