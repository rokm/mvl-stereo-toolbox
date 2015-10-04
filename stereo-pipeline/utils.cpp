/*
 * Stereo Pipeline: utility functions
 * Copyright (C) 2014-2015 Rok Mandeljc
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


#include <stereo-pipeline/utils.h>


namespace MVL {
namespace StereoToolbox {
namespace Utils {


// *********************************************************************
// *                              Helpers                              *
// *********************************************************************
QString cvDepthToString (int depth)
{
    switch (depth) {
        case CV_8U: return "CV_8U";
        case CV_8S: return "CV_8S";
        case CV_16U: return "CV_16U";
        case CV_16S: return "CV_16S";
        case CV_32S: return "CV_32S";
        case CV_32F: return "CV_32F";
        case CV_64F: return "CV_64F";
        default: return "unk";
    }
}


// *********************************************************************
// *       Functions for dumping/loading matrix to a binary file       *
// *********************************************************************
static QDataStream &operator << (QDataStream &stream, const cv::Mat &matrix)
{
    stream << (quint32)matrix.cols; // Width
    stream << (quint32)matrix.rows; // Height
    stream << (quint16)matrix.channels(); // Number of channels
    stream << (quint16)matrix.depth(); // Depth

    for (int y = 0; y < matrix.rows; y++) {
        for (int x = 0; x < matrix.cols; x++) {
            switch (matrix.type()) {
                case CV_8UC1: {
                    const unsigned char &entry = matrix.at<unsigned char>(y, x);
                    stream << entry;
                    break;
                }
                case CV_16SC1: {
                    const short &entry = matrix.at<short>(y, x);
                    stream << entry;
                    break;
                }
                case CV_32SC1: {
                    const int &entry = matrix.at<int>(y, x);
                    stream << entry;
                    break;
                }
                case CV_32FC1: {
                    const float &entry = matrix.at<float>(y, x);
                    stream << entry;
                    break;
                }
                case CV_32FC3: {
                    const cv::Vec3f &entry = matrix.at<cv::Vec3f>(y, x);
                    stream << entry[0] << entry[1] << entry[2];
                    break;
                }
                default: {
                    throw QString("Unhandled matrix format %1!").arg(matrix.type());
                }
            }
        }
    }

    return stream;
}

void writeMatrixToBinaryFile (const cv::Mat &matrix, const QString &fileName, bool compress)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        throw QString("Failed to open file!");
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Signature - binary matrix dump
    stream << (quint8)'B';
    stream << (quint8)'M';
    stream << (quint8)'D';
    stream << (quint8)(compress ? 'C' : ' ');

    if (!compress) {
        // Dump matrix directly to output stream
        stream << matrix;
    } else {
        // Dump matrix to temporary stream, and compress the underlying
        // byte array
        QByteArray tmpData;
        QDataStream tmpStream(&tmpData, QIODevice::WriteOnly);
        tmpStream.setVersion(QDataStream::Qt_5_0);
        tmpStream.setByteOrder(QDataStream::LittleEndian);
        tmpStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        tmpStream << matrix;

        stream << qCompress(tmpData);
    }
}


static QDataStream &operator >> (QDataStream &stream, cv::Mat &matrix)
{
    quint32 cols, rows;
    quint16 channels, depth;

    stream >> cols; // Width
    stream >> rows; // Height
    stream >> channels; // Number of channels
    stream >> depth; // Depth

    matrix.create(rows, cols, CV_MAKETYPE(depth, channels));

    for (int y = 0; y < matrix.rows; y++) {
        for (int x = 0; x < matrix.cols; x++) {
            switch (matrix.type()) {
                case CV_8UC1: {
                    unsigned char &entry = matrix.at<unsigned char>(y, x);
                    stream >> entry;
                    break;
                }
                case CV_16SC1: {
                    short &entry = matrix.at<short>(y, x);
                    stream >> entry;
                    break;
                }
                case CV_32SC1: {
                    int &entry = matrix.at<int>(y, x);
                    stream >> entry;
                    break;
                }
                case CV_32FC1: {
                    float &entry = matrix.at<float>(y, x);
                    stream >> entry;
                    break;
                }
                case CV_32FC3: {
                    cv::Vec3f &entry = matrix.at<cv::Vec3f>(y, x);
                    stream >> entry[0];
                    stream >> entry[1];
                    stream >> entry[2];
                    break;
                }
                default: {
                    throw QString("Unhandled matrix format %1!").arg(matrix.type());
                }
            }
        }
    }

    return stream;
}

void readMatrixFromBinaryFile (cv::Mat &matrix, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        throw QString("Failed to open file!");
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Read and validate signature
    quint8 signature[4];
    stream >> signature[0];
    stream >> signature[1];
    stream >> signature[2];
    stream >> signature[3];

    if (signature[0] != 'B' || signature[1] != 'M' || signature[2] != 'D' || (signature[3] != ' ' && signature[3] != 'C')) {
        throw QString("Invalid binary matrix file!!");
    }

    if (signature[3] == ' ') {
        stream >> matrix;
    } else {
        QByteArray compressedData;
        stream >> compressedData;

        QByteArray tmpData = qUncompress(compressedData);
        QDataStream tmpStream(&tmpData, QIODevice::ReadOnly);
        tmpStream.setVersion(QDataStream::Qt_5_0);
        tmpStream.setByteOrder(QDataStream::LittleEndian);
        tmpStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        tmpStream >> matrix;
    }
}


// *********************************************************************
// *                 Additional visualization functions                *
// *********************************************************************
template <typename TYPE>
void __createColorCodedDisparityCpu (const cv::Mat &disparity, cv::Mat &image, int numLevels)
{
    unsigned int H;
    const float S = 1;
    const float V = 1;

    unsigned int hi;
    float f, p, q, t;

    float x, y, z;
    unsigned char b, g, r;

    for (int i = 0; i < disparity.rows; i++) {
        const TYPE *disparity_ptr = disparity.ptr<TYPE>(i);
        for (int j = 0; j < disparity.cols; j++) {
            H = ((numLevels - disparity_ptr[j]) * 240)/numLevels;

            hi = (H/60) % 6;
            f = H/60.f - H/60;
            p = V * (1 - S);
            q = V * (1 - f * S);
            t = V * (1 - (1 - f) * S);

            switch (hi) {
                case 0: {
                    // R = V, G = t, B = p
                    x = p;
                    y = t;
                    z = V;
                    break;
                }
                case 1: {
                    // R = q, G = V, B = p
                    x = p;
                    y = V;
                    z = q;
                    break;
                }
                case 2: {
                    // R = p, G = V, B = t
                    x = t;
                    y = V;
                    z = p;
                    break;
                }
                case 3: {
                    // R = p, G = q, B = V
                    x = V;
                    y = q;
                    z = p;
                    break;
                }
                case 4: {
                    // R = t, G = p, B = V
                    x = V;
                    y = p;
                    z = t;
                    break;
                }
                case 5: {
                    // R = V, G = p, B = q
                    x = q;
                    y = p;
                    z = V;
                    break;
                }
            }

            b = cv::saturate_cast<unsigned char>(x*255);
            g = cv::saturate_cast<unsigned char>(y*255);
            r = cv::saturate_cast<unsigned char>(z*255);

            image.at<cv::Vec3b>(i, j) = cv::Vec3b(b, g, r);
        }
    }
}

void createColorCodedDisparityCpu (const cv::Mat &disparity, cv::Mat &image, int numLevels)
{
    image.create(disparity.rows, disparity.cols, CV_8UC3);

    switch (disparity.type()) {
        case CV_8UC1: {
            return __createColorCodedDisparityCpu<unsigned char>(disparity, image, numLevels);
        }
        case CV_16SC1: {
            return __createColorCodedDisparityCpu<short>(disparity, image, numLevels);
        }
        case CV_32SC1: {
            return __createColorCodedDisparityCpu<int>(disparity, image, numLevels);
        }
        case CV_32FC1: {
            return __createColorCodedDisparityCpu<float>(disparity, image, numLevels);
        }
        default: {
            throw QString("Unhandled disparity format %1!").arg(disparity.type());
        }
    }
}

void createAnaglyph (const cv::Mat &left, const cv::Mat &right, cv::Mat &anaglyph)
{
    // Split left and right image into channels - BGR!
    cv::Mat leftChannels[3];
    cv::Mat rightChannels[3];

    cv::split(left, leftChannels);
    cv::split(right, rightChannels);

    // Construct anaglyph
    cv::Mat anaglyphChannels[3];
    anaglyphChannels[0] = rightChannels[0]; // B
    anaglyphChannels[1] = rightChannels[1]; // G
    anaglyphChannels[2] = leftChannels[2]; // R

    cv::merge(anaglyphChannels, 3, anaglyph);
}


// *********************************************************************
// *                          PCD file export                          *
// *********************************************************************
void writePointCloudToPcdFile (const cv::Mat &image, const cv::Mat &points, const QString &fileName, bool binary)
{
    // Validate input data
    if (image.rows*image.cols != points.rows*points.cols) {
        throw QString("Size mismatch between image and points matrices!");
    }

    // Count valid points
    int validPoints = 0;
    for (int y = 0; y < points.rows; y++) {
        const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);
        for (int x = 0; x < points.cols; x++) {
            const cv::Vec3f &xyz = pointsPtr[x];
            if (std::isfinite(xyz[2])) {
                validPoints++;
            }
        }
    }

    // Open file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        throw QString("Failed to open file!");
    }

    // Prepare ASCII header
    QString header = QString(
        "# .PCD v0.7 - Point Cloud Data file format\n"
        "VERSION 0.7\n"
        "FIELDS x y z rgb\n"
        "SIZE 4 4 4 4\n"
        "TYPE F F F F\n"
        "COUNT 1 1 1 1\n"
        "WIDTH %1\n"
        "HEIGHT %2\n"
        "VIEWPOINT 0 0 0 1 0 0 0\n"
        "POINTS %3\n"
        "DATA %4\n").arg(validPoints).arg(1).arg(validPoints).arg(binary ? "binary" : "ascii");

    if (binary) {
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_0);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // Write header
        QByteArray headerBytes = header.toLatin1();
        stream.writeRawData(headerBytes.data(), headerBytes.size());

        for (int y = 0; y < image.rows; y++) {
            const cv::Vec3b *imagePtr = image.ptr<cv::Vec3b>(y);
            const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);
            for (int x = 0; x < image.cols; x++) {
                const cv::Vec3b &bgr = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                if (!std::isfinite(xyz[2])) {
                    continue;
                }

                // Convert RGB to PCL floating-point representation
                union {
                    unsigned int i;
                    float f;
                } rgb;

                rgb.i = static_cast<unsigned int>(bgr[2]) << 16 |
                        static_cast<unsigned int>(bgr[1]) << 8 |
                        static_cast<unsigned int>(bgr[0]);

                // Store
                stream << xyz[0] << xyz[1] << xyz[2] << rgb.f;
            }
        }

    } else {
        QTextStream stream(&file);
        stream.setRealNumberPrecision(8);

        stream << header;

        for (int y = 0; y < image.rows; y++) {
            const cv::Vec3b *imagePtr = image.ptr<cv::Vec3b>(y);
            const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);
            for (int x = 0; x < image.cols; x++) {
                const cv::Vec3b &bgr = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                if (!std::isfinite(xyz[2])) {
                    continue;
                }

                // Convert RGB to PCL floating-point representation
                union {
                    unsigned int i;
                    float f;
                } rgb;

                rgb.i = static_cast<unsigned int>(bgr[2]) << 16 |
                        static_cast<unsigned int>(bgr[1]) << 8 |
                        static_cast<unsigned int>(bgr[0]);

                // Store
                stream << xyz[0] << " " << xyz[1] << " " << xyz[2] << " " << rgb.f << "\n";
            }
        }
    }
}

} // Utils
} // StereoToolbox
} // MVL

