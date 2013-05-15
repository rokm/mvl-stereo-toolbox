#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>

#include "ui_Toolbox.h"


class StereoPipeline;
class StereoMethod;

// *********************************************************************
// *                              Toolbox                              *
// *********************************************************************
class Toolbox : public QMainWindow, private Ui::Toolbox
{
    Q_OBJECT

public:
    Toolbox (QWidget * = 0);
    virtual ~Toolbox ();

protected slots:
    void setStereoMethod (int);

    void updateInputImages ();
    void updateDepthImage ();

protected:
    StereoPipeline *pipeline;
    QList<StereoMethod *> methods;
};

#endif
