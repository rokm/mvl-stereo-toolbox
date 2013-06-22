#ifndef IMAGE_FILE_CAPTURE_CONFIG_WIGET_H
#define IMAGE_FILE_CAPTURE_CONFIG_WIGET_H

#include <QtGui>


class VideoFileCapture;
class UrlDialog;

class VideoFileCaptureConfigWidget : public QWidget
{
    Q_OBJECT

public:
    VideoFileCaptureConfigWidget (VideoFileCapture *, QWidget * = 0);
    virtual ~VideoFileCaptureConfigWidget ();

protected slots:
    void loadFile ();
    void loadUrl ();

    void updateVideoInformation ();

protected:
    VideoFileCapture *capture;

    QPushButton *pushButtonFile;
    QPushButton *pushButtonUrl;
    
    QTextEdit *textEditFilename;
    QLabel *labelResolution;
    
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
