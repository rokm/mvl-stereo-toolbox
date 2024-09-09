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

#include "mpo_file.h"

#include <opencv2/imgcodecs.hpp>


Q_LOGGING_CATEGORY(debugMpo, "mpo-file", QtWarningMsg)


namespace Mpo {


// Raw IFD structure
typedef QHash< quint16, QPair<quint16, QVariant> > MpRawIfd;

static QDataStream& operator >> (QDataStream &stream, MpImageInfo &entry);


// *********************************************************************
// *                             Exception                             *
// *********************************************************************
class Exception : public std::runtime_error
{
public:
    Exception (const QString &message);
    Exception (const QString &messagePrefix, const std::exception &e);

    virtual ~Exception ();
};


// Exception implementation
Exception::Exception (const QString &message)
    : std::runtime_error(message.toStdString())
{
}

Exception::Exception (const QString &messagePrefix, const std::exception &e)
    : Exception(QStringLiteral("%1: %2").arg(messagePrefix).arg(QString::fromStdString(e.what())))
{
}

Exception::~Exception ()
{
}


// *********************************************************************
// *                 Private implementation of MpFile                  *
// *********************************************************************
class MpFilePrivate
{
    Q_DISABLE_COPY(MpFilePrivate)
    Q_DECLARE_PUBLIC(MpFile)

protected:
    MpFilePrivate (MpFile *parent);
    MpFile * const q_ptr;

protected:
    static QByteArray getMpoMarker (QFile &file, quint32 &markerOffset);

    static bool determineMarkerByteOrder (const QByteArray &data);

    static void parseFirstMpoMarker (const QByteArray &data, QList<MpImageInfo> &imageEntries, MpImageAttributeMap &firstImageAttributes);
    static void parseMpoMarker (const QByteArray &data, MpImageAttributeMap &imageAttributes);

    static MpRawIfd readMpIfd (QDataStream &stream, quint32 baseOffset, quint32 &offsetToNextIfd);

    static QList<MpImageInfo> parseMpIndexIfd (const MpRawIfd &ifd, bool bigEndian);
    static MpImageAttributeMap parseMpAttributeIfd (const MpRawIfd &ifd, bool bigEndian);

protected:
    QString filename; // Filename

    QList<MpImageInfo> imageEntries; // MPO Image entries

    int referenceImage; // Index of the reference image
};


MpFilePrivate::MpFilePrivate (MpFile *parent)
    : q_ptr(parent),
      referenceImage(-1)
{
}


// MpFile implementation
MpFile::MpFile (QObject *parent)
    : QObject(parent), d_ptr(new MpFilePrivate(this))
{
}

MpFile::MpFile (const QString &filename, QObject *parent)
    : MpFile(parent)
{
    loadFile(filename);
}

MpFile::~MpFile ()
{
}


int MpFile::getNumberOfImages () const
{
    Q_D(const MpFile);
    return d->imageEntries.size();
}

int MpFile::getReferenceImage () const
{
    Q_D(const MpFile);
    return d->referenceImage;
}


const MpImageInfo &MpFile::getImageInfo (int idx) const
{
    Q_D(const MpFile);
    return d->imageEntries[idx];
}

cv::Mat MpFile::loadImage (int idx)
{
    Q_D(const MpFile);

    cv::Mat image;

    if (idx < 0 || idx >= d->imageEntries.size()) {
        return image;
    }

    const MpImageInfo &entry = d->imageEntries[idx];

    // Open file
    QFile file(d->filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return image;
    }

    // Seek to position in file and read the image
    file.seek(entry.dataOffset);

    QByteArray buffer(entry.dataSize, 0);
    if (file.read(buffer.data(), buffer.size()) != buffer.size()) {
        return image;
    }

    // Decode
    return cv::imdecode(cv::Mat(buffer.size(), 1, CV_8U, buffer.data()), cv::IMREAD_ANYCOLOR);
}


// *********************************************************************
// *                          MPO file parser                          *
// *********************************************************************
void MpFile::loadFile (const QString &filename)
{
    Q_D(MpFile);

    // Cleanup
    d->imageEntries.clear();
    d->referenceImage = -1;
    d->filename = filename;

    // Open file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw Exception(QStringLiteral("Failed to open file %1 for reading!").arg(filename));
    }

