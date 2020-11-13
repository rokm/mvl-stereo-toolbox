/*
 * Stereo Pipeline: asynchronous pipeline: image-pair source placeholder element
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__SOURCE_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__SOURCE_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class ImagePairSource;

namespace AsyncPipeline {


class SourceElement : public Element
{
    Q_OBJECT

public:
    SourceElement (QObject *parent = nullptr);
    virtual ~SourceElement ();

    void setImagePairSource (QObject *source);
    QObject *getImagePairSource ();

    cv::Mat getLeftImage () const;
    cv::Mat getRightImage () const;

    void getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const;

    void setFramerateLimit (double limit);
    double getFramerateLimit () const;

protected slots:
    void handleImagesChange (); // Must be slot due to old-syntax!

signals:
    void eject ();
    void sourceChanged ();

    void imagesChanged ();

    void framerateLimitChanged (double limit);

protected:
    // Image pair source object
    QObject *sourceObject;
    QObject *sourceParent;
    ImagePairSource *sourceIface;

    QList<QMetaObject::Connection> signalConnections;

    QElapsedTimer timeLastUpdate;
    double framerateLimit;

    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat imageL;
    cv::Mat imageR;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
