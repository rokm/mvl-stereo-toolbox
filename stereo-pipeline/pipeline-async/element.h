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

    float getFramesPerSecond () const;
    int getLastOperationTime () const;
    int getNumberOfDroppedFrames () const;

protected:
    void incrementUpdateCount ();
    void dropFrame ();

    void estimateFps ();

signals:
    void error (const QString &message);
    void stateChanged (bool active);

    void frameDropped (int number);
    void frameRateReport (float fps);

protected:
    bool state;

    int updateCounter;
    float fps;
    QTime fpsTime;
    QTimer *fpsTimer;

    QThread *thread;

    // Cached data (lock reused by children)
    mutable QReadWriteLock lock;
    int droppedCounter;
    int lastOperationTime;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
