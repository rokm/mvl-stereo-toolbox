/*
 * Stereo Pipeline: asynchronous pipeline: stereo method placeholder element
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__METHOD_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__METHOD_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class StereoMethod;

namespace AsyncPipeline {


class MethodElement : public Element
{
    Q_OBJECT

public:
    MethodElement (QObject *parent = Q_NULLPTR);
    virtual ~MethodElement ();

    void setStereoMethod (QObject *method);
    QObject *getStereoMethod ();

    void loadParameters (const QString &filename);
    void saveParameters (const QString &filename) const;

    void computeDisparity (const cv::Mat &imageL, const cv::Mat &imageR);

    cv::Mat getDisparity () const;
    void getDisparity (cv::Mat &disparity, int &numDisparityLevels) const;

signals:
    void eject ();
    void methodChanged ();
    void parameterChanged ();

    void disparityComputationRequest (const cv::Mat imageL, const cv::Mat imageR);
    void disparityChanged ();

protected:
    // Stereo method object
    QObject *methodObject;
    QObject *methodParent;
    StereoMethod *methodIface;

    QList<QMetaObject::Connection> signalConnections;

    mutable QMutex mutex; // Method mutex

    // Cached disparity (under parent's lock!)
    cv::Mat disparity;
    int numDisparityLevels;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat disparity;
        int numDisparityLevels;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
