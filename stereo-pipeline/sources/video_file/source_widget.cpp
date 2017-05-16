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
    QHBoxLayout *hbox;
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

    QVBoxLayout *layout = new QVBoxLayout(scrollArea->widget());

    // Video file
    hbox = new QHBoxLayout();
    hbox->setContentsMargins(0, 0, 0, 0);

    tooltip = "Video file path or URL.";

    label = new QLabel("Video file: ", this);
    label->setToolTip(tooltip);

    hbox->addWidget(label);

    lineEdit = new QLineEdit(this);
    lineEditVideoFile = lineEdit;

    connect(lineEditVideoFile, &QLineEdit::returnPressed, this, [this] () {
        if (lineEditVideoFile->text() != videoFilename) {
            videoFilename = lineEditVideoFile->text();
            emit videoFileLoadRequested(videoFilename);
        }
    });
    connect(this, &SourceWidget::videoFileLoadRequested, source, &Source::openVideoFile, Qt::QueuedConnection); // A two-piece connection due to different thread affinity

    hbox->addWidget(lineEdit);

    button = new QPushButton("Browse");
    connect(button, &QPushButton::clicked, this, [this] () {
        QString filename = QFileDialog::getOpenFileName(this, "Select video file", QString(), "Video files (*.avi *.mp4 *.mkv *.mpeg *.mpg);; All files (*.*)");
        if (!filename.isEmpty()) {
            videoFilename = filename;
            lineEditVideoFile->setText(videoFilename);
            emit videoFileLoadRequested(videoFilename); // Use same type of connection as above
        }
    });

    hbox->addWidget(button);

    layout->addLayout(hbox);

     // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    // Video playback/info widget and layout
    widgetVideo = new QWidget();
    layout->addWidget(widgetVideo);

    layout->addWidget(line);

    // Spacer
    layout->addStretch();


    // *** Setup the playback/info widget and layout ***
    QVBoxLayout *layoutVideo = new QVBoxLayout(widgetVideo);
    layoutVideo->setContentsMargins(0, 0, 0, 0);

    // Playback
    tooltip = "Start/pause playback.";

    button = new QPushButton("Play", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, &QPushButton::toggled, source, [this] (bool active) {
        if (active) {
            source->startPlayback();
        } else {
            source->stopPlayback();
        }
    }, Qt::QueuedConnection);
    connect(source, &Source::playbackStateChanged, button, &QPushButton::setChecked);
    pushButtonPlayPause = button;

    layoutVideo->addWidget(button);

    // Position
    hbox = new QHBoxLayout();

    hbox->addStretch(5);

    label = new QLabel("<b>Frame: </b>", this);
    hbox->addWidget(label);

    spinBoxFrame = new QSpinBox(this);
    connect(spinBoxFrame, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), source, [this] (int value) {
        source->setVideoPosition(value - 1);
    }, Qt::QueuedConnection);
    hbox->addWidget(spinBoxFrame);

    hbox->addStretch(1);

    label = new QLabel("<b>Time: </b>", this);
    hbox->addWidget(label);

    timeEditPosition = new QTimeEdit(this);
    timeEditPosition->setDisplayFormat("hh:mm:ss.zzz");
    timeEditPosition->setEnabled(false); // We do not support time-based seek yet
    hbox->addWidget(timeEditPosition);

    hbox->addStretch(5);

    layoutVideo->addLayout(hbox);

    // Position slider
    sliderPosition = new QSlider(Qt::Horizontal, this);
    sliderPosition->setSingleStep(1);
    sliderPosition->setPageStep(100);
    sliderPosition->setTracking(false);
    layoutVideo->addWidget(sliderPosition);
    connect(sliderPosition, &QSlider::valueChanged, source, [this] (int value) {
        source->setVideoPosition(value - 1);
    }, Qt::QueuedConnection);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layoutVideo->addWidget(line);

    // Video info
    label = new QLabel("<b>Video information:</b>", this);
    layoutVideo->addWidget(label);

    labelVideoResolution = new QLabel(this);
    layoutVideo->addWidget(labelVideoResolution);

    labelVideoFramerate = new QLabel(this);
    layoutVideo->addWidget(labelVideoFramerate);

    labelVideoLength = new QLabel(this);
    layoutVideo->addWidget(labelVideoLength);

    // Init
    connect(source, &Source::videoFileChanged, this, &SourceWidget::updateVideoInfo, Qt::QueuedConnection);
    connect(source, &Source::videoPositionChanged, this, &SourceWidget::updateVideoPosition, Qt::QueuedConnection);

    updateVideoInfo(false);
}

SourceWidget::~SourceWidget ()
{
}


// *********************************************************************
// *                             Video file                            *
// *********************************************************************
void SourceWidget::updateVideoInfo (bool available)
{
    int width = 0, height = 0, length = 0;
    float framerate = 0.0f;

    if (available) {
        widgetVideo->show();

        width = source->getVideoWidth();
        height = source->getVideoHeight();
        framerate = source->getVideoFramerate();
        length = source->getVideoLength();

        labelVideoResolution->setText(QString("<b>Resolution:</b> %1x%2").arg(width).arg(height));
        labelVideoFramerate->setText(QString("<b>Framerate:</b> %1").arg(framerate));
        labelVideoLength->setText(QString("<b>Length:</b> %1").arg(length));
    } else {
        widgetVideo->hide();

        labelVideoResolution->setText("<b>Resolution:</b> N/A");
        labelVideoFramerate->setText("<b>Framerate:</b> N/A");
        labelVideoLength->setText("<b>Length:</b> N/A");
    }

    pushButtonPlayPause->setEnabled(available);

    sliderPosition->setEnabled(available);
    sliderPosition->setRange(0, length);

    spinBoxFrame->setRange(0, length);
    spinBoxFrame->setSuffix(QString(" / %1").arg(length));
}


// *********************************************************************
// *                              Playback                             *
// *********************************************************************
void SourceWidget::updateVideoPosition (int frame, int length)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(frame * 1000 / source->getVideoFramerate());

    timeEditPosition->blockSignals(true);
    timeEditPosition->setTime(time);
    timeEditPosition->blockSignals(false);

    spinBoxFrame->blockSignals(true);
    spinBoxFrame->setValue(frame);
    spinBoxFrame->blockSignals(false);

    sliderPosition->blockSignals(true);
    sliderPosition->setValue(frame);
    sliderPosition->blockSignals(false);
}


} // SourceVideoFile
} // Pipeline
} // StereoToolbox
} // MVL