    // Get the first MPO marker
    quint32 mpoMarkerDataOffset = 0; // Input/output parameter
    QByteArray mpoMarkerData;

    try {
        mpoMarkerData = d->getMpoMarker(file, mpoMarkerDataOffset);
    } catch (const std::exception &e) {
        throw Exception(QStringLiteral("Invalid MPO file - parser error"), e);
    }

    if (mpoMarkerData.isEmpty()) {
        throw Exception(QStringLiteral("Invalid MPO file - no MPO marker data found!"));
    }

    // Parse the first MPO marker
    MpImageAttributeMap firstImageAttributes;
    try {
        d->parseFirstMpoMarker(mpoMarkerData, d->imageEntries, firstImageAttributes);
    } catch (const std::exception &e) {
        throw Exception(QStringLiteral("Failed to parse MPO marker"), e);
    }

    // Store attributes of first image
    d->imageEntries[0].attributes = firstImageAttributes;

    // Process the image entries
    for (int i = 0; i < d->imageEntries.size(); i++) {
        MpImageInfo &entry = d->imageEntries[i];

        if (i >= 1) {
            // Adjust the offsets
            entry.dataOffset += mpoMarkerDataOffset;
        }

        if (entry.representative) {
            d->referenceImage = i;
        }
    }

    // Retrieve attributes for images
    qCDebug(debugMpo) << "Retrieving attributes for images...";
    for (int i = 1; i < d->imageEntries.size(); i++) {
        MpImageInfo &entry = d->imageEntries[i];

        mpoMarkerDataOffset = entry.dataOffset;

        qCDebug(debugMpo) << "Retrieving attributes for image" << i << "from file offset" << mpoMarkerDataOffset;
        try {
            mpoMarkerData = d->getMpoMarker(file, mpoMarkerDataOffset);
        }  catch (const std::exception &e) {
            qCDebug(debugMpo).nospace() << "Failed to retrieve MPO marker data for image " << i << ": " << QString::fromStdString(e.what());
            continue;
        }

        if (mpoMarkerData.isEmpty()) {
            qCDebug(debugMpo) << "No MPO marker found for image" << i;
            continue;
        }

        // Parse attributes into entry's attribute map
        d->parseMpoMarker(mpoMarkerData, entry.attributes);
    }
}


// *********************************************************************
// *                     MPO Marker data retrieval                     *
// *********************************************************************
// Marker types
enum MarkerType
{
    SOI   = 0xD8, SOF0  = 0xC0, SOF2  = 0xC2, DHT   = 0xC4,
    DQT   = 0xDB, DRI   = 0xDD, SOS   = 0xDA,

    RST0  = 0xD0, RST1  = 0xD1, RST2  = 0xD2, RST3  = 0xD3,
    RST4  = 0xD4, RST5  = 0xD5, RST6  = 0xD6, RST7  = 0xD7,

    APP0  = 0xE0, APP1  = 0xE1, APP2  = 0xE2, APP3  = 0xE3,
    APP4  = 0xE4, APP5  = 0xE5, APP6  = 0xE6, APP7  = 0xE7,
    APP8  = 0xE8, APP9  = 0xE9, APP10 = 0xEA, APP11 = 0xEB,
    APP12 = 0xEC, APP13 = 0xED, APP14 = 0xEE, APP15 = 0xEF,

    COM   = 0xFE, EOI   = 0xD9
};

