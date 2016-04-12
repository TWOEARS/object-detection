#include "acobjectdetection.h"

#include "objectdetection_c_types.h"

#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 


using namespace cv;
using namespace std;
/* --- Task FindObjects ------------------------------------------------- */

std::vector<cv::Point2f> inPts, outPts;
/* --- Activity Start --------------------------------------------------- */

/** Codel InitStart of activity Start.
 *
 * Triggered by objectdetection_start.
 * Yields to objectdetection_start, objectdetection_ether.
 */
genom_event
InitStart(const objectdetection_Camera *Camera,
          const objectdetection_inObjects *inObjects,
          genom_context self)
{
    int i, j;
    float objectWidth, objectHeight;
    cv::Mat frame;
    cv::Mat cvHomography(3, 3, CV_32F);
    std::vector<Rect> bounding;
    Rect object;

    cv::namedWindow("output", cv::WINDOW_AUTOSIZE);
    

    Camera->read(self);
    if(Camera->data(self) != NULL)
    {
        frame = Mat(Camera->data(self)->height, Camera->data(self)->width,CV_8UC3, Camera->data(self)->data._buffer);
        cv::cvtColor(frame, frame, CV_RGB2BGR);

        inObjects->read(self);
        if(inObjects->data(self) != NULL)
        {
            printf("Data size: %d\n", inObjects->data(self)->data._length);

            for(i=0; i<(inObjects->data(self)->data._length/12); i++)
            {
                objectWidth = inObjects->data(self)->data._buffer[12*i+1];
                objectHeight = inObjects->data(self)->data._buffer[12*i+2];

		        // Find corners OpenCV
		        cvHomography.at<float>(0,0) = inObjects->data(self)->data._buffer[12*i+3];
		        cvHomography.at<float>(1,0) = inObjects->data(self)->data._buffer[12*i+4];
		        cvHomography.at<float>(2,0) = inObjects->data(self)->data._buffer[12*i+5];
		        cvHomography.at<float>(0,1) = inObjects->data(self)->data._buffer[12*i+6];
		        cvHomography.at<float>(1,1) = inObjects->data(self)->data._buffer[12*i+7];
		        cvHomography.at<float>(2,1) = inObjects->data(self)->data._buffer[12*i+8];
		        cvHomography.at<float>(0,2) = inObjects->data(self)->data._buffer[12*i+9];
		        cvHomography.at<float>(1,2) = inObjects->data(self)->data._buffer[12*i+10];
		        cvHomography.at<float>(2,2) = inObjects->data(self)->data._buffer[12*i+11];
		        inPts.push_back(cv::Point2f(0,0));
		        inPts.push_back(cv::Point2f(objectWidth,0));
		        inPts.push_back(cv::Point2f(0,objectHeight));
		        inPts.push_back(cv::Point2f(objectWidth,objectHeight));
		        cv::perspectiveTransform(inPts, outPts, cvHomography);

                printf("Object %d detected, CV corners at (%f,%f) (%f,%f) (%f,%f) (%f,%f)\n",
						    (int) inObjects->data(self)->data._buffer[12*i],
						    outPts.at(0).x, outPts.at(0).y,
						    outPts.at(1).x, outPts.at(1).y,
						    outPts.at(2).x, outPts.at(2).y,
						    outPts.at(3).x, outPts.at(3).y);
                bounding.push_back(Rect(outPts.at(0).x,outPts.at(0).y,outPts.at(3).x-outPts.at(0).x,outPts.at(3).y-outPts.at(0).y));
                cv::rectangle(frame, bounding.at(i), cv::Scalar(0, 0, 255));
            }

            //Find where the rectangles overlap and consider that the position of the object.
            printf("Total of bounding boxes: %d\n", bounding.size());
            if(bounding.size()>0)
            {
                if(bounding.size() == 1)
                    object = bounding.at(0);
                if(bounding.size() == 2)
                {
                    object = bounding.at(0) & bounding.at(1);
                        if(!(object.area()>0))
                            object = Rect(0,0,0,0);
                    
                }
                if(bounding.size()>2)
                {
                    
                }
            }
            if(object.area()>0)
                cv::rectangle(frame, object, cv::Scalar(0, 255, 0));

            printf("\n");
        }
        cv::imshow("output", frame);
    }

    if(cv::waitKey(30) == -1)
    {
        return objectdetection_start;
    }
    else
    {
        frame.release();
        cvHomography.release();
        return objectdetection_ether;
    }
}
