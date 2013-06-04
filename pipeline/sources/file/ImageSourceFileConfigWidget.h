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
class UrlDialog;

class ImageSourceFileConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    ImageSourceFileConfigWidget (ImageSourceFile *, QWidget * = 0);
    virtual ~ImageSourceFileConfigWidget ();

protected slots:
    void loadFiles ();
    void loadUrls ();

    void updateImageInformation ();

protected:
    ImageSourceFile *source;

    QPushButton *pushButtonLoadFiles;
    QPushButton *pushButtonLoadUrls;

    QPushButton *pushButtonPeriodicRefresh;

    QSpinBox *spinBoxRefreshPeriod;
    
    QLabel *labelFilenameLeft;
    QLabel *labelFilenameRight;

    UrlDialog *dialogUrl;
};


class UrlDialog : public QDialog
{
    Q_OBJECT

public:
    UrlDialog (QWidget * = 0);
    virtual ~UrlDialog ();

    QString getUrlLeft () const;
    QString getUrlRight () const;

protected:
    QTextEdit *textEditUrl1;
    QTextEdit *textEditUrl2;
};

#endif