// Attempts to locate MPO marker in the file from the specified offset
// on. Returns the buffer containing the marker data, and stores the
// offset at which the marker data was located.
QByteArray MpFilePrivate::getMpoMarker (QFile &file, quint32 &markerOffset)
{
    // Seek to designated marker offset
    file.seek(markerOffset);

    // Create big-endian data stream
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);

    // Read header - we should be located at the SOI marker
    quint8 header[2];
    stream >> header[0] >> header[1];

    if (!(header[0] == 0xFF && header[1] == MarkerType::SOI)) {
        throw Exception(QStringLiteral("Invalid file header (found 0x%1 0x%2; expected 0xFF 0xD8)!").arg(header[0], 0, 16).arg(header[1], 0, 16));
    }

    while (!stream.atEnd()) {
        quint8 marker[2];
        quint16 markerSize;

        stream >> marker[0] >> marker[1];

        if (marker[0] != 0xFF) {
            throw Exception(QStringLiteral("Invalid marker; does not begin with 0xFF!"));
        }

        qCDebug(debugMpo) << "Found marker:" << QString::number(marker[0], 16) << QString::number(marker[1], 16);

        switch (marker[1]) {
            // For all the markers just skip bytes in file pointed by followed two bytes (field size)
            case MarkerType::SOF0:
            case MarkerType::SOF2:
            case MarkerType::DHT:
            case MarkerType::DQT:
            case MarkerType::DRI:
            case MarkerType::SOS:
            case MarkerType::RST0:
            case MarkerType::RST1:
            case MarkerType::RST2:
            case MarkerType::RST3:
            case MarkerType::RST4:
            case MarkerType::RST5:
            case MarkerType::RST6:
            case MarkerType::RST7:
            case MarkerType::APP0:
            case MarkerType::APP1:
            case MarkerType::APP3:
            case MarkerType::APP4:
            case MarkerType::APP5:
            case MarkerType::APP6:
            case MarkerType::APP7:
            case MarkerType::APP8:
            case MarkerType::APP9:
            case MarkerType::APP10:
            case MarkerType::APP11:
            case MarkerType::APP12:
            case MarkerType::APP13:
            case MarkerType::APP14:
            case MarkerType::APP15:
            case MarkerType::COM: {
                // Read marker size, and skip marker
                stream >> markerSize;
                stream.skipRawData(markerSize - 2);
                break;
            }
            // SOI and EOI don't have the size field after the marker
            case MarkerType::SOI: {
                break;
            }
            case MarkerType::EOI: {
                // Return empty array to indicate that no marker was
                // found
                return QByteArray();
            }
            case MarkerType::APP2: {
                stream >> markerSize;

                // Store marker offset
                quint32 tmpMarkerOffset = file.pos();

                // Read whole marker
                QByteArray buffer(markerSize - 2, 0); // Allocate buffer
                if (stream.readRawData(buffer.data(), buffer.size()) != buffer.size()) {
                    throw Exception(QStringLiteral("Failed to read whole marker data!"));
                }

                if (buffer[0] == 'M' && buffer[1] == 'P' && buffer[2] == 'F' && buffer[3] == '\x00') {
                    markerOffset = tmpMarkerOffset;
                    return buffer;
                }

                break;
            }
            default: {
                throw Exception(QStringLiteral("Invalid marker code: %1").arg(marker[1], 16));
            }
        }
    }

    return QByteArray();
}


// *********************************************************************
// *                         MPO Marker parser                         *
// *********************************************************************
bool MpFilePrivate::determineMarkerByteOrder (const QByteArray &data)
{
    if (data[4] == 0x49 && data[5] == 0x49 && data[6] == 0x2A && data[7] == 0x00) {
        return false; // Little endian
    } else if (data[4] == 0x4D && data[5] == 0x4D && data[6] == 0x00 && data[7] == 0x2A) {
        return true; // Big endian
    }

    throw Exception(
        QStringLiteral("Invalid MP endian tag: %1 %2 %3 %4!")
            .arg(quint8(data[0]), 0, 16)
            .arg(quint8(data[1]), 0, 16)
            .arg(quint8(data[2]), 0, 16)
            .arg(quint8(data[3]), 0, 16)
    );
}


