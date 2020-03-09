/*
 * OpenCV CUDA Belief Propagation: plugin
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
namespace StereoMethodOpenCvCudaBp {


class Plugin : public QObject, PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mvl-stereo-toolbox.Plugin.StereoMethod.OpenCV_CUDA_BP")
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::PluginFactory)

    PluginType getPluginType () const override {
        return PluginStereoMethod;
    }

    QString getShortName () const override {
        return "CUDA_BP";
    }

    QString getDescription () const override {
        return "OpenCV CUDA Belief Propagation";
    }

    QObject *createObject (QObject *parent = nullptr) const override {
        return new Method(parent);
    }
};

// Because we have Q_OBJECT in source file
#include "plugin.moc"


} // StereoMethodOpenCvCudaBp
} // Pipeline
} // StereoToolbox
} // MVL
