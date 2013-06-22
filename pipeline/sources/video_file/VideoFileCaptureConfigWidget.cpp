#include "VideoFileCaptureConfigWidget.h"
#include "VideoFileCapture.h"


VideoFileCaptureConfigWidget::VideoFileCaptureConfigWidget (VideoFileCapture *c, QWidget *parent)
    : QWidget(parent), capture(c)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load from files
    QHBoxLayout *buttonBox = new QHBoxLayout();
    layout->addRow(buttonBox);
    
    tooltip = "Load from harddisk.";
    
    button = new QPushButton("File", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadFile()));
    pushButtonFile = button;

    buttonBox->addWidget(button);

    // Load from URLs
    tooltip = "Load from URL.";
    
    button = new QPushButton("URL", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadUrl()));
    pushButtonUrl = button;

    buttonBox->addWidget(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);
    
    // Image info: filename
    tooltip = "Video file name or URL.";

    label = new QLabel("<b>File/URL:</b>", this);
    label->setToolTip(tooltip);

    textEditFilename = new QTextEdit(this);
    textEditFilename->setReadOnly(true);

    layout->addRow(label, textEditFilename);

    // Image info: resolution
    tooltip = "Video resolution.";

    label = new QLabel("<b>Resolution:</b>", this);
    label->setToolTip(tooltip);

    labelResolution = new QLabel(this);
    labelResolution->setWordWrap(true);

    layout->addRow(label, labelResolution);

    // URL dialog
    dialogUrl = new UrlDialog(this);

    // Signals
    connect(capture, SIGNAL(videoLoaded()), this, SLOT(updateVideoInformation()));
}

VideoFileCaptureConfigWidget::~VideoFileCaptureConfigWidget ()
{
}

void VideoFileCaptureConfigWidget::updateVideoInformation ()
{
    // Display image information
    textEditFilename->setText(QString("%1").arg(capture->getVideoFilename()));
    labelResolution->setText(QString("%1x%2 @ %3 FPS").arg(capture->getFrameWidth()).arg(capture->getFrameHeight()).arg(capture->getFPS()));
}

void VideoFileCaptureConfigWidget::loadFile ()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load video", QString(), "Videos (*.avi *.asf *.mp4 *.mkv *.mpg)");
    if (!filename.isEmpty()) {
        capture->setVideoFile(filename);
    }
}


void VideoFileCaptureConfigWidget::loadUrl ()
{
    // Run the dialog
    if (dialogUrl->exec() == QDialog::Accepted) {
        // Set URL
        capture->setVideoFile(dialogUrl->getUrl());
    }
}



// *********************************************************************
// *                            URL dialog                             *
// *********************************************************************
UrlDialog::UrlDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    textEditUrl = new QTextEdit(this);
    layout->addRow("Video URL", textEditUrl);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

UrlDialog::~UrlDialog ()
{
}

QString UrlDialog::getUrl() const
{
    return textEditUrl->toPlainText();
}