void MpFilePrivate::parseMpoMarker (const QByteArray &data, MpImageAttributeMap &imageAttributes)
{
    // We already read first four bytes

    // MP endian (4 bytes)
    bool bigEndian = determineMarkerByteOrder(data);

    qCDebug(debugMpo) << "Byte order:" << (bigEndian ? "big" : "little");

    // Construct a seekable buffer on top of data
    QBuffer buffer;
    buffer.setData(data); // The only way to set const QByteArray
    buffer.open(QIODevice::ReadOnly);

    QDataStream stream(&buffer);
    stream.setByteOrder(bigEndian ? QDataStream::BigEndian : QDataStream::LittleEndian);

    stream.skipRawData(8);

    quint32 baseOffset = 4; // Base offset are the MP Endian bytes

    // Offset to first IFD (4 bytes)
    quint32 firstIfdOffset;
    stream >> firstIfdOffset;

    // *** MP Attribute IFD for image ***
    MpRawIfd mpAttributeIfd;
    quint32 nextIfdOffset;

    // Read
    stream.device()->seek(baseOffset + firstIfdOffset);
    qCDebug(debugMpo) << "Reading MP Attribute IFD from offset" << stream.device()->pos();
    mpAttributeIfd = readMpIfd(stream, baseOffset, nextIfdOffset);

    // Parse
    qCDebug(debugMpo) << "Parsing MP Attribute IFD...";
    imageAttributes = parseMpAttributeIfd(mpAttributeIfd, bigEndian);
}

void MpFilePrivate::parseFirstMpoMarker (const QByteArray &data, QList<MpImageInfo> &imageEntries, MpImageAttributeMap &firstImageAttributes)
{
    // We already read first four bytes

    // MP endian (4 bytes)
    bool bigEndian = determineMarkerByteOrder(data);

    qCDebug(debugMpo) << "Byte order:" << (bigEndian ? "big" : "little");

    // Construct a seekable buffer on top of data
    QBuffer buffer;
    buffer.setData(data); // The only way to set const QByteArray
    buffer.open(QIODevice::ReadOnly);

    QDataStream stream(&buffer);
    stream.setByteOrder(bigEndian ? QDataStream::BigEndian : QDataStream::LittleEndian);

    stream.skipRawData(8);

    quint32 baseOffset = 4; // Base offset are the MP Endian bytes

    // Offset to first IFD (4 bytes)
    quint32 firstIfdOffset;
    stream >> firstIfdOffset;

    // *** MP Index IFD ***
    MpRawIfd mpIndexIfd;
    quint32 attributeIfdOffset;

    // Read
    stream.device()->seek(baseOffset + firstIfdOffset);
    qCDebug(debugMpo) << "Reading MP Index IFD from offset" << stream.device()->pos();
    mpIndexIfd = readMpIfd(stream, baseOffset, attributeIfdOffset);

    // Parse
    qCDebug(debugMpo) << "Parsing MP Index IFD...";
    imageEntries = parseMpIndexIfd(mpIndexIfd, bigEndian);

    for (int i = 1; i < imageEntries.size(); i++) {
        MpImageInfo &entry = imageEntries[i];
        entry.dataOffset += baseOffset; // Adjust the offset by base offset
    }

    // *** MP Attribute IFD for first image ***
    if (attributeIfdOffset != 0) {
        MpRawIfd mpAttributeIfd;
        quint32 nextIfdOffset;

        // Read
        stream.device()->seek(baseOffset + attributeIfdOffset);
        qCDebug(debugMpo) << "Reading MP Attribute IFD for first image from offset" << stream.device()->pos();
        mpAttributeIfd = readMpIfd(stream, baseOffset, nextIfdOffset);

        // Parse
        qCDebug(debugMpo) << "Parsing MP Attribute IFD for first image...";
        firstImageAttributes = parseMpAttributeIfd(mpAttributeIfd, bigEndian);
    }
}

// *********************************************************************
// *                        Generic IFD parser                         *
// *********************************************************************
// Tag value types
enum TagValueType
{
    BYTE = 1,
    ASCII = 2,
    SHORT = 3,
    LONG = 4,
    RATIONAL = 5,
    UNDEFINED = 7,
    SLONG = 9,
    SRATIONAL = 10,
};

