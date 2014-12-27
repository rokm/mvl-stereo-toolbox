/*
 * Stereo Pipeline: rectification
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__RECTIFICATION_H
#define MVL_STEREO_TOOLBOX__PIPELINE__RECTIFICATION_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class RectificationPrivate;

class MVL_STEREO_PIPELINE_EXPORT Rectification : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Rectification)
    Q_DECLARE_PRIVATE(Rectification)
    QScopedPointer<RectificationPrivate> const d_ptr;

public:
    Rectification (QObject * = 0);
    virtual ~Rectification ();

    void setStereoCalibration (const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Size &);
    void loadStereoCalibration (const QString &);
    void saveStereoCalibration (const QString &) const;
    void clearStereoCalibration ();

    // Static import/export functions
    static void exportStereoCalibration (const QString &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Size &);
    static void importStereoCalibration (const QString &, cv::Mat &, cv::Mat &, cv::Mat &, cv::Mat &, cv::Mat &, cv::Mat &, cv::Size &);

    void setPerformRectification (bool);
    bool getPerformRectification () const;

    const cv::Rect &getRoi () const;
    void setRoi (const cv::Rect &);

    void rectifyImagePair (const cv::Mat &, const cv::Mat &, cv::Mat &, cv::Mat &) const;

    bool getState () const;

    const cv::Size &getImageSize () const;
    const cv::Mat &getReprojectionMatrix () const;
    float getStereoBaseline () const;

protected:
    void initializeRectification ();

signals:
    void stateChanged (bool);

    void performRectificationChanged (bool);

    void error (QString) const;

    void roiChanged ();
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
