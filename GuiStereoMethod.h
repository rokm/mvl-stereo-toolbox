#ifndef GUI_STEREO_METHOD_H
#define GUI_STEREO_METHOD_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class StereoMethod;

class ImageDisplayWidget;


class GuiStereoMethod : public QWidget
{
    Q_OBJECT

public:
    GuiStereoMethod (StereoPipeline *, QList<StereoMethod *> &, QWidget * = 0);
    virtual ~GuiStereoMethod ();

protected slots:
    void setMethod (int);
    
    void updateImage ();

    void saveImage ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    QList<StereoMethod *> methods;

    // GUI
    QPushButton *pushButtonSaveImage;
    
    ImageDisplayWidget *displayDisparityImage;

    QStatusBar *statusBar;
};


#endif