MpRawIfd MpFilePrivate::readMpIfd (QDataStream &stream, quint32 baseOffset, quint32 &offsetToNextIfd)
{
    MpRawIfd ifd;

    // Number of entries (2-byte)
    quint16 count;
    stream >> count;

    qCDebug(debugMpo) << "IFD has" << count << "entries";

    for (int i = 0; i < count; i++) {
        // Bytes 0-1: Tag
        // Bytes 2-3: Type:
        //   -  1: BYTE
        //   -  2: ASCII
        //   -  3: SHORT (2-byte)
        //   -  4: LONG (4-byte)
        //   -  5: RATIONAL (2 x 4-byte LONG)
        //   -  7: UNDEFINED (bytes)
        //   -  9: SLONG (4-byte signed LONG)
        //   - 10: SRATIONAL (2 x 4-byte SLONG)
        // Bytes 4-7: Count
        // Bytes 8-11: Value offset: "In cases where the value fits in
        //   4 Bytes, the value itself is recorded. If the value is
        //   smaller than 4 Bytes, the value is stored in the 4-byte
        //   area starting from the left, i.e., from the lower end of
        //   the byte offset area. For example, in big endian format,
        //   if the type is SHORT and the value is 1, it is  recorded as
        //   00010000h.

        quint16 tagId;
        quint16 valueType;
        quint32 valueCount;

        stream >> tagId;
        stream >> valueType;
        stream >> valueCount;

        qCDebug(debugMpo) << "Tag:" << QString::number(tagId, 16) << ", type" << valueType << ", element count" << valueCount;

        switch (valueType) {
            case TagValueType::LONG: {
                // Long
                QVector<quint32> values(valueCount);

                if (valueCount == 1) {
                    stream >> values[0];
                } else {
                    // For now, we support only single-value LONG
                    throw Exception(QStringLiteral("Multi-value LONG tags are not supported!"));
                }

                ifd.insert(tagId, qMakePair(valueType, QVariant::fromValue(values)));
                break;
            }
            case TagValueType::RATIONAL: {
                // Rational (two longs)
                QVector<MpRational> values(valueCount);

                // Always read from offset
                if (true) {
                    quint32 offset;
                    stream >> offset;

                    // Seek to the offset, read the bytes, seek back
                    qint64 oldPos = stream.device()->pos();

                    stream.device()->seek(baseOffset + offset);
                    for (quint32 i = 0; i < valueCount; i++) {
                        stream >> values[i].first;
                        stream >> values[i].second;
                    }
                    stream.device()->seek(oldPos);
                }

                ifd.insert(tagId, qMakePair(valueType, QVariant::fromValue(values)));
                break;
            }
            case TagValueType::UNDEFINED: {
                // Undefined (bytes)
                QVector<quint8> values(valueCount);

                if (valueCount <= 4) {
                    stream.readRawData(reinterpret_cast<char*>(values.data()), valueCount);
                    stream.skipRawData(4 - valueCount);
                } else {
                    quint32 offset;
                    stream >> offset;

                    // Seek to the offset, read the bytes, seek back
                    qint64 oldPos = stream.device()->pos();

                    stream.device()->seek(baseOffset + offset);
                    stream.readRawData(reinterpret_cast<char*>(values.data()), valueCount);
                    stream.device()->seek(oldPos);
                }

                ifd.insert(tagId, qMakePair(valueType, QVariant::fromValue(values)));
                break;
            }
            case TagValueType::SRATIONAL: {
                // Signed rational (two signed longs)
                QVector<MpSRational> values(valueCount);

                // Always read from offset
                if (true) {
                    quint32 offset;
                    stream >> offset;

                    // Seek to the offset, read the bytes, seek back
                    qint64 oldPos = stream.device()->pos();

                    stream.device()->seek(baseOffset + offset);
                    for (quint32 i = 0; i < valueCount; i++) {
                        stream >> values[i].first;
                        stream >> values[i].second;
                    }
                    stream.device()->seek(oldPos);
                }

                ifd.insert(tagId, qMakePair(valueType, QVariant::fromValue(values)));
                break;
            }
            default: {
                throw Exception(QStringLiteral("Unhandled tag data type: %1!").arg(valueType));
            }
        }
    }

    // Offset to next IFD
    stream >> offsetToNextIfd;

    qCDebug(debugMpo) << "Offset to next IFD" << offsetToNextIfd;

    return ifd;
}


