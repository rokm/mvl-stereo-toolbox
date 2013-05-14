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

#include <QtCore>
#include <QtGui>

#include "StereoPipeline.h"
#include "StereoMethodBlockMatching.h"

#include <opencv2/highgui/highgui.hpp>

int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    qDebug() << "MVL StereoToolbox v.1.0, (C) 2013 Rok Mandeljc <rok.mandeljc@fe.uni-lj.si>";
    
    StereoPipeline *pipeline = new StereoPipeline();
    StereoMethodBlockMatching *method = new StereoMethodBlockMatching();
    
   
    cv::Mat imgL = cv::imread("tsukuba/scene1.row3.col3.ppm", 0);
    cv::Mat imgR = cv::imread("tsukuba/scene1.row3.col5.ppm", 0);
    
    cv::imshow("Left", imgL);
    cv::imshow("Right", imgR);

    method->setPreFilterSize(5);
    method->setPreFilterCap(1);
	method->setSADWindowSize(5);
	method->setMinDisparity(0);
	method->setNumDisparities(64);
	method->setTextureThreshold(0);
    method->setUniquenessRatio(0);
    method->setSpeckleWindowSize(0);
	method->setSpeckleRange(0);  

    pipeline->setStereoMethod(method);
  
    pipeline->processImagePair(imgL, imgR);
    
    cv::imshow("Depth", pipeline->getDepthImage());
    
    pipeline->deleteLater();
    
    return app.exec();
}
