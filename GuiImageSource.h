#ifndef GUI_IMAGE_SOURCE_H
#define GUI_IMAGE_SOURCE_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class ImageSource;

class GuiImageSource : public QWidget
{
    Q_OBJECT

public:
    GuiImageSource (StereoPipeline *, QList<ImageSource *> &, QWidget * = 0);
    virtual ~GuiImageSource ();

protected slots:
    void setSource (int);
    
    void updateImages ();

protected:
    // Pipeline
    StereoPipeline *pipeline;
    QList<ImageSource *> sources;

    // GUI
    QLabel *labelImageLeft;
    QLabel *labelImageRight;

    QStatusBar *statusBar;
};


#endif