// *********************************************************************
// *                        MP Index IFD parser                        *
// *********************************************************************
QList<MpImageInfo> MpFilePrivate::parseMpIndexIfd (const MpRawIfd &ifd, bool bigEndian)
{
    QList<MpImageInfo> imageEntries;

    MpRawIfd::ConstIterator iter;

    // MP Format Version Number
    iter = ifd.constFind(0xB000);
    if (iter == ifd.constEnd()) {
        throw Exception(QStringLiteral("Missing 'MPFVersion' tag!"));
    } else {
        quint16 type = iter.value().first;
        const QVector<quint8> &value = iter.value().second.value< QVector<quint8> >();

        if (type != TagValueType::UNDEFINED) {
            throw Exception(QStringLiteral("'MPFVersion' is not of type UNDEFINED!"));
        }

        if (value.size() != 4) {
            throw Exception(QStringLiteral("'MPFVersion' does not have 4 elements!"));
        }

        if (value[0] != '0' || value[1] != '1' || value[2] != '0' || value[3] != '0') {
            throw Exception(QStringLiteral("'MPFVersion' value is not 0100!"));
        }
    }

    // Number of Images
    int numberOfImages;

    iter = ifd.constFind(0xB001);
    if (iter == ifd.constEnd()) {
        throw Exception(QStringLiteral("Missing 'NumberOfImages' tag!"));
    } else {
        quint16 type = iter.value().first;
        const QVector<quint32> &value = iter.value().second.value< QVector<quint32> >();

        if (type != TagValueType::LONG) {
            throw Exception(QStringLiteral("'NumberOfImages' is not of type LONG!"));
        }

        if (value.size() != 1) {
            throw Exception(QStringLiteral("'NumberOfImages' does not have 1 element!"));
        }

        numberOfImages = value[0];
    }

    // MP Entry
    iter = ifd.constFind(0xB002);
    if (iter == ifd.constEnd()) {
        throw Exception(QStringLiteral("Missing 'MPEntry' tag!"));
    } else {
        quint16 type = iter.value().first;
        const QVector<quint8> &value = iter.value().second.value< QVector<quint8> >();

        if (type != TagValueType::UNDEFINED) {
            throw Exception(QStringLiteral("'MPEntry' is not of type UNDEFINED!"));
        }

        if (value.size() != 16*numberOfImages) {
            throw Exception(QStringLiteral("'MPEntry' has incorrect number of elements (should be 16x%1; value is %2)").arg(numberOfImages).arg(value.size()));
        }

        // Parse the MPO entries
        QByteArray valueData(reinterpret_cast<const char*>(value.data()), value.size());

        QDataStream stream(valueData);
        stream.setByteOrder(bigEndian ? QDataStream::BigEndian : QDataStream::LittleEndian);

        imageEntries.reserve(numberOfImages);

        for (int i = 0; i < numberOfImages; i++) {
            MpImageInfo entry;
            stream >> entry;

            // Append
            imageEntries.append(entry);
        }
    }

    return imageEntries;
}


// *********************************************************************
// *                      MP Attribute IFD parser                      *
// *********************************************************************
template<typename T>
void copyAttributeValue (const QString &name, quint16 code, int expectedCount, const QVariant &data, MpImageAttributeMap &attributeMap)
{
    // Generic parser always stores values in vectors
    const QVector<T> &value = data.value< QVector<T> >();

    if (value.size() != expectedCount) {
        throw Exception(QStringLiteral("'%1' has incorrect number of elements (value is %1; expected %2)").arg(name).arg(value.size()).arg(expectedCount));
    }

    if (expectedCount == 1) {
        attributeMap.insert(code, QVariant::fromValue(value[0]));
    } else {
        attributeMap.insert(code, QVariant::fromValue(value));
    }
}


