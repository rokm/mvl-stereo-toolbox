/*
 * MVL Stereo Toolbox: main
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

#include "toolbox.h"


int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    qInfo() << qPrintable(QString("MVL Stereo Toolbox v.%1").arg(PROJECT_VERSION));
    qInfo() << qPrintable(QString("(C) 2013-%1 Rok Mandeljc <rok.mandeljc@gmail.com>\n").arg(QDate::currentDate().year()));

    MVL::StereoToolbox::GUI::Toolbox toolbox;
    toolbox.show();

    return app.exec();
}
