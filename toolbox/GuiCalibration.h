#ifndef GUI_CALIBRATION_H
#define GUI_CALIBRATION_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class StereoCalibration;

class ImagePairDisplayWidget;

class BoardParametersDialog;


class GuiCalibration : public QWidget
{
    Q_OBJECT

public:
    GuiCalibration (StereoPipeline *, StereoCalibration *, QWidget * = 0);
    virtual ~GuiCalibration ();

protected slots:
    void doCalibration ();
    void importCalibration ();
    void exportCalibration ();
    void clearCalibration ();

    void saveImages ();

    void updateImage ();
    void updateState ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    StereoCalibration *calibration;

    // GUI
    QPushButton *pushButtonCalibrate;
    QPushButton *pushButtonImport;
    QPushButton *pushButtonExport;
    QPushButton *pushButtonClear;
    QPushButton *pushButtonSaveImages;
    
    ImagePairDisplayWidget *displayPair;

    QStatusBar *statusBar;

    BoardParametersDialog *patternDialog;
};

#endif
