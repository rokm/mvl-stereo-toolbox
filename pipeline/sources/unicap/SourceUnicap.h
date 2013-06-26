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

#ifndef IMAGE_SOURCE_Unicap_H
#define IMAGE_SOURCE_Unicap_H

#include "ImagePairSource.h"

#include <unicap.h>


class CameraUnicap;
class CameraListModel;

class SourceUnicap : public ImagePairSource
{
    Q_OBJECT

public:
    SourceUnicap (QObject * = 0);
    virtual ~SourceUnicap ();

    virtual void stopSource ();

    virtual QWidget *createConfigWidget (QWidget * = 0);

    CameraListModel *getCameraListModel();

    void setLeftCamera (int);
    void setRightCamera (int);

    CameraUnicap *getLeftCamera ();
    CameraUnicap *getRightCamera ();

public slots:
    void scanForDevices ();
    void startStopCapture (bool);

protected:
    void createCamera (CameraUnicap *&, int);
    void releaseCamera (CameraUnicap *&);

protected slots:
    void synchronizeFrames ();

signals:
    void leftCameraChanged ();
    void rightCameraChanged ();

protected:
    CameraListModel *cameraListModel;

    CameraUnicap *leftCamera;
    CameraUnicap *rightCamera;

    bool leftFrameReady, rightFrameReady;
};

#endif
