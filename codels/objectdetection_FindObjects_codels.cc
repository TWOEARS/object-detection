#include "acobjectdetection.h"

#include "objectdetection_c_types.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;
/* --- Task FindObjects ------------------------------------------------- */

cv::Mat frame;

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
    cv::namedWindow("output", cv::WINDOW_AUTOSIZE);
    

    Camera->read(self);
    if(Camera->data(self) != NULL)
    {
        frame = Mat(Camera->data(self)->height, Camera->data(self)->width,CV_8UC3, Camera->data(self)->data._buffer);


        cv::imshow("output", frame);
    }

    if(cv::waitKey(30) == -1)
    {
        return objectdetection_start;
    }
    else
    {
        frame.release();
        return objectdetection_ether;
    }
}
