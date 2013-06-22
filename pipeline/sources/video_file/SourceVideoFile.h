/*
 * Image File Pair Source: source
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

#ifndef SOURCE_FILE_H
#define SOURCE_FILE_H

#include "ImagePairSource.h"


class VideoFileCapture;

class SourceVideoFile : public ImagePairSource
{
    Q_OBJECT

public:
    SourceVideoFile (QObject * = 0);
    virtual ~SourceVideoFile ();

    virtual void stopSource ();

    virtual QWidget *createConfigWidget (QWidget * = 0);

    VideoFileCapture *getLeftVideoCapture ();
    VideoFileCapture *getRightVideoCapture ();

    void loadVideoPair (const QString &, const QString &);

    bool getPlaybackState () const;
    
public slots:
    void setPlaybackState (bool);
    
protected slots:
    //void periodicRefresh ();
    void synchronizeFrames ();

signals:
    void playbackStateChanged (bool);
    
protected:
    VideoFileCapture *leftVideoCapture;
    VideoFileCapture *rightVideoCapture;

    bool leftFrameReady, rightFrameReady;
};

#endif
