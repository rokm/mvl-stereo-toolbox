/*
 * Image File Source: image file widget
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__IMAGE_FILE_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__IMAGE_FILE_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
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


} // SourceImageFile
} // Pipeline
} // StereoToolbox
} // MVL


#endif
