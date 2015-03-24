/*
 * Stereo Pipeline: reprojection
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__REPROJECTION_P_H
#define MVL_STEREO_TOOLBOX__PIPELINE__REPROJECTION_P_H


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class ReprojectionPrivate
{
    Q_DISABLE_COPY(ReprojectionPrivate)
    Q_DECLARE_PUBLIC(Reprojection)

    Reprojection * const q_ptr;

    ReprojectionPrivate (Reprojection *);

protected:
    cv::Mat Q;

    QList<int> supportedMethods;
    int reprojectionMethod;
};


ReprojectionPrivate::ReprojectionPrivate (Reprojection *reprojection)
    : q_ptr(reprojection)
{
    // Create list of supported methods
    supportedMethods.append(Reprojection::MethodToolboxCpu);
    supportedMethods.append(Reprojection::MethodOpenCvCpu);
#ifdef HAVE_OPENCV_CUDASTEREO
    try {
        if (cv::cuda::getCudaEnabledDeviceCount()) {
#ifdef HAVE_CUDA
            supportedMethods.append(Reprojection::MethodToolboxCuda);
#endif
            supportedMethods.append(Reprojection::MethodOpenCvCuda);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif

    // Default method: Toolbox CPU
    reprojectionMethod = Reprojection::MethodToolboxCpu;
}


} // Pipeline
} // StereoToolbox
} // MVL


#endif
