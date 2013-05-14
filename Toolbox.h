#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>

#include "ui_Toolbox.h"


// *********************************************************************
// *                              Toolbox                              *
// *********************************************************************
class Toolbox : public QMainWindow, private Ui::Toolbox
{
    Q_OBJECT

public:
    Toolbox (QWidget * = 0);
    virtual ~Toolbox ();

private:
};

#endif
