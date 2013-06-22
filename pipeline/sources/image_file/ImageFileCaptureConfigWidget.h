/*
 * Image File Pair Source: image capture config widget
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
