/*
 * MP (Multi-Picture) file parser
 * Copyright (C) 2018 Rok Mandeljc
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

#ifndef MPO_FILE_H
#define MPO_FILE_H


#include <QtCore>
#include <opencv2/core.hpp>


namespace Mpo {


// Unsigned rational number; nominator and denominator
typedef QPair<quint32, quint32> MpRational;

// Signed rational number; nominator and denominator
typedef QPair<qint32, qint32> MpSRational;


// Image attribute map
typedef QHash<quint16, QVariant> MpImageAttributeMap;

// MP Individual Image attributes defined by CIPA DC-007-2009
enum MpImageAttribute
{
    MPFVersion = 0xB000, // QVector<quint8> of length 4
    MPIndividualNum = 0xB101, // quint32
    PanOrientation = 0xB201, // quint32
    PanOverlap_H = 0xB202, // MpRational
    PanOverlap_V = 0xB203, // MpRational
    BaseViewpointNum = 0xB204, // quint32
    ConvergenceAngle = 0xB205, // MpSRational
    BaselineLength = 0xB206, // MpRational
    VerticalDivergence = 0xB207, // MpSRational
    AxisDistance_X = 0xB208, // MpSRational
    AxisDistance_Y = 0xB209, // MpSRational
    AxisDistance_Z = 0xB20A, // MpSRational
    YawAngle = 0xB20B, // MpSRational
    PitchAngle = 0xB20C, // MpSRational
    RollAngle = 0xB20D // MpSRational
};


// Multi-picture file image information entry
struct MpImageInfo
{
    bool dependentParent; // Image is a parent image
    bool dependentChild; // Image is a child image

    bool representative; // Image is representative image

    quint32 imageFormat; // Image format: 0 = JPEG

    quint32 imageType; // Image type: see MpImageType enum

    quint32 dataSize; // Image data length
    quint32 dataOffset; // Image data offset

    quint16 dependentEntry1; // 1st dependent entry
    quint16 dependentEntry2; // 2nd dependent entry

    // Attribute map
    MpImageAttributeMap attributes;
};


// Image type codes
enum MpImageType
{
    BaselinePrimaryImage = 0x030000,
    LargeThumbnailVGA    = 0x010001,
    LargeThubmnailFullHD = 0x010002,
    PanoramaImage        = 0x020001,
    DisparityImage       = 0x020002,
    MultiAngleImage      = 0x020003,
    UndefinedImage       = 0x000000,
};


// MP object file file
class MpFilePrivate;

class MpFile : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MpFile)
    Q_DECLARE_PRIVATE(MpFile)

protected:
    QScopedPointer<MpFilePrivate> const d_ptr;
    MpFile (MpFilePrivate *d, QObject *parent = nullptr);

public:
    MpFile (QObject *parent = nullptr);
    MpFile (const QString &filename, QObject *parent = nullptr);

    virtual ~MpFile ();

    // Load an MPO file
    void loadFile (const QString &filename);

    // Returns number of all images in the MPO file
    int getNumberOfImages () const;

    // Returns index of reference/representative image in the MPO file
    int getReferenceImage () const;

    // Retrieve information about the specified image
    const MpImageInfo &getImageInfo (int idx) const;

    // Load image with the given index
    cv::Mat loadImage (int idx);
};


} // Mpo

#endif
