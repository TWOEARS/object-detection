#include "acobjectdetection.h"

#include "objectdetection_c_types.h"

#include <stdio.h>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 

#include "misc.h"
#include "find_object_2d.h"

#define TRUE    1
#define FALSE   0
using namespace cv;
using namespace std;
/* --- Task FindObjects ------------------------------------------------- */

//std::vector<char *> objectNames;
uint32_t NobjectNames=0;
char **objectNames, **tmpobjectNames;


objectsData* models;
int numObj;

/* --- Activity Start --------------------------------------------------- */

/** Codel InitStart of activity Start.
 *
 * Triggered by objectdetection_start.
 * Yields to objectdetection_exec, objectdetection_ether.
 */
genom_event
InitStart(const char *objectPath, genom_context self)
{
    int i, j, count;
    char *inputPath=NULL;
    char *tmpName=NULL;
    char *filePath=NULL;
    char *tmpTxt;
    char tmp[150];
    size_t len;
    DIR *dir;
    struct dirent *ent;
    FILE *pFile;
    int *tmpNum, *auxNum, NtmpNum=0;

    uint32_t NobjectNames=0;
    char **objectNames, **tmpobjectNames;

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

    //Removes temporarly files (*~) that might be in the objects's folder.
    tmpTxt = (char *) malloc(512*sizeof(char));    //500 MAX number of characters for path.
    sprintf(tmpTxt, "rm %s*~", inputPath);
    printf("Trying to remove temporarly files: %s\n", tmpTxt);
    system(tmpTxt);
    free(tmpTxt);

    if((dir = opendir(inputPath)) != NULL)
    {
        while((ent=readdir(dir)) != NULL)
        {
            //printf("%s\n", ent->d_name);
            if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
            {
                if(NobjectNames == 0)
                {
                    NobjectNames++;
                    objectNames = (char **) malloc(NobjectNames*sizeof(char*));
                    objectNames[0] = (char *) malloc((strlen(ent->d_name)+1)*sizeof(char));
                    strcpy(objectNames[0], ent->d_name);
                }
                else
                {
                    //Save current names to a tmp array.
                    tmpobjectNames = (char **) malloc(NobjectNames*sizeof(char*));
                    for(i=0; i<NobjectNames; i++)
                    {
                        tmpobjectNames[i] = (char *) malloc((strlen(objectNames[i])+1)*sizeof(char));
                        strcpy(tmpobjectNames[i], objectNames[i]);
                    }

                    //Deallocate 'old' names array.
                    for(i=0; i<NobjectNames; i++)
                        free(objectNames[i]);
                    free(objectNames);
                    NobjectNames++;
                    //Allocate names array with one more (new) element.
                    objectNames = (char **) malloc((NobjectNames+1)*sizeof(char*));
                    //Copy elements from 'old' elements in tmp to the array.
                    for(i=0; i<NobjectNames-1; i++)
                    {
                        objectNames[i] = (char *) malloc((strlen(tmpobjectNames[i])+1)*sizeof(char));
                        strcpy(objectNames[i], tmpobjectNames[i]);
                    }
                    //Copy new element to array.
                    objectNames[i] = (char *) malloc((strlen(ent->d_name)+1)*sizeof(char));
                    strcpy(objectNames[i], ent->d_name);
                    for(i=0; i<NobjectNames-1; i++)
                        free(tmpobjectNames[i]);
                    free(tmpobjectNames);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        printf("Directory [%s] could not be opened.\n", inputPath);
        return objectdetection_ether;
    }

    models = (objectsData *) malloc(NobjectNames*sizeof(struct objectsData));
    //Copy file name as "model's name"
    numObj = NobjectNames;
    for (i=0; i<numObj; i++)
    {
        tmpName = (char *) malloc((strlen(objectNames[i])+1)*sizeof(char));
        strcpy(tmpName, objectNames[i]);
        tmpName[strlen(tmpName)-4] = '\0';  //removes .txt
        models[i].name = (char *) malloc((strlen(tmpName)+1)*sizeof(char));
        strcpy(models[i].name, tmpName);
    }

    filePath = NULL;
    // Retrieve file names to struct.
    filePath = NULL;
    for (i=0; i<numObj; i++)
    {
        filePath = (char *) malloc((strlen(inputPath)+strlen(models[i].name)+4+1)*sizeof(char));    //+4 to add .txt and +1 for '\0'.
        sprintf(filePath, "%s%s.txt", inputPath, models[i].name);

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
            {
                tmp[--len] = '\0';
                if(NtmpNum == 0)
                {
                    NtmpNum++;
                    tmpNum = (int *) malloc(sizeof(int));
                    tmpNum[0] = atoi(tmp);
                }
                else
                {
                    //Save current numers to a tmp array.
                    auxNum = (int *) malloc(NtmpNum*sizeof(int));
                    for(j=0; j<NtmpNum; j++)
                        auxNum[j] = tmpNum[j];

                    //Deallocate 'old' tmpNum array.
                    free(tmpNum);
                    NtmpNum++;
                    //Allocate tmpNum array with one more (new) element.
                    tmpNum = (int *) malloc(NtmpNum*sizeof(int));
                    //Copy elements from 'old' array to the new one.
                    for(j=0; j<NtmpNum-1; j++)
                        tmpNum[j] = auxNum[j];
                    //Copy new element to array.
                    tmpNum[j] = atoi(tmp);
                    free(auxNum);
                }
            }
        }

        models[i].length = NtmpNum;
        NtmpNum = 0;
        models[i].buffer = (int *) malloc(models[i].length*sizeof(int));
        for(j=0; j<models[i].length; j++)
            models[i].buffer[j] = tmpNum[j];

        models[i].ID = i;
        models[i].Nbounding = 0;

        fclose(pFile);   
        free(filePath);
    }
    

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
    int i, j, k, l;
    float objectWidth, objectHeight;
    cv::Mat frame;
    cv::Mat cvHomography(3, 3, CV_32F);
    std::vector<Rect> bounding;
    Rect object, R1, R2;
    Rect *tmpBounding;
    cv::namedWindow("output", cv::WINDOW_AUTOSIZE);

    Camera->read(self);
    if(Camera->data(self) != NULL)
    {
        frame = Mat(Camera->data(self)->height, Camera->data(self)->width,CV_8UC3, Camera->data(self)->data._buffer);
        cv::cvtColor(frame, frame, CV_RGB2BGR);

        inObjects->read(self);
        if(inObjects->data(self) != NULL)
        {
            find_object(frame, inObjects, models, numObj, self);
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
        NobjectNames=0;
        free(objectNames);
        return objectdetection_ether;
    }
}
