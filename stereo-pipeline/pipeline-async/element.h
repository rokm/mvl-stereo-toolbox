#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__ELEMENT_H

#include <QtCore>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


class Element : public QObject
{
    Q_OBJECT

public:
    Element (const QString &name, QObject *parent = Q_NULLPTR);
    virtual ~Element ();

    void setState (bool active);
    bool getState () const;

protected:
    void incrementUpdateCount ();
    void estimateFps ();

signals:
    void error (const QString &message);
    void stateChanged (bool active);

protected:
    bool state;

    int updateCounter;
    float fps;
    QTime fpsTime;
    QTimer *fpsTimer;

    QThread *thread;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
