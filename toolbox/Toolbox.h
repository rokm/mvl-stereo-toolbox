#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>

class ImageSource;
class StereoCalibration;
class StereoMethod;
class StereoPipeline;

class GuiImageSource;
class GuiCalibration;
class GuiStereoMethod;

class Toolbox : public QWidget
{
    Q_OBJECT

public:
    Toolbox ();
    virtual ~Toolbox ();

protected slots:
    void showWindowImageSource ();
    void showWindowCalibration ();
    void showWindowStereoMethod ();

    void setPushButtonImageSourceActiveState (bool);
    void setPushButtonCalibrationActiveState (bool);
    void setPushButtonStereomethodActiveState (bool);

protected:
    void createGui ();

    void loadSources (QDir &, QList<ImageSource *> &);
    void loadMethods (QDir &, QList<StereoMethod *> &);

protected:
    GuiImageSource *windowImageSource;
    GuiCalibration *windowCalibration;
    GuiStereoMethod *windowStereoMethod;

    QPushButton *pushButtonImageSource;
    QPushButton *pushButtonImageSourceActive;
    QPushButton *pushButtonCalibration;
    QPushButton *pushButtonCalibrationActive;
    QPushButton *pushButtonStereoMethod;
    QPushButton *pushButtonStereoMethodActive;
    
    StereoPipeline *pipeline;
    StereoCalibration *calibration;

    QList<ImageSource *> imageSources;
    QList<StereoMethod *> stereoMethods;
};


#endif
