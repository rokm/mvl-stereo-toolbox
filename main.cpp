/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 
#include "GuiImageSource.h"
#include "GuiCalibration.h"
#include "GuiStereoMethod.h"

#include "StereoPipeline.h"

#include "ImageSourceFile.h"
#include "ImageSourceDC1394.h"

#include "StereoMethodBlockMatching.h"
#include "StereoMethodSemiGlobalBlockMatching.h"
#include "StereoMethodVar.h"

#include "StereoMethodELAS.h"

#include "StereoMethodConstantSpaceBeliefPropagationGPU.h"
#include "StereoMethodBeliefPropagationGPU.h"
#include "StereoMethodBlockMatchingGPU.h"


int main (int argc, char **argv)
{
    QApplication app(argc, argv);
       
    qDebug() << "MVL StereoToolbox v.1.0, (C) 2013 Rok Mandeljc <rok.mandeljc@fe.uni-lj.si>";

    // *** Create pipeline ***
    StereoPipeline *pipeline = new StereoPipeline();

    // *** All available image sources ***
    QList<ImageSource *> sources;

    sources.append(new ImageSourceFile());
    sources.append(new ImageSourceDC1394());

    // *** All available stereo methods ***
    QList<StereoMethod *> methods;
    
    methods.append(new StereoMethodBlockMatching());
    methods.append(new StereoMethodSemiGlobalBlockMatching());
    methods.append(new StereoMethodVar());
    methods.append(new StereoMethodELAS());

    if (cv::gpu::getCudaEnabledDeviceCount()) {
        cv::gpu::setDevice(0);
        
        methods.append(new StereoMethodBlockMatchingGPU());
        methods.append(new StereoMethodBeliefPropagationGPU());
        methods.append(new StereoMethodConstantSpaceBeliefPropagationGPU());
    }

    // *** GUI ***
    GuiImageSource *guiImageSource = new GuiImageSource(pipeline, sources);
    guiImageSource->show();

    GuiStereoMethod *guiStereoMethod = new GuiStereoMethod(pipeline, methods);
    guiStereoMethod->show();
    
    return app.exec();
}
