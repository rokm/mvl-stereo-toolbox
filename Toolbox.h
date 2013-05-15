#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>

#include "ui_Toolbox.h"


class StereoPipeline;

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
    void updateDepthImage ();

protected:
    StereoPipeline *pipeline;
};

#endif
