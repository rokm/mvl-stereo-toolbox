/*
 * Stereo Pipeline: stereo method interface
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__STEREO_METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__STEREO_METHOD_H

#include <QtCore>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class StereoMethod
{
public:
    virtual QString getShortName () const = 0;

    // Config widget
    virtual QWidget *createConfigWidget (QWidget * = 0) = 0;

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &) = 0;

    // Parameter import/export
    virtual void loadParameters (const QString &) = 0;

    virtual void saveParameters (const QString &) const = 0;

    // These are actually signals, but they are not allowed in non-QObject classes
protected:
    virtual void parameterChanged () = 0;
};


} // Pipeline
} // StereoToolbox
} // MVL


Q_DECLARE_INTERFACE(MVL::StereoToolbox::Pipeline::StereoMethod, "MVL_Stereo_Toolbox.StereoMethod/1.0")


#endif
