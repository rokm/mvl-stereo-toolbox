/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef STEREO_METHOD
#define STEREO_METHOD

#include <QtCore>

#include <opencv2/core/core.hpp>


class StereoMethod : public QObject
{
    Q_OBJECT

public:
    StereoMethod (QObject * = 0);
    virtual ~StereoMethod ();

    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &) = 0;

signals:
    void parameterChanged ();
};

#endif
