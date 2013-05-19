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

    const QString &getLeftFilename () const;
    int getLeftWidth () const;
    int getLeftHeight () const;
    int getLeftChannels () const;

    const QString &getRightFilename () const;
    int getRightWidth () const;
    int getRightHeight () const;
    int getRightChannels () const;
    
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
