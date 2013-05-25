#ifndef CALIBRATION_PATTERN_DISPLAY_WIDGET_H
#define CALIBRATION_PATTERN_DISPLAY_WIDGET_H

#include "ImageDisplayWidget.h"


class CalibrationPatternDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT

public:
    CalibrationPatternDisplayWidget (const QString & = QString(), QWidget * = 0);
    virtual ~CalibrationPatternDisplayWidget ();

    void setImage (const cv::Mat &, bool, const std::vector<cv::Point2f> &, const cv::Size &);

protected:
    virtual void paintEvent (QPaintEvent *);

protected:
    bool patternFound;
    cv::Size patternSize;
    std::vector<cv::Point2f> patternPoints;
};

#endif
