/*
 * Stereo Pipeline: asynchronous pipeline: visualization placeholder element
 * Copyright (C) 2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__VISUALIZATION_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__VISUALIZATION_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class DisparityVisualization;

namespace AsyncPipeline {


class VisualizationElement : public Element
{
    Q_OBJECT

public:
    VisualizationElement (QObject *parent = nullptr);
    virtual ~VisualizationElement ();

    DisparityVisualization *getVisualization ();

    void visualizeDisparity (const cv::Mat &disparity, int numDisparityLevels);

    cv::Mat getImage () const;
    void getImage (cv::Mat &image) const;

signals:
    void eject ();
    void disparityVisualizationRequest (const cv::Mat disparity, int numDisparityLevels);

    void visualizationMethodChanged ();
    void imageChanged ();

protected:
    // Visualization object
    DisparityVisualization *visualization;

    mutable QMutex mutex; // Method mutex


    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat image;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat image;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
