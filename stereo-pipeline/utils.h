/*
 * Stereo Pipeline: utility functions
 * Copyright (C) 2014-2017 Rok Mandeljc
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
MVL_STEREO_PIPELINE_EXPORT QString cvDepthToString (int depth);

// Functions for dumping/loading matrix to a binary file
MVL_STEREO_PIPELINE_EXPORT void writeMatrixToBinaryFile (const cv::Mat &matrix, const QString &fileName, bool compress = true);
MVL_STEREO_PIPELINE_EXPORT void readMatrixFromBinaryFile (cv::Mat &matrix, const QString &fileName);

// Additional visualization
MVL_STEREO_PIPELINE_EXPORT void createColorCodedDisparityCpu (const cv::Mat &disparity, cv::Mat &image, int numLevels);
MVL_STEREO_PIPELINE_EXPORT void createAnaglyph (const cv::Mat &left, const cv::Mat &right, cv::Mat &anaglyph);

// Point-cloud export to PCD file
MVL_STEREO_PIPELINE_EXPORT void writePointCloudToPcdFile (const cv::Mat &image, const cv::Mat &points, const QString &fileName, bool binary = true);


} // Utils
} // StereoToolbox
} // MVL


#endif
