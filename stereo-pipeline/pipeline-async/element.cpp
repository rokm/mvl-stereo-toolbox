/*
 * Stereo Pipeline: asynchronous pipeline: base placeholder element
 * Copyright (C) 2017 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "element.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


Element::Element (const QString &name, QObject *parent)
    : QObject(parent), state(true), updateCounter(0), fps(0.0f),
      droppedCounter(0), lastOperationTime(0)
{
    thread = new QThread(this);
    thread->setObjectName(name + "Thread");
    thread->start();

    fpsTimer = new QTimer(this);
    fpsTimer->setInterval(5*1000); // Estimate every five seconds
    connect(fpsTimer, &QTimer::timeout, this, &Element::estimateFps);
    fpsTimer->start();

    fpsTime.restart();

    // Shut the element down on error
    connect(this, &Element::error, this, [this] () {
        setState(false);
    });
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

        emit frameRateReport(fps);

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
void Element::dropFrame ()
{
    lock.lockForWrite();
    droppedCounter++;
    lock.unlock();

    emit frameDropped(droppedCounter);
}

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

        // Emit report
        emit frameRateReport(fps);
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

float Element::getFramesPerSecond () const
{
    QReadLocker locker(&lock);
    return fps;
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL
