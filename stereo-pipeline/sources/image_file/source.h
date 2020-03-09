/*
 * Image File Source: source
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__SOURCE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__IMAGE_FILE__SOURCE_H

#include <QtNetwork>

#include <stereo-pipeline/image_pair_source.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceImageFile {


class ImageFile;

class Source : public QObject, public ImagePairSource
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::ImagePairSource)

public:
    Source (QObject *parent = nullptr);
    virtual ~Source ();

    virtual QString getShortName () const override;
    virtual void getImages (cv::Mat &left, cv::Mat &right) const override;
    virtual void stopSource () override;
    virtual QWidget *createConfigWidget (QWidget *parent = nullptr) override;

    ImageFile *getLeftImageFile ();
    ImageFile *getRightImageFile ();

    void loadImagePair (const QString &left, const QString &right, bool remote);

    bool getPeriodicRefreshState () const;
    int getRefreshPeriod () const;

    void setPeriodicRefreshState (bool enable);
    void setRefreshPeriod (int newPeriod);

protected:
    void periodicRefresh ();
    void synchronizeFrames ();

signals:
    void periodicRefreshStateChanged (bool enabled);
    void refreshPeriodChanged (int period);

    // Signals from interface
    void imagesChanged () override;
    void error (QString message) override;

protected:
    QTimer *refreshTimer;
    int refreshPeriod;

    ImageFile *leftImageFile;
    ImageFile *rightImageFile;

    bool leftImageReady, rightImageReady;

    // Images
    mutable QReadWriteLock imagesLock;

    cv::Mat imageLeft;
    cv::Mat imageRight;
};


} // SourceImageFile
} // Pipeline
} // StereoToolbox
} // MVL


#endif
