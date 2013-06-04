/*
 * Stereo Pipeline: image source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

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

    const QString &getShortName () const;

    virtual void getImages (cv::Mat &, cv::Mat &);

    virtual void stopSource ();

    // Config widget
    virtual QWidget *createConfigWidget (QWidget * = 0) = 0;

signals:
    void imagesChanged ();

    void error (const QString);

protected:
    QString shortName;
    
    // Images
    QReadWriteLock imagesLock;
    
    cv::Mat imageLeft;
    cv::Mat imageRight;
};

#endif
