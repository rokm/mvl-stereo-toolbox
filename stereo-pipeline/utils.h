/*
 * Stereo Pipeline: utility functions
 * Copyright (C) 2014-2015 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__UTILS_H
#define MVL_STEREO_TOOLBOX__PIPELINE__UTILS_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Utils {

// Helpers
MVL_STEREO_PIPELINE_EXPORT QString cvDepthToString (int);

// Functions for dumping/loading matrix to a binary file
MVL_STEREO_PIPELINE_EXPORT void writeMatrixToBinaryFile (const cv::Mat &, const QString &, bool = true);

// Additional visualization
MVL_STEREO_PIPELINE_EXPORT void createColorCodedDisparityCpu (const cv::Mat &, cv::Mat &, int);
MVL_STEREO_PIPELINE_EXPORT void createAnaglyph (const cv::Mat &, const cv::Mat &, cv::Mat &);


} // Utils
} // StereoToolbox
} // MVL


#endif
