/*
 * Stereo Pipeline: rectification
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
    Rectification (QObject *parent = Q_NULLPTR);
    virtual ~Rectification ();

    void setStereoCalibration (const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &imageSize, bool zeroDisparity, double alpha);
    void loadStereoCalibration (const QString &filename);
    void saveStereoCalibration (const QString &filename) const;
    void clearStereoCalibration ();

    // Static import/export functions
    static void exportStereoCalibration (const QString &filename, const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &imageSize, bool zeroDisparity, double alpha);
    static void importStereoCalibration (const QString &filename, cv::Mat &cameraMatrix1, cv::Mat &distCoeffs1, cv::Mat &cameraMatrix2, cv::Mat &distCoeffs2, cv::Mat &rotation, cv::Mat &translation, cv::Size &imageSize, bool &zeroDisparity, double &alpha);

    void setPerformRectification (bool enable);
    bool getPerformRectification () const;

    float getAlpha () const;
    void setAlpha (float alpha);

    bool getZeroDisparity () const;
    void setZeroDisparity (bool enable);

    void rectifyImagePair (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &img1r, cv::Mat &img2r) const;

    bool isCalibrationValid () const;

    // Individual calibration parameters
    const cv::Size &getImageSize () const;

    const cv::Mat &getCameraMatrix1 () const;
    const cv::Mat &getDistortionCoefficients1 () const;

    const cv::Mat &getCameraMatrix2 () const;
    const cv::Mat &getDistortionCoefficients2 () const;

    const cv::Mat &getRotationBetweenCameras () const;
    const cv::Mat &getTranslationBetweenCameras () const;

    const cv::Mat &getEssentialMatrix () const;
    const cv::Mat &getFundamentalMatrix () const;

    // Rectification parameters
    const cv::Mat &getRectificationTransformMatrix1 () const;
    const cv::Mat &getRectifiedCameraMatrix1 () const;

    const cv::Mat &getRectificationTransformMatrix2 () const;
    const cv::Mat &getRectifiedCameraMatrix2 () const;

    const cv::Mat &getReprojectionMatrix () const;

    float getStereoBaseline () const;

protected:
    void initializeRectification ();

signals:
    void calibrationChanged (bool valid);

    void performRectificationChanged (bool enable);

    void error (const QString &message) const;

    void zeroDisparityChanged ();
    void alphaChanged ();
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
