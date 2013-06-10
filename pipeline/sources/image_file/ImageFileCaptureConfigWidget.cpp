#include "ImageFileCaptureConfigWidget.h"
#include "ImageFileCapture.h"


ImageFileCaptureConfigWidget::ImageFileCaptureConfigWidget (ImageFileCapture *c, QWidget *parent)
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
    
    tooltip = "Load new pair of images from harddisk.";
    
    button = new QPushButton("File", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadFile()));
    pushButtonFile = button;

    buttonBox->addWidget(button);

    // Load from URLs
    tooltip = "Load new pair of images from URLs.";
    
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
    tooltip = "Left image file name or URL.";

    label = new QLabel("<b>File/URL:</b>", this);
    label->setToolTip(tooltip);

    textEditFilename = new QTextEdit(this);
    textEditFilename->setReadOnly(true);

    layout->addRow(label, textEditFilename);

    // Image info: resolution
    tooltip = "Image resolution.";

    label = new QLabel("<b>Resolution:</b>", this);
    label->setToolTip(tooltip);

    labelResolution = new QLabel(this);
    labelResolution->setWordWrap(true);

    layout->addRow(label, labelResolution);

    // Image info: channels
    tooltip = "Number of channels.";

    label = new QLabel("<b>Channels:</b>", this);
    label->setToolTip(tooltip);

    labelChannels = new QLabel(this);
    labelChannels->setWordWrap(true);

    layout->addRow(label, labelChannels);

    // URL dialog
    dialogUrl = new UrlDialog(this);

    // Signals
    connect(capture, SIGNAL(imageReady()), this, SLOT(updateImageInformation()));
}

ImageFileCaptureConfigWidget::~ImageFileCaptureConfigWidget ()
{
}

void ImageFileCaptureConfigWidget::updateImageInformation ()
{
    // Display image information
    textEditFilename->setText(QString("%1").arg(capture->getImageFilename()));
    labelResolution->setText(QString("%1x%2").arg(capture->getImageWidth()).arg(capture->getImageHeight()));
    labelChannels->setText(QString("%1").arg(capture->getImageChannels()));
}

void ImageFileCaptureConfigWidget::loadFile ()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");
    if (!filename.isEmpty()) {
        capture->setImageFileOrUrl(filename, false);
    }
}


void ImageFileCaptureConfigWidget::loadUrl ()
{
    // Run the dialog
    if (dialogUrl->exec() == QDialog::Accepted) {
        // Set URL
        capture->setImageFileOrUrl(dialogUrl->getUrl(), true);
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
    layout->addRow("Image URL", textEditUrl);

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
