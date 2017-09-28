/*
 * Stereo Pipeline: asynchronous pipeline: rectification placeholder element
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__RECTIFICATION_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__RECTIFICATION_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class Rectification;

namespace AsyncPipeline {


class RectificationElement : public Element
{
    Q_OBJECT

public:
    RectificationElement (QObject *parent = Q_NULLPTR);
    virtual ~RectificationElement ();

    Rectification *getRectification ();

    void rectifyImages (const cv::Mat &imageL, const cv::Mat &imageR);

    cv::Mat getLeftImage () const;
    cv::Mat getRightImage () const;

    void getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const;

    cv::Mat getReprojectionMatrix () const;

signals:
    void eject ();
    void imageRectificationRequest (const cv::Mat imageL, const cv::Mat imageR);

    void imagesChanged ();

    void calibrationChanged (bool valid);
    void performRectificationChanged (bool enabled);

protected:
    // Rectification object
    Rectification *rectification;

    mutable QMutex mutex; // Method mutex


    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat imageL;
    cv::Mat imageR;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat imageL;
        cv::Mat imageR;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
