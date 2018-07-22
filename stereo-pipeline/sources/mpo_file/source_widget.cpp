/*
 * MPO File Source: source widget
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

#include "source_widget.h"
#include "source.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceMpoFile {


SourceWidget::SourceWidget (Source *source, QWidget *parent)
    : QWidget(parent),
      source(source)
{
    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QPushButton *button;
    QLineEdit *lineEdit;
    QFrame *line;
    QHBoxLayout *hbox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>MPO source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    baseLayout->addWidget(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    baseLayout->addWidget(line);

    // Scrollable area with layout
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(new QWidget(this));

    baseLayout->addWidget(scrollArea);

    QVBoxLayout *layout = new QVBoxLayout(scrollArea->widget());

    // MPO file
    hbox = new QHBoxLayout();
    hbox->setContentsMargins(0, 0, 0, 0);

    tooltip = "MPO file path.";

    label = new QLabel("MPO file: ", this);
    label->setToolTip(tooltip);

    hbox->addWidget(label);

    lineEdit = new QLineEdit(this);
    lineEditMpoFile = lineEdit;

    connect(lineEditMpoFile, &QLineEdit::returnPressed, this, [this] () {
        if (lineEditMpoFile->text() != mpoFilename) {
            mpoFilename = lineEditMpoFile->text();
            emit mpoFileLoadRequested(mpoFilename);
        }
    });
    connect(this, &SourceWidget::mpoFileLoadRequested, source, &Source::openMpoFile, Qt::QueuedConnection); // A two-piece connection due to different thread affinity

    hbox->addWidget(lineEdit);

    button = new QPushButton("Browse");
    connect(button, &QPushButton::clicked, this, [this] () {
        QString filename = QFileDialog::getOpenFileName(this, "Select MPO file", QString(), "MPO files (*.mpo);; All files (*.*)");
        if (!filename.isEmpty()) {
            mpoFilename = filename;
            lineEditMpoFile->setText(mpoFilename);
            emit mpoFileLoadRequested(mpoFilename); // Use same type of connection as above
        }
    });

    hbox->addWidget(button);

    layout->addLayout(hbox);

    layout->addStretch(1);
}

SourceWidget::~SourceWidget ()
{
}


} // SourceMpoFile
} // Pipeline
} // StereoToolbox
} // MVL
