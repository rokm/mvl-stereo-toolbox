#ifndef STEREO_CALIBRATION
#define STEREO_CALIBRATION

#include <QtCore>

#include <opencv2/core/core.hpp>


class StereoCalibration : public QObject
{
    Q_OBJECT

public:
    StereoCalibration (QObject * = 0);
    virtual ~StereoCalibration ();

    void loadCalibration (const QString &);
    void saveCalibration (const QString &) const;

    void calibrateFromImages (const QStringList &, int, int, float);

    void rectifyImagePair (const cv::Mat &, const cv::Mat &, cv::Mat &, cv::Mat &) const;

protected:
    void initializeRectification ();

signals:
    void stateChanged (bool);

protected:
    bool isValid;

    // Raw calibration parameters
    cv::Size imageSize;
        
    cv::Mat M1, M2; // Camera matrices
    cv::Mat D1, D2; // Distortion coefficients

    cv::Mat R, T, E, F;

    // Rectification parameters
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;
    cv::Rect validRoi1, validRoi2;

    bool isVerticalStereo;

    // Rectification maps
    cv::Mat map11, map12, map21, map22;
};


#endif
