/*
 * Stereo Pipeline: plugin interface
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef PLUGIN_FACTORY_H
#define PLUGIN_FACTORY_H

#include <QtPlugin>


class PluginFactory : public QObject
{
    Q_OBJECT

public:
    enum PluginType {
        PluginStereoMethod,
        PluginImageSource,
    };

    virtual PluginType getPluginType () const = 0;
    
    virtual QString getShortName () const = 0;
    virtual QString getDescription () const = 0;
    
    virtual QObject *createObject (QObject * = 0) const = 0;    
};

#endif
