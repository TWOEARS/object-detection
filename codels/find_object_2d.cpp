/*  Copyright (c) 2016, LAAS/CNRS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "find_object_2d.h"

void find_object(cv::Mat frame, sequence_float Data, objectsData *models, int numObj, genom_context self)
{
    int i, j, k, l;
    float objectWidth, objectHeight;
    cv::Mat cvHomography(3, 3, CV_32F);
    std::vector<Rect> bounding;
    Rect object, R1, R2;
    Rect *tmpBounding;
    std::vector<cv::Point2f> inPts, outPts;

    for(i=0; i<numObj; i++)
        models[i].Nbounding = 0;

    for(i=0; i<(Data._length/12); i++)
    {
        objectWidth = Data._buffer[12*i+1];
        objectHeight = Data._buffer[12*i+2];

        // Find corners OpenCV
        cvHomography.at<float>(0,0) = Data._buffer[12*i+3];
        cvHomography.at<float>(1,0) = Data._buffer[12*i+4];
        cvHomography.at<float>(2,0) = Data._buffer[12*i+5];
        cvHomography.at<float>(0,1) = Data._buffer[12*i+6];
        cvHomography.at<float>(1,1) = Data._buffer[12*i+7];
        cvHomography.at<float>(2,1) = Data._buffer[12*i+8];
        cvHomography.at<float>(0,2) = Data._buffer[12*i+9];
        cvHomography.at<float>(1,2) = Data._buffer[12*i+10];
        cvHomography.at<float>(2,2) = Data._buffer[12*i+11];
        inPts.push_back(cv::Point2f(0,0));
        inPts.push_back(cv::Point2f(objectWidth,0));
        inPts.push_back(cv::Point2f(0,objectHeight));
        inPts.push_back(cv::Point2f(objectWidth,objectHeight));
        cv::perspectiveTransform(inPts, outPts, cvHomography);

        // Find to which model the ID from find_object_2d (/object topic) belongs to.
        for(j=0; j<numObj; j++)
        {
            for(k=0; k<models[j].length; k++)
            {
                if((int) Data._buffer[12*i] == models[j].buffer[k])
                {
                    if(models[j].Nbounding == 0)
                    {
                        models[j].Nbounding++;
                        models[j].bounding = (Rect *) malloc(sizeof(Rect));
                        models[j].bounding[0] = Rect(outPts.at(0).x, outPts.at(0).y, outPts.at(3).x-outPts.at(0).x, outPts.at(3).y-outPts.at(0).y);
                    }
                    else
                    {
                        //Save current rects to a tmp array.
                        tmpBounding = (Rect *) malloc(models[j].Nbounding*sizeof(Rect));
                        for(l=0; l<models[j].Nbounding; l++)
                        {
                            tmpBounding[l] = models[j].bounding[l];
                        }
                        //Deallocate 'old' models[j].bounding array.
                        free(models[j].bounding);
                        models[j].Nbounding++;
                        //Allocate models[j].bounding with one more (new) element/
                        models[j].bounding = (Rect *) malloc(models[j].Nbounding*sizeof(Rect));
                        //Copy elements from 'old' array to the new one.
                        for(l=0; l<models[j].Nbounding-1; l++)
                        {
                            models[j].bounding[l] = tmpBounding[l];
                        }
                        //Copy new element to array.
                        models[j].bounding[l] = Rect(outPts.at(0).x, outPts.at(0).y, objectWidth, objectHeight);
                    }
                    break;
                }
            }
        }         
    }
    for(i=0; i<numObj; i++)
    {
        if(models[i].Nbounding == 0)
            models[i].found = FALSE;
        else
        {
            models[i].found = TRUE;
            for(j=0; j<models[i].Nbounding; j++)
            {
                bounding.push_back(models[i].bounding[j]);
            }
        }
        object = commonArea(bounding);
        bounding.resize(0);
        if(object.area()>0)
        {
            cv::rectangle(frame, object, cv::Scalar(0, 0, 255));
            cv::putText(frame, models[i].name, cv::Point(object.x,object.y-10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,0));
        }
    }
}
