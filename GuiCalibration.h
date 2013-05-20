#ifndef GUI_CALIBRATION_H
#define GUI_CALIBRATION_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class StereoCalibration;

class ImageDisplayWidget;

class GuiCalibration : public QWidget
{
    Q_OBJECT

public:
    GuiCalibration (StereoPipeline *, StereoCalibration *, QWidget * = 0);
    virtual ~GuiCalibration ();

protected slots:   
    void updateImages ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    StereoCalibration *calibration;

    // GUI
    ImageDisplayWidget *displayRectified;

    QStatusBar *statusBar;
};

#endif
