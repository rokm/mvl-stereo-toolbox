/*
 * Video File Source: source widget
 * Copyright (C) 2014-2015 Rok Mandeljc
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

#include "source_widget.h"
#include "source.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceVideoFile {


SourceWidget::SourceWidget (Source *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QPushButton *button;
    QLineEdit *lineEdit;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Video source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    baseLayout->addWidget(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    baseLayout->addWidget(line);

    // Scrollable area with layout
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(new QWidget(this));

    baseLayout->addWidget(scrollArea);

    QGridLayout *layout = new QGridLayout(scrollArea->widget());

    // Video file
    tooltip = "Video file path or URL.";

    label = new QLabel("Video file: ", this);
    label->setToolTip(tooltip);

    layout->addWidget(label, 0, 0, 1, 1);

    lineEdit = new QLineEdit(this);
    lineEditVideoFile = lineEdit;

    layout->addWidget(lineEdit, 0, 1, 1, 1);

    button = new QPushButton("Browse");
    connect(button, &QPushButton::clicked, this, &SourceWidget::browseForVideoFile);

    layout->addWidget(button, 0, 2, 1, 1);

    // Open
    tooltip = "Open video file.";

    button = new QPushButton("Open", this);
    button->setToolTip(tooltip);
    connect(button, &QPushButton::clicked, this, &SourceWidget::openVideoFile);
    pushButtonOpen = button;

    layout->addWidget(button, 1, 0, 1, 3);


    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, 2, 0, 1, 3);

    // Playback
    tooltip = "Start/pause playback.";

    button = new QPushButton("Play", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, &QPushButton::toggled, this, [this] (bool active) {
        if (active) {
            source->startPlayback();
        } else {
            source->stopPlayback();
        }
    });
    connect(source, &Source::playbackStateChanged, button, &QPushButton::setChecked);
    pushButtonPlayPause = button;

    layout->addWidget(button, 3, 0, 1, 3);

    // Position label
    labelVideoPosition = new QLabel(this);
    labelVideoPosition->setAlignment(Qt::AlignHCenter);
    layout->addWidget(labelVideoPosition, 4, 0, 1, 3);

    // Position slider
    sliderPosition = new QSlider(Qt::Horizontal, this);
    sliderPosition->setSingleStep(1);
    sliderPosition->setPageStep(100);
    layout->addWidget(sliderPosition, 5, 0, 1, 3);
    connect(sliderPosition, &QSlider::valueChanged, source, [this] (int value) {
        source->setVideoPosition(value - 1);
    });

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(line, 6, 0, 1, 3);

    // Video info
    label = new QLabel("<b>Video info<b>", this);
    layout->addWidget(label, 7, 0, 1, 3);

    labelVideoWidth = new QLabel(this);
    layout->addWidget(labelVideoWidth, 8, 0, 1, 3);

    labelVideoHeight = new QLabel(this);
    layout->addWidget(labelVideoHeight, 9, 0, 1, 3);

    labelVideoFps = new QLabel(this);
    layout->addWidget(labelVideoFps, 10, 0, 1, 3);

    labelVideoLength = new QLabel(this);
    layout->addWidget(labelVideoLength, 11, 0, 1, 3);

    // Spacer
    layout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 12, 0, 1, 3);

    // Init
    connect(source, &Source::videoFileReadyChanged, this, &SourceWidget::videoFileReadyChanged);
    connect(source, &Source::videoPositionChanged, this, &SourceWidget::videoPositionChanged);

    videoFileReadyChanged(false);
}

SourceWidget::~SourceWidget ()
{
}


// *********************************************************************
// *                             Video file                            *
// *********************************************************************
void SourceWidget::browseForVideoFile ()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select video file", QString(), "Video files (*.avi *.mp4 *.mkv *.mpeg *.mpg);; All files (*.*)");
    if (!filename.isEmpty()) {
        lineEditVideoFile->setText(filename);
    }
}


void SourceWidget::openVideoFile ()
{
    QString fileName = lineEditVideoFile->text();

    // If file name is empty, do the browse first
    if (fileName.isEmpty()) {
        browseForVideoFile();
        fileName = lineEditVideoFile->text();
    }

    // Open if file name is valid
    if (!fileName.isEmpty()) {
        source->openVideoFile(fileName);
    }
}

void SourceWidget::videoFileReadyChanged (bool available)
{
    int width = 0, height = 0, length = 0;
    float fps = 0.0f;

    if (available) {
        width = source->getVideoWidth();
        height = source->getVideoHeight();
        fps = source->getVideoFps();
        length = source->getVideoLength();

        labelVideoWidth->setText(QString("Width: %1").arg(width));
        labelVideoHeight->setText(QString("Height: %1").arg(height));
        labelVideoFps->setText(QString("FPS: %1").arg(fps));
        labelVideoLength->setText(QString("Length: %1").arg(length));

        labelVideoPosition->setText(QString("Position: %1/%2").arg(0).arg(length));
    } else {
        labelVideoWidth->setText("Width: N/A");
        labelVideoHeight->setText("Height: N/A");
        labelVideoFps->setText("FPS: N/A");
        labelVideoLength->setText("Length: N/A");

        labelVideoPosition->setText("Position: N/A");
    }

    pushButtonPlayPause->setEnabled(available);

    sliderPosition->setEnabled(available);
    sliderPosition->setRange(0, length);
}


// *********************************************************************
// *                              Playback                             *
// *********************************************************************
void SourceWidget::videoPositionChanged (int frame, int length)
{
    qint64 milliseconds = frame * 1000 / source->getVideoFps();
    int h = milliseconds / (1000 * 60 * 60);
    milliseconds -= h * (1000 * 60 * 60);
    int m = milliseconds / (1000 * 60);
    milliseconds -= m * (1000 * 60);
    int s = milliseconds / (1000);
    milliseconds -= s * (1000);

    labelVideoPosition->setText(QString("Position: %1/%2    %3:%4:%5.%6").arg(frame).arg(length).arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0')));

    sliderPosition->blockSignals(true);
    sliderPosition->setValue(frame);
    sliderPosition->blockSignals(false);
}


} // SourceVideoFile
} // Pipeline
} // StereoToolbox
} // MVL
