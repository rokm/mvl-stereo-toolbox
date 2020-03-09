/*
 * Stereo Pipeline: image pair source interface
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__IMAGE_PAIR_SOURCE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__IMAGE_PAIR_SOURCE_H

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class ImagePairSource
{
public:
    virtual QString getShortName () const = 0;

    virtual void getImages (cv::Mat &left, cv::Mat &right) const = 0;
    virtual void stopSource () = 0;

    // Config widget
    virtual QWidget *createConfigWidget (QWidget *parent = nullptr) = 0;

    // These are actually signals, but they are not allowed in non-QObject classes
public:
    virtual void imagesChanged () = 0;
    virtual void error (QString message) = 0;
};


} // Pipeline
} // StereoToolbox
} // MVL


Q_DECLARE_INTERFACE(MVL::StereoToolbox::Pipeline::ImagePairSource, "MVL_Stereo_Toolbox.ImagePairSource/1.0")


#endif
