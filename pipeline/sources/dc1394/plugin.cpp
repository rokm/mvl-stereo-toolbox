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

#include <plugin_factory.h>
#include "source.h"

using namespace SourceDC1394;


class Plugin : public QObject, public PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(PluginFactory)
    
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
        return new Source(parent);
    }
};

Q_EXPORT_PLUGIN2(dc1394, Plugin)

// Because we have Q_OBJECT in source file
#include "plugin.moc"
