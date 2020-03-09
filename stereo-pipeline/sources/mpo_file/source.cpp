/*
 * MPO File Source: source
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

#include "source.h"
#include "source_widget.h"

#include <stereo-pipeline/exception.h>

#include "mpo_file.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceMpoFile {


Source::Source (QObject *parent)
    : QObject(parent), ImagePairSource()
{
}

Source::~Source ()
{
}


// *********************************************************************
// *                     ImagePairSource interface                     *
// *********************************************************************
QString Source::getShortName () const
{
    return "MPO";
}

void Source::getImages (cv::Mat &left, cv::Mat &right) const
{
    // Copy images under lock
    QReadLocker locker(&imagesLock);
    imageLeft.copyTo(left);
    imageRight.copyTo(right);
}

void Source::stopSource ()
{
    // No-op
}

QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


// *********************************************************************
// *                             MPO file                              *
// *********************************************************************
static void loadDisparityMpo (const QString &filename, cv::Mat &imageLeft, cv::Mat &imageRight)
{
    // Load MPO file
    Mpo::MpFile mpo(filename);

    // Gather all disparity images
    struct DisparityImage {
        int idx; // Image index in the list of MPO objects
        int number; // Individual image number
        int baseViewNumber; // Base viewpoint number
    };

    QList<DisparityImage> disparityImages;
    int baseIdx = -1;
    int otherIdx = -1;

    for (int i = 0; i < mpo.getNumberOfImages(); i++) {
        const Mpo::MpImageInfo &info = mpo.getImageInfo(i);

        if (info.imageType != Mpo::MpImageType::DisparityImage) {
            continue;
        }

        DisparityImage entry;

        entry.idx = i;

        // Attributes; we are interested in Individual Number and
        // Base Viewpoint Number
        Mpo::MpImageAttributeMap::ConstIterator iter;
        iter = info.attributes.constFind(Mpo::MpImageAttribute::MPIndividualNum);
        if (iter == info.attributes.constEnd()) {
            throw Exception(QStringLiteral("'MPIndividualNum' attribute missing!"));
        }
        entry.number = iter.value().value<quint32>();

        iter = info.attributes.constFind(Mpo::MpImageAttribute::BaseViewpointNum);
        if (iter == info.attributes.constEnd()) {
            throw Exception(QStringLiteral("'BaseViewpointNum' attribute missing!"));
        }
        entry.baseViewNumber = iter.value().value<quint32>();

        if (entry.number == entry.baseViewNumber && baseIdx == -1) {
            // We found our first base viewpoint
            baseIdx = disparityImages.size();
        }

        disparityImages.append(entry);
    }

    if (disparityImages.size() < 2) {
        throw Exception(QStringLiteral("MPO file contains fewer than 2 disparity images!"));
    }

    if (baseIdx == -1) {
        throw Exception(QStringLiteral("Could not determine base viewpoint!"));
    }

    // Find the first viewpoint that corresponds to our base viewpoint
    for (int i = 0; i < disparityImages.size(); i++) {
        if (i != baseIdx && disparityImages[i].baseViewNumber == disparityImages[baseIdx].number) {
            otherIdx = i;
            break;
        }
    }

    if (otherIdx == -1) {
        throw Exception(QStringLiteral("Could not find other viewpoint!"));
    }

    // Load images
    int idxLeft, idxRight;

    if (disparityImages[baseIdx].number < disparityImages[otherIdx].number) {
        // If base view's number is lower than the other view's, then
        // base view is on the left...
        idxLeft = disparityImages[baseIdx].idx;
        idxRight = disparityImages[otherIdx].idx;
    } else {
        // ... otherwise, it is on the right
        idxRight = disparityImages[baseIdx].idx;
        idxLeft = disparityImages[otherIdx].idx;
    }

    imageLeft = mpo.loadImage(idxLeft);
    imageRight = mpo.loadImage(idxRight);
}


void Source::openMpoFile (const QString &filename)
{
    // Load disparity image pair from MPO
    cv::Mat tmpLeft, tmpRight;
    try {
        loadDisparityMpo(filename, tmpLeft, tmpRight);
    } catch (const std::exception &e) {
        emit error(QStringLiteral("Failed to load disparity MP file: %1").arg(QString::fromStdString(e.what())));
        return;
    }

    // Set images
    QWriteLocker locker(&imagesLock);

    tmpLeft.copyTo(imageLeft);
    tmpRight.copyTo(imageRight);

    locker.unlock();

    emit imagesChanged();
}


} // SourceMpoFile
} // Pipeline
} // StereoToolbox
} // MVL
