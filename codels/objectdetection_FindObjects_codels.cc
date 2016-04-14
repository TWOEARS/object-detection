#include "acobjectdetection.h"

#include "objectdetection_c_types.h"

#include <stdio.h>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 


using namespace cv;
using namespace std;
/* --- Task FindObjects ------------------------------------------------- */

std::vector<cv::Point2f> inPts, outPts;
std::vector<char *> objectNames;

struct objectsData
{
    char* name;
    int ID;
    uint32_t length;
    int* buffer;
    std::vector<Rect> bounding;
};
objectsData* models;
int numObj;

/* --- Activity Start --------------------------------------------------- */

/* Function's Headers*/
Rect commonArea(std::vector<Rect> bounding);
/* Function's Headers*/

/** Codel InitStart of activity Start.
 *
 * Triggered by objectdetection_start.
 * Yields to objectdetection_exec, objectdetection_ether.
 */
genom_event
InitStart(const char *objectPath, genom_context self)
{
    int i, j, count;;
    char *filePath=NULL;
    char *inputPath=NULL;
    char *tmpName=NULL;
    char tmp[150];
    size_t len;
    DIR *dir;
    FILE *pFile;
    struct dirent *ent;
    std::vector<int> tmpNum;

    //Check if path ends with /, if not add it.
    if(objectPath[strlen(objectPath)-1] == '/')
    {
        inputPath = (char *) malloc((strlen(objectPath)+1)*sizeof(char));
        strcpy(inputPath, objectPath);
    }
    else
    {
        inputPath = (char *) malloc((strlen(objectPath)+1)*sizeof(char));
        sprintf(inputPath, "%s/", objectPath);
    }

    if((dir = opendir(inputPath)) != NULL)
    {
        while((ent=readdir(dir)) != NULL)
        {
            //printf("%s\n", ent->d_name);
            if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
            {
                objectNames.push_back(ent->d_name);
                //printf("%s saved [%d]\n", ent->d_name, strlen(ent->d_name));
            }
        }
        closedir(dir);
    }
    else
    {
        printf("Directory [%s] could not be opened.\n", inputPath);
        return objectdetection_ether;
    }
    printf("inputPath: %s\n", inputPath);
    
    models = (objectsData *) malloc(objectNames.size()*sizeof(struct objectsData));
    //Copy file name as "model's name"
    numObj = objectNames.size();
    for (i=0; i<numObj; i++)
    {
        tmpName = (char *) malloc((strlen(objectNames.at(i))+1)*sizeof(char));
        strcpy(tmpName, objectNames.at(i));
        tmpName[strlen(tmpName)-4] = '\0';  //removes .txt
        models[i].name = (char *) malloc((strlen(tmpName)+1)*sizeof(char));
        strcpy(models[i].name, tmpName);
    }

    printf("Names: %d\n", numObj);
    for (i=0; i<numObj; i++)
    {
        printf("Model %d: %s\n", i, models[i].name);
    }
    
    filePath = NULL;
    printf("Files's list:\n");
    // Retrieve file names to struct.
    filePath = NULL;
    for (i=0; i<numObj; i++)
    {
        printf("models[%d].name: %s\n", i,  models[i].name);
        printf("FULL PATH: %s%s\n", inputPath, models[i].name);

        filePath = (char *) malloc((strlen(inputPath)+strlen(models[i].name)+4+1)*sizeof(char));    //+4 to add .txt and +1 for '\0'.
        sprintf(filePath, "%s%s.txt", inputPath, models[i].name);
        printf("filePath: %s\n\n", filePath);

        pFile = fopen(filePath, "r");
        if(pFile==NULL)
        {
            printf("--(!)Error loading file %s\n", filePath); 
            return objectdetection_ether;
        };
        count = 0;
        while(fgets(tmp, 150, pFile) != NULL)
        {
            count++;
            len = strlen(tmp);
            if(len>0 && tmp[len-1]=='\n')
                tmp[--len] = '\0';

            tmpNum.push_back(atoi(tmp));
        }
        models[i].length = tmpNum.size();
        models[i].buffer = (int *) malloc(models[i].length*sizeof(int));
        for(j=0; j<models[i].length; j++)
            models[i].buffer[j] = tmpNum.at(j);

        tmpNum.resize(0);
        fclose(pFile);        
        free(filePath);

    }
    printf("end for\n");

    return objectdetection_exec;
}

/** Codel ExecStart of activity Start.
 *
 * Triggered by objectdetection_exec.
 * Yields to objectdetection_exec, objectdetection_ether.
 */
genom_event
ExecStart(const objectdetection_Camera *Camera,
          const objectdetection_inObjects *inObjects,
          genom_context self)
{
    int i, j;
    float objectWidth, objectHeight;
    cv::Mat frame;
    cv::Mat cvHomography(3, 3, CV_32F);
    std::vector<Rect> bounding;
    Rect object, R1, R2;

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
                //cv::rectangle(frame, bounding.at(i), cv::Scalar(0, 0, 255));
            }

            //Find where the rectangles overlap and consider that the position of the object.
            printf("Total of bounding boxes: %d\n", bounding.size());
            object = commonArea(bounding);
            if(object.area()>0)
                cv::rectangle(frame, object, cv::Scalar(0, 255, 0));

            printf("\n");
        }
        cv::imshow("output", frame);
    }

    if(cv::waitKey(30) == -1)
    {
        return objectdetection_exec;
    }
    else
    {
        frame.release();
        cvHomography.release();
        for(i=0; i<numObj; i++)
        {
            free(models[i].name);
            free(models[i].buffer);
        }

        free(models);
        objectNames.resize(0);
        return objectdetection_ether;
    }
}


/* Functions */
Rect commonArea(std::vector<Rect> bounding)
{
    int i;
    Rect object, R1, R2;

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
            object = bounding.at(0) & bounding.at(1);
            for(i=2; i<bounding.size(); i++)
            {
                R1 = object;
                R2 = bounding.at(i);
                object = R1 & R2;
            }
        }
    }
    return object;
}
