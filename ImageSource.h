#ifndef IMAGE_SOURCE_H
#define IMAGE_SOURCE_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class ImageSource : public QObject
{
    Q_OBJECT

public:
    ImageSource (QObject * = 0);
    virtual ~ImageSource ();

    // Config interface
    const QString &getShortName () const;
    QWidget *getConfigWidget ();

protected:
    QString shortName;
    QWidget *configWidget;
};

#endif
