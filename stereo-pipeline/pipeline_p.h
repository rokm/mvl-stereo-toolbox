/*
 * Stereo Pipeline: pipeline
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_P_H
#define MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_P_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <QtConcurrent>
#include <opencv2/core.hpp>


Q_DECLARE_METATYPE(cv::Mat);


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

namespace AsyncPipeline {
    class SourceElement;
    class RectificationElement;
    class MethodElement;
    class ReprojectionElement;
    class VisualizationElement;
}


class PipelinePrivate
{
    Q_DISABLE_COPY(PipelinePrivate)
    Q_DECLARE_PUBLIC(Pipeline)

    Pipeline * const q_ptr;

    PipelinePrivate (Pipeline *parent);

protected:
    AsyncPipeline::SourceElement *source;
    AsyncPipeline::RectificationElement *rectification;
    AsyncPipeline::MethodElement *stereoMethod;
    AsyncPipeline::ReprojectionElement *reprojection;
    AsyncPipeline::VisualizationElement *visualization;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
