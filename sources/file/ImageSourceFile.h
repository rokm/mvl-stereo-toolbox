/*
 * File Image Source: source
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

#endif
