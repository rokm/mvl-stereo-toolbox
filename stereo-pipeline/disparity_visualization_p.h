/*
 * Stereo Pipeline: disparity visualization
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


class DisparityVisualizationPrivate
{
    Q_DISABLE_COPY(DisparityVisualizationPrivate)
    Q_DECLARE_PUBLIC(DisparityVisualization)

    DisparityVisualization * const q_ptr;

    DisparityVisualizationPrivate (DisparityVisualization *parent);

protected:
    QList<int> supportedMethods;
    int method;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
