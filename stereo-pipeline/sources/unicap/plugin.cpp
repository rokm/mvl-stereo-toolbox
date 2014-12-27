/*
 * Unicap Source: plugin
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include <stereo-pipeline/plugin_factory.h>
#include "source.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


class Plugin : public QObject, PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mvl-stereo-toolbox.Plugin.Source.Unicap")
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::PluginFactory)

    PluginType getPluginType () const {
        return PluginImagePairSource;
    }

    QString getShortName () const {
        return "Unicap";
    }

    QString getDescription () const {
        return "Unicap Image Pair Source";
    }

    QObject *createObject (QObject *parent = 0) const {
        return new Source(parent);
    }
};

// Because we have Q_OBJECT in source file
#include "plugin.moc"


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL
