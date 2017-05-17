#include "element.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


Element::Element (const QString &name, QObject *parent)
    : QObject(parent), state(true), updateCounter(0), fps(0.0f)
{
    thread = new QThread(this);
    thread->setObjectName(name + "Thread");
    thread->start();

    fpsTimer = new QTimer(this);
    fpsTimer->setInterval(5*1000); // Estimate every five seconds
    connect(fpsTimer, &QTimer::timeout, this, &Element::estimateFps);
    fpsTimer->start();

    fpsTime.restart();
}

Element::~Element ()
{
    thread->quit();
    if (!thread->wait(15*1000)) {
        qWarning() << "Thread" << thread << "failed to finish!";
    }
}


void Element::setState (bool active)
{
    if (state != active) {
        state = active;

        // Reset stats
        updateCounter = 0;
        fps = 0.0f;
        fpsTime.restart();

        // Emit state change
        emit stateChanged(state);
    }
}

bool Element::getState () const
{
    return state;
}


// *********************************************************************
// *                    Update frequency statistics                    *
// *********************************************************************
void Element::incrementUpdateCount ()
{
    updateCounter++;
}


void Element::estimateFps ()
{
    // Estimate only if there were some updates; otherwise, leave
    // untouched and wait for next callback execution
    if (updateCounter && fpsTime.elapsed()) {
        // Estimate
        fps = updateCounter / (fpsTime.elapsed()/1000.0f);

        // Reset
        updateCounter = 0;
        fpsTime.restart();

        qInfo() << this << "fps:" << fps;
    } else {
        fps = 0.0f;
    }
}

int Element::getLastOperationTime () const
{
    QReadLocker locker(&lock);
    return lastOperationTime;
}

int Element::getNumberOfDroppedFrames () const
{
    QReadLocker locker(&lock);
    return droppedCounter;
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL
