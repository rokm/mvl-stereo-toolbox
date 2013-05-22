#include "GuiCalibration.h"

#include "ImagePairDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoCalibration.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


class BoardParametersDialog : public QDialog
{
public:
    BoardParametersDialog (QWidget *parent = 0)
        : QDialog(parent)
    {
        QFormLayout *layout = new QFormLayout(this);
        setLayout(layout);

        setWindowTitle("Calibration pattern");

        QLabel *label;
        QFrame *separator;
        QDialogButtonBox *buttonBox;

        // Board width
        label = new QLabel("Board width", this);
        label->setToolTip("Checkboard width (number of inside corners in horizontal direction)");

        spinBoxBoardWidth = new QSpinBox(this);
        spinBoxBoardWidth->setRange(1, 1000);
        spinBoxBoardWidth->setValue(19);
        layout->addRow(label, spinBoxBoardWidth);

        // Board height
        label = new QLabel("Board height", this);
        label->setToolTip("Checkboard height (number of inside corners in horizontal direction)");

        spinBoxBoardHeight = new QSpinBox(this);
        spinBoxBoardHeight->setRange(1, 1000);
        spinBoxBoardHeight->setValue(12);
        layout->addRow(label, spinBoxBoardHeight);

        // Square size
        label = new QLabel("Square size", this);
        label->setToolTip("Size of checkboard square");

        spinBoxSquareSize = new QDoubleSpinBox(this);
        spinBoxSquareSize->setRange(1.0, 1000.0);
        spinBoxSquareSize->setValue(25.0);
        spinBoxSquareSize->setSuffix(" mm");
        layout->addRow(label, spinBoxSquareSize);

        // Separator
        separator = new QFrame(this);
        separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        layout->addRow(separator);

        // Button box
        buttonBox = new QDialogButtonBox(this);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        layout->addRow(buttonBox);

        buttonBox->addButton(QDialogButtonBox::Ok);
        buttonBox->addButton(QDialogButtonBox::Cancel);
    }
    
    virtual ~BoardParametersDialog ()
    {
    }

    void getParameters (int &boardWidth, int &boardHeight, float &squareSize)
    {
        boardWidth = spinBoxBoardWidth->value();
        boardHeight = spinBoxBoardHeight->value();
        squareSize = spinBoxSquareSize->value();
    }

protected:
    QSpinBox *spinBoxBoardWidth;
    QSpinBox *spinBoxBoardHeight;
    QDoubleSpinBox *spinBoxSquareSize;    
};



GuiCalibration::GuiCalibration (StereoPipeline *p, StereoCalibration *c, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), calibration(c)
{
    setWindowTitle("Stereo calibration");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Calibrate");
    pushButton->setToolTip("Calibrate from list of images.");
    connect(pushButton, SIGNAL(released()), this, SLOT(doCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonCalibrate = pushButton;

    pushButton = new QPushButton("Import calib.");
    pushButton->setToolTip("Import calibration from file.");
    connect(pushButton, SIGNAL(released()), this, SLOT(importCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonImport = pushButton;

    pushButton = new QPushButton("Export calib.");
    pushButton->setToolTip("Export current calibration to file.");
    connect(pushButton, SIGNAL(released()), this, SLOT(exportCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonExport = pushButton;

    pushButton = new QPushButton("Clear calib.");
    pushButton->setToolTip("Clear current calibration.");
    connect(pushButton, SIGNAL(released()), this, SLOT(clearCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonClear = pushButton;

    pushButton = new QPushButton("Save rectified images");
    pushButton->setToolTip("Save rectified image pair.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImages = pushButton;

    buttonsLayout->addStretch();

    // Rectified image pair
    displayPair = new ImagePairDisplayWidget("Rectified image pair", this);
    displayPair->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayPair, 1, 0);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);

    // Pipeline
    connect(pipeline, SIGNAL(rectifiedImagesChanged()), this, SLOT(updateImage()));

    // Calibration
    connect(calibration, SIGNAL(stateChanged(bool)), this, SLOT(updateState()));
    updateState();

    // Pattern dialog
    patternDialog = new BoardParametersDialog(this);
}

GuiCalibration::~GuiCalibration ()
{
}

void GuiCalibration::updateImage ()
{
    displayPair->setImagePair(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage());
}

void GuiCalibration::updateState ()
{
    if (calibration->getState()) {
        statusBar->showMessage("Calibration set; rectifying input images.");

        displayPair->setImagePairROI(calibration->getLeftROI(), calibration->getRightROI());

        pushButtonClear->setEnabled(true);
        pushButtonExport->setEnabled(true);
    } else {
        statusBar->showMessage("Calibration not set; passing input images through.");

        displayPair->setImagePairROI(cv::Rect(), cv::Rect());

        pushButtonClear->setEnabled(false);
        pushButtonExport->setEnabled(false);
    }

    //updateImage();
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void GuiCalibration::doCalibration ()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select calibration images or list file", QString(), "Images (*.jpg *.png *.bmp *.tif *.ppm *.pgm);; Text file (*.txt)");

    // If no files are chosen, stop
    if (!fileNames.size()) {
        return;
    }

    // If we are given a single file, assume it is a file list
    if (fileNames.size() == 1) {
        QFile listFile(fileNames[0]);
        if (listFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&listFile);
            QString line;
            
            // Clear file names
            fileNames.clear();

            while (1) {
                line = stream.readLine();
                if (line.isNull()) {
                    break;
                } else {
                    fileNames << line;
                }
            }

            if (!fileNames.size()) {
                throw QString("Image list is empty!");
            }
        } else {
            throw QString("Failed to open image list file \"%1\"!").arg(fileNames[0]);
        }
    }

    // Run pattern dialog
    if (patternDialog->exec() != QDialog::Accepted) {
        return;
    }
    
    int boardWidth;
    int boardHeight;
    float squareSize;
    patternDialog->getParameters(boardWidth, boardHeight, squareSize);

    // Finally, do calibration
    calibration->calibrateFromImages(fileNames, boardWidth, boardHeight, squareSize);
}

void GuiCalibration::importCalibration ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load calibration from file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        calibration->loadCalibration(fileName);
    }
}

void GuiCalibration::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        calibration->saveCalibration(fileName);
    }
}

void GuiCalibration::clearCalibration ()
{
    calibration->clearCalibration();
}

// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiCalibration::saveImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat tmpImg1, tmpImg2;

    pipeline->getLeftRectifiedImage().copyTo(tmpImg1);
    pipeline->getRightRectifiedImage().copyTo(tmpImg2);
    
    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save rectified images");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            ext = "jpg";
        }

        // Create filename
        QString fileNameLeft = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "L" + "." + ext;
        QString fileNameRight = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "R" + "." + ext;

        try {
            cv::imwrite(fileNameLeft.toStdString(), tmpImg1);
            cv::imwrite(fileNameRight.toStdString(), tmpImg2);
        } catch (cv::Exception e) {
            qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
        }
    }
}
