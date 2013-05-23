/*
 * Stereo Pipeline: stereo method
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

#ifndef STEREO_METHOD_H
#define STEREO_METHOD_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class QWidget;

class StereoMethod : public QObject
{
    Q_OBJECT

public:
    StereoMethod (QObject * = 0);
    virtual ~StereoMethod ();

    // Image information
    void setImageDimensions (int, int, int);
    
    int getImageWidth () const;
    int getImageHeight () const;
    int getImageChannels () const;
    
    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &) = 0;

    // Parameter import/export
    void loadParameters (const QString &);
    virtual void loadParameters (const cv::FileStorage &);

    void saveParameters (const QString &) const;
    virtual void saveParameters (cv::FileStorage &) const;

    // Config interface
    const QString &getShortName () const;
    QWidget *getConfigWidget ();

    // Generic parameter setting
    template <typename T> void setParameter (T &parameter, const T &newValue) {
        // Set only if necessary
        if (parameter != newValue) {
            parameter = newValue;
            emit parameterChanged();
        }
    }

signals:
    void imageDimensionsChanged ();
    void parameterChanged ();

protected:
    QString shortName;
    QWidget *configWidget;

    // Image dimensions
    int imageWidth;
    int imageHeight;
    int imageChannels;
};

#endif