MpImageAttributeMap MpFilePrivate::parseMpAttributeIfd (const MpRawIfd &ifd, bool bigEndian)
{
    Q_UNUSED(bigEndian)

    const struct ValidAttribute {
        QString name;
        quint32 code;
        quint32 type;
        int count;
    } validAttributes[] = {
        { "MPFVersion", MpImageAttribute::MPFVersion, TagValueType::UNDEFINED, 4 },
        { "MPIndividualNum", MpImageAttribute::MPIndividualNum, TagValueType::LONG, 1 },
        { "PanOrientation", MpImageAttribute::PanOrientation, TagValueType::LONG, 1 },
        { "PanOverlap_H", MpImageAttribute::PanOverlap_H, TagValueType::RATIONAL, 1 },
        { "PanOverlap_V", MpImageAttribute::PanOverlap_V, TagValueType::RATIONAL, 1 },
        { "BaseViewpointNum", MpImageAttribute::BaseViewpointNum, TagValueType::LONG, 1 },
        { "ConvergenceAngle", MpImageAttribute::ConvergenceAngle, TagValueType::SRATIONAL, 1 },
        { "BaselineLength", MpImageAttribute::BaselineLength, TagValueType::RATIONAL, 1 },
        { "VerticalDivergence", MpImageAttribute::VerticalDivergence, TagValueType::SRATIONAL, 1 },
        { "AxisDistance_X", MpImageAttribute::AxisDistance_X, TagValueType::SRATIONAL, 1 },
        { "AxisDistance_Y", MpImageAttribute::AxisDistance_Y, TagValueType::SRATIONAL, 1 },
        { "AxisDistance_Z", MpImageAttribute::AxisDistance_Z, TagValueType::SRATIONAL, 1 },
        { "YawAngle", MpImageAttribute::YawAngle, TagValueType::SRATIONAL, 1 },
        { "PitchAngle", MpImageAttribute::PitchAngle, TagValueType::SRATIONAL, 1 },
        { "RollAngle", MpImageAttribute::RollAngle, TagValueType::SRATIONAL, 1 },
    };

    MpImageAttributeMap attributeMap;

    for (size_t i = 0; i < sizeof(validAttributes)/sizeof(validAttributes[0]); i++) {
        const ValidAttribute &attribute = validAttributes[i];

        // Find the entry
        MpRawIfd::ConstIterator iter = ifd.constFind(attribute.code);
        if (iter == ifd.constEnd()) {
            continue;
        }

        // Validate type
        quint16 type = iter.value().first;
        if (type != attribute.type) {
            throw Exception(QStringLiteral("'%1' is not of expected type %2 (actual type: %3)").arg(attribute.name).arg(attribute.type).arg(type));
        }

        switch (type) {
            case TagValueType::UNDEFINED: {
                copyAttributeValue<quint8>(attribute.name, attribute.code, attribute.count, iter.value().second, attributeMap);
                break;
            }
            case TagValueType::LONG: {
                copyAttributeValue<quint32>(attribute.name, attribute.code, attribute.count, iter.value().second, attributeMap);
                break;
            }
            case TagValueType::RATIONAL: {
                copyAttributeValue<MpRational>(attribute.name, attribute.code, attribute.count, iter.value().second, attributeMap);
                break;
            }
            case TagValueType::SRATIONAL: {
                copyAttributeValue<MpSRational>(attribute.name, attribute.code, attribute.count, iter.value().second, attributeMap);
                break;
            }
            default: {
                qCWarning(debugMpo) << "Unhandled attribute type:" << type << "for tag" << attribute.name;
                break;
            }
        }
    }

    return attributeMap;
}


// *********************************************************************
// *                    Individual MP Image entry                      *
// *********************************************************************
static QDataStream& operator >> (QDataStream &stream, MpImageInfo &entry)
{
    // Individual Image Attribute (4-byte)
    quint32 attribute;
    stream >> attribute;

    entry.dependentParent = attribute & (1 << 31); // Byte 31
    entry.dependentChild = attribute & (1 << 30); // Byte 30
    entry.representative = attribute & (1 << 29); // Byte 29

    entry.imageFormat = (attribute & 0x07FFFFFF) >> 24; // Bytes 26-24

    entry.imageType = attribute & 0x00FFFFFF; // Lowest 24 bytes (23-0)

    // Individual Image Size (4-byte)
    stream >> entry.dataSize;

    // Individual Image Data Offset (4-byte)
    stream >> entry.dataOffset;

    // Dependent image 1 Entry Number (2-byte)
    stream >> entry.dependentEntry1;

    // Dependent image 2 Entry Number (2-byte)
    stream >> entry.dependentEntry2;

    return stream;
}


} // Mpo
