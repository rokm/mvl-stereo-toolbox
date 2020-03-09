/*
 * MVL Stereo Toolbox: calibration wizard: images selection page
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_IMAGES_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_IMAGES_H


#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                    Image selection page: common                   *
// *********************************************************************
class PageImages : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QStringList images READ getImages WRITE setImages NOTIFY imagesChanged)
    Q_PROPERTY(int patternType READ getPatternType WRITE setPatternType)
    Q_PROPERTY(bool liveCapture READ getLiveCapture)

public:
    PageImages (const QString &fieldPrefix, QWidget *parent = nullptr);
    virtual ~PageImages ();

    bool getLiveCapture () const;

protected:
    void addImages ();
    void clearImages ();

    virtual QStringList getImages () const;
    void setImages (const QStringList &filenames);

    void appendImages (const QStringList &filenames);

    int getPatternType () const;
    void setPatternType (int type);

signals:
    void imagesChanged ();

protected:
    QString fieldPrefix;

    // Image sequence
    QGroupBox *groupBoxImages;

    QPushButton *pushButtonAddImages;
    QPushButton *pushButtonClearImages;
    QListWidget *listWidgetImages;

    // Pattern
    QSpinBox *spinBoxPatternWidth;
    QSpinBox *spinBoxPatternHeight;
    QDoubleSpinBox *spinBoxElementSize;
    QComboBox *comboBoxPatternType;
    QSpinBox *spinBoxScaleLevels;
    QDoubleSpinBox *spinBoxScaleIncrement;
};


// *********************************************************************
// *                 Image selection page: single camera               *
// *********************************************************************
class PageSingleCameraImages : public PageImages
{
    Q_OBJECT

public:
    PageSingleCameraImages (QWidget *parent = nullptr);
    virtual ~PageSingleCameraImages ();

    virtual int nextId () const override;
    virtual bool isComplete () const override;
};


// *********************************************************************
// *                  Image selection page: left camera                *
// *********************************************************************
class PageLeftCameraImages : public PageImages
{
    Q_OBJECT

    Q_PROPERTY(bool skipCalibration READ getSkipCalibration WRITE setSkipCalibration)

public:
    PageLeftCameraImages (QWidget *parent = nullptr);
    virtual ~PageLeftCameraImages ();

    virtual int nextId () const override;
    virtual bool isComplete () const override;

    virtual void setVisible (bool visible) override;

    bool getSkipCalibration () const;
    void setSkipCalibration (bool skip);

protected:
    bool skipCalibration;

private:
    QMetaObject::Connection customButtonConnection;
};


// *********************************************************************
// *                 Image selection page: right camera                *
// *********************************************************************
class PageRightCameraImages : public PageImages
{
    Q_OBJECT

    Q_PROPERTY(bool skipCalibration READ getSkipCalibration WRITE setSkipCalibration)

public:
    PageRightCameraImages (QWidget *parent = nullptr);
    virtual ~PageRightCameraImages ();

    virtual void initializePage () override;

    virtual int nextId () const override;
    virtual bool isComplete () const override;

    virtual void setVisible (bool visible) override;

    bool getSkipCalibration () const;
    void setSkipCalibration (bool skip);

protected:
    bool skipCalibration;

private:
    QMetaObject::Connection customButtonConnection;
};


// *********************************************************************
// *                    Image selection page: stereo                   *
// *********************************************************************
class PageStereoImages : public PageImages
{
    Q_OBJECT
    Q_PROPERTY(int imagesOrder READ getImagesOrder WRITE setImagesOrder)

public:
    PageStereoImages (QWidget *parent = nullptr);
    virtual ~PageStereoImages ();

    virtual int nextId () const override;
    virtual bool isComplete () const override;

    virtual void initializePage () override;

    virtual QStringList getImages () const override;

    enum ImagesOrder {
        Interleaved,
        Grouped
    };

    int getImagesOrder () const;
    void setImagesOrder (int order);

protected:
    void updateImageEntries ();

protected:
    QRadioButton *radioButtonInterleaved;
    QRadioButton *radioButtonGrouped;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
