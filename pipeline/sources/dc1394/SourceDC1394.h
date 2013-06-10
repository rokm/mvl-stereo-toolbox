/*
 * DC1394 Image Pair Source: source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef IMAGE_SOURCE_DC1394_H
#define IMAGE_SOURCE_DC1394_H

#include "ImagePairSource.h"

#include <dc1394/dc1394.h>


class CameraDC1394;
class CameraListModel;

class SourceDC1394 : public ImagePairSource
{
    Q_OBJECT

public:
    SourceDC1394 (QObject * = 0);
    virtual ~SourceDC1394 ();

    virtual void stopSource ();

    virtual QWidget *createConfigWidget (QWidget * = 0);

    CameraListModel *getCameraListModel();

    void setLeftCamera (int);
    void setRightCamera (int);

    CameraDC1394 *getLeftCamera ();
    CameraDC1394 *getRightCamera ();

public slots:
    void scanBus ();
    void startStopCapture (bool);

protected:
    void createCamera (CameraDC1394 *&, int);
    void releaseCamera (CameraDC1394 *&);

protected slots:
    void frameAggregator ();

signals:
    void leftCameraChanged ();
    void rightCameraChanged ();

protected:
    dc1394_t *fw;
    
    CameraListModel *cameraListModel;

    CameraDC1394 *leftCamera;
    CameraDC1394 *rightCamera;

    bool leftFrameReady, rightFrameReady;
};

#endif
