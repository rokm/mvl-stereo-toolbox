#include <QtCore>
#include <QtGui>

int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    qDebug() << "MVL StereoToolbox v.1.0";
    qDebug() << "(C) 2013 Rok Mandeljc <rok.mandeljc@fe.uni-lj.si>";
    
    return app.exec();
}
