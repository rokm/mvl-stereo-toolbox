/*
 * File Image Source: config widget
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

#ifndef IMAGE_SOURCE_FILE_CONFIG_WIDGET_H
#define IMAGE_SOURCE_FILE_CONFIG_WIDGET_H

#include <QtGui>


class ImageSourceFile;

class ImageSourceFileConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    ImageSourceFileConfigWidget (ImageSourceFile *, QWidget * = 0);
    virtual ~ImageSourceFileConfigWidget ();

protected slots:
    void loadImages ();

protected:
    ImageSourceFile *source;

    QPushButton *buttonLoadImages;
    QLabel *labelFilenameLeft;
    QLabel *labelFilenameRight;
};

#endif
