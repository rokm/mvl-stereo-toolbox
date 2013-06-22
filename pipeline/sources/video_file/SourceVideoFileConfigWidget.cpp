/*
 * Video File Pair Source: config widget
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

#include "SourceVideoFileConfigWidget.h"
#include "SourceVideoFile.h"
#include "VideoFileCapture.h"


SourceVideoFileConfigWidget::SourceVideoFileConfigWidget (SourceVideoFile *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Video file source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load from files
    tooltip = "Load new pair of videos from harddisk.";
    
    button = new QPushButton("Load video pair", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadVideoPair()));
    pushButtonLoadPair = button;

    layout->addRow(button);

    // Periodic refresh
    tooltip = "Start/stop playback";
    
    button = new QPushButton("Play", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), source, SLOT(setPlaybackState(bool)));
    connect(source, SIGNAL(playbackStateChanged(bool)), button, SLOT(setChecked(bool)));
    pushButtonPlay = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Video sources
    QHBoxLayout *boxVideos = new QHBoxLayout();
    layout->addRow(boxVideos);

    boxVideos->addWidget(createVideoFrame(true)); // Left video frame
    boxVideos->addWidget(createVideoFrame(false)); // Right video frame
}

SourceVideoFileConfigWidget::~SourceVideoFileConfigWidget ()
{
}


QWidget *SourceVideoFileConfigWidget::createVideoFrame (bool left)
{
    QFrame *videoFrame;
    QLabel *label;
    QString tooltip;

    QFormLayout *layout;
   
    // Image frame
    videoFrame = new QFrame(this);
    videoFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(videoFrame);
    videoFrame->setLayout(layout);

    // Label
    label = new QLabel(left ? "<b>Left image</b>" : "<b>Right image</b>", videoFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Widget
    if (left) {
        layout->addRow(source->getLeftVideoCapture()->createConfigWidget(this));
    } else {
        layout->addRow(source->getRightVideoCapture()->createConfigWidget(this));
    }
    
    return videoFrame;
}


void SourceVideoFileConfigWidget::loadVideoPair ()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right video", QString(), "Videos (*.avi *.asf *.mp4 *.mkv *.mpg)");

    // Take first two images
    if (filenames.size() >= 2) {
        // Load video pair
        source->loadVideoPair(filenames[0], filenames[1]);
    }
}
