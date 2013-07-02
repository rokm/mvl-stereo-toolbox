/*
 * Stereo Pipeline: stereo method interface
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

#ifndef STEREO_METHOD_H
#define STEREO_METHOD_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class QWidget;

class StereoMethod
{
public:
    StereoMethod ();
    virtual ~StereoMethod ();

    const QString &getShortName () const;

    // Config widget
    virtual QWidget *createConfigWidget (QWidget * = 0) = 0;

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &) = 0;

    // Parameter import/export
    void loadParameters (const QString &);
    virtual void loadParameters (const cv::FileStorage &);

    void saveParameters (const QString &) const;
    virtual void saveParameters (cv::FileStorage &) const;

    // Generic parameter setting
    template <typename T> void setParameter (T &parameter, const T &newValue) {
        // Set only if necessary
        if (parameter != newValue) {
            mutex.lock();
            parameter = newValue;
            mutex.unlock();
            
            emit parameterChanged();
        }
    }

    // These are actually signals, but they are not allowed in non-QObject classes
protected:
    virtual void parameterChanged () = 0;

protected:
    QString shortName;

    QMutex mutex;
};

Q_DECLARE_INTERFACE(StereoMethod, "MVL_Stereo_Toolbox.StereoMethod/1.0")

#endif
