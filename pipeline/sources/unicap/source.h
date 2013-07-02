/*
 * Unicap Image Pair Source: source
 * Copyright (C) 2013 Rok Mandeljc
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

#ifndef IMAGE_SOURCE_UNICAP_H
#define IMAGE_SOURCE_UNICAP_H

#include <image_pair_source.h>

#include <unicap.h>


namespace SourceUnicap {

class Camera;
class CameraList;

class Source : public ImagePairSource
{
    Q_OBJECT

public:
    Source (QObject * = 0);
    virtual ~Source ();

    virtual void stopSource ();

    virtual QWidget *createConfigWidget (QWidget * = 0);

    CameraList *getCameraList ();

    void setLeftCamera (int);
    void setRightCamera (int);

    Camera *getLeftCamera ();
    Camera *getRightCamera ();

public slots:
    void scanForDevices ();
    void startStopCapture (bool);

protected:
    void createCamera (Camera *&, int);
    void releaseCamera (Camera *&);

protected slots:
    void synchronizeFrames ();

signals:
    void leftCameraChanged ();
    void rightCameraChanged ();

protected:
    CameraList *cameraList;

    Camera *leftCamera;
    Camera *rightCamera;

    bool leftFrameReady, rightFrameReady;
};

}

#endif
