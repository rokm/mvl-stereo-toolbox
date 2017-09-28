/*
 * OpenCV Block Matching: plugin
 * Copyright (C) 2013-2017 Rok Mandeljc
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
#include "method.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvBm {


class Plugin : public QObject, PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mvl-stereo-toolbox.Plugin.StereoMethod.OpenCV_BM")
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::PluginFactory)

    PluginType getPluginType () const override {
        return PluginStereoMethod;
    }

    QString getShortName () const override {
        return "BM";
    }

    QString getDescription () const override {
        return "OpenCV Block Matching";
    }

    QObject *createObject (QObject *parent = Q_NULLPTR) const override {
        return new Method(parent);
    }
};

// Because we have Q_OBJECT in source file
#include "plugin.moc"


} // StereoMethodOpenCvBm
} // Pipeline
} // StereoToolbox
} // MVL
