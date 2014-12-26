/*
 * Video File Source: config widget
 * Copyright (C) 2014 Rok Mandeljc
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

#ifndef SOURCE_VIDEO_FILE_CONFIG_WIDGET_H
#define SOURCE_VIDEO_FILE_CONFIG_WIDGET_H

#include <QtWidgets>


namespace SourceVideoFile {

class Source;

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    SourceWidget (Source *, QWidget * = 0);
    virtual ~SourceWidget ();

protected:
    void browseForVideoFile ();
    void openVideoFile ();
    void videoFileReadyChanged (bool);

    void changePlaybackState (bool);
    void videoPositionChanged (int, int);

protected:
    Source *source;

    QLineEdit *lineEditVideoFile;
    QPushButton *pushButtonOpen;

    QLabel *labelVideoWidth;
    QLabel *labelVideoHeight;
    QLabel *labelVideoFps;
    QLabel *labelVideoLength;

    QPushButton *pushButtonPlayPause;
    QLabel *labelVideoPosition;
    QSlider *sliderPosition;
};

}

#endif
