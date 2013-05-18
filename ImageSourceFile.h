#ifndef IMAGE_SOURCE_FILE_H
#define IMAGE_SOURCE_FILE_H

#include "ImageSource.h"


class ImageSourceFile : public ImageSource
{
    Q_OBJECT

public:
    ImageSourceFile (QObject * = 0);
    virtual ~ImageSourceFile ();

    void loadImagePair (const QString &, const QString &);

    const QString &getFilenameLeft () const;
    const QString &getFilenameRight () const;

protected:
    QString filenameLeft;
    QString filenameRight;
};


// Config widget
#include <QtGui>

class ConfigTabFile : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabFile (ImageSourceFile *, QWidget * = 0);
    virtual ~ConfigTabFile ();

protected slots:
    void loadImages ();

protected:
    ImageSourceFile *source;

    QPushButton *buttonLoadImages;
    QLabel *labelFilenameLeft;
    QLabel *labelFilenameRight;
};

#endif
