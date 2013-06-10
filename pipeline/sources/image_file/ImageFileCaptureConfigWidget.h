#ifndef IMAGE_FILE_CAPTURE_CONFIG_WIGET_H
#define IMAGE_FILE_CAPTURE_CONFIG_WIGET_H

#include <QtGui>


class ImageFileCapture;
class UrlDialog;

class ImageFileCaptureConfigWidget : public QWidget
{
    Q_OBJECT

public:
    ImageFileCaptureConfigWidget (ImageFileCapture *, QWidget * = 0);
    virtual ~ImageFileCaptureConfigWidget ();

protected slots:
    void loadFile ();
    void loadUrl ();

    void updateImageInformation ();

protected:
    ImageFileCapture *capture;

    QPushButton *pushButtonFile;
    QPushButton *pushButtonUrl;
    
    QTextEdit *textEditFilename;
    QLabel *labelResolution;
    QLabel *labelChannels;
    
    UrlDialog *dialogUrl;
};

// URL dialog
class UrlDialog : public QDialog
{
    Q_OBJECT

public:
    UrlDialog (QWidget * = 0);
    virtual ~UrlDialog ();

    QString getUrl () const;

protected:
    QTextEdit *textEditUrl;
};


#endif
