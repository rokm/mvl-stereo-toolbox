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


// *********************************************************************
// *                 Additional visualization functions                *
// *********************************************************************
void createColorCodedDisparityCpu (const cv::Mat &disparity, cv::Mat &image, int numLevels)
{
    image.create(disparity.rows, disparity.cols, CV_8UC3);

    for (int i = 0; i < disparity.rows; i++) {
        for (int j = 0; j < disparity.cols; j++) {
            unsigned char d = disparity.at<unsigned char>(i, j);

            unsigned int H = ((numLevels - d) * 240)/numLevels;
            const float S = 1;
            const float V = 1;

            unsigned int hi = (H/60) % 6;
            float f = H/60.f - H/60;
            float p = V * (1 - S);
            float q = V * (1 - f * S);
            float t = V * (1 - (1 - f) * S);

            float x, y, z;

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

            unsigned char b = (unsigned char)(std::max(0.0f, std::min(x, 1.0f)) * 255.f);
            unsigned char g = (unsigned char)(std::max(0.0f, std::min(y, 1.0f)) * 255.f);
            unsigned char r = (unsigned char)(std::max(0.0f, std::min(z, 1.0f)) * 255.f);

            image.at<cv::Vec3b>(i, j) = cv::Vec3b(b, g, r);
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


} // Utils
} // StereoToolbox
} // MVL

