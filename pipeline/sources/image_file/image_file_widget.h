/*
 * Image File Pair Source: image file config widget
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef IMAGE_FILE_CONFIG_WIGET_H
#define IMAGE_FILE_CONFIG_WIGET_H

#include <QtWidgets>


namespace SourceImageFile {
    
class ImageFile;
class UrlDialog;

class ImageFileWidget : public QWidget
{
    Q_OBJECT

public:
    ImageFileWidget (ImageFile *, QWidget * = 0);
    virtual ~ImageFileWidget ();

protected:
    ImageFile *file;
    
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

}

#endif
