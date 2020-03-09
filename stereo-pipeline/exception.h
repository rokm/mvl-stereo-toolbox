/*
 * Stereo Pipeline: exception
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__EXCEPTION_H
#define MVL_STEREO_TOOLBOX__PIPELINE__EXCEPTION_H

#include <QtCore>
#include <stdexcept>

#include <stereo-pipeline/export.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class MVL_STEREO_PIPELINE_EXPORT Exception : public std::runtime_error
{
public:
    Exception (const QString &message);
    Exception (const QString &messagePrefix, const std::exception &e);

    virtual ~Exception ();
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
