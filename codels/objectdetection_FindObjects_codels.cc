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

objectsData* modelsL, *modelsR;
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

    modelsL = (objectsData *) malloc(NobjectNames*sizeof(struct objectsData));
    //Copy file name as "model's name"
    numObj = NobjectNames;
    for (i=0; i<numObj; i++)
    {
        tmpName = (char *) malloc((strlen(objectNames[i])+1)*sizeof(char));
        strcpy(tmpName, objectNames[i]);
        tmpName[strlen(tmpName)-4] = '\0';  //removes .txt
        modelsL[i].name = (char *) malloc((strlen(tmpName)+1)*sizeof(char));
        strcpy(modelsL[i].name, tmpName);
    }

    filePath = NULL;
    // Retrieve file names to struct.
    filePath = NULL;
    for (i=0; i<numObj; i++)
    {
        filePath = (char *) malloc((strlen(inputPath)+strlen(modelsL[i].name)+4+1)*sizeof(char));    //+4 to add .txt and +1 for '\0'.
        sprintf(filePath, "%s%s.txt", inputPath, modelsL[i].name);

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

        modelsL[i].length = NtmpNum;
        NtmpNum = 0;
        modelsL[i].buffer = (int *) malloc(modelsL[i].length*sizeof(int));
        for(j=0; j<modelsL[i].length; j++)
            modelsL[i].buffer[j] = tmpNum[j];

        modelsL[i].ID = i;
        modelsL[i].Nbounding = 0;

        fclose(pFile);   
        free(filePath);
    }

    //Copy the extracted data for modelsR.
    modelsR = (objectsData *) malloc(numObj*sizeof(struct objectsData));
    for(i=0; i<numObj; i++)
    {
        modelsR[i].name = (char *) malloc((strlen(modelsL[i].name)+1)*sizeof(char));
        strcpy(modelsR[i].name, modelsL[i].name);

        modelsR[i].ID = modelsL[i].ID;

        modelsR[i].length = modelsL[i].length;
        modelsR[i].buffer = (int *) malloc(modelsR[i].length*sizeof(int));
        for(j=0; j<modelsR[i].length; j++)
            modelsR[i].buffer[j] = modelsL[i].buffer[j];    
    }
    
    /////////////////////////////////
    printf("Model Left:\n");
    for(i=0; i<numObj; i++)
    {
        printf("\tName: %s\n", modelsL[i].name);
        printf("\t\tID: %d\n", modelsL[i].ID);
        printf("\t\tbuffer[%d]: ", modelsL[i].length);
        for(j=0; j<modelsL[i].length; j++)
            printf("%d ", modelsL[i].buffer[j]);
        printf("\n");
    }
    printf("Model Right:\n");
    for(i=0; i<numObj; i++)
    {
        printf("\tName: %s\n", modelsR[i].name);
        printf("\t\tID: %d\n", modelsR[i].ID);
        printf("\t\tbuffer[%d]: ", modelsR[i].length);
        for(j=0; j<modelsR[i].length; j++)
            printf("%d ", modelsR[i].buffer[j]);
        printf("\n");
    }
    /////////////////////////////////

    return objectdetection_exec;
}

/** Codel ExecStart of activity Start.
 *
 * Triggered by objectdetection_exec.
 * Yields to objectdetection_exec, objectdetection_ether.
 */
genom_event
ExecStart(const objectdetection_CameraL *CameraL,
          const objectdetection_inObjectsL *inObjectsL,
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

    CameraL->read(self);
    if(CameraL->data(self) != NULL)
    {
        frame = Mat(CameraL->data(self)->height, CameraL->data(self)->width,CV_8UC3, CameraL->data(self)->data._buffer);
        cv::cvtColor(frame, frame, CV_RGB2BGR);

        inObjectsL->read(self);
        if(inObjectsL->data(self) != NULL)
        {
            find_object(frame, inObjectsL->data(self)->data, modelsL, numObj, self);
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
            free(modelsL[i].name);
            free(modelsL[i].buffer);
        }

        free(modelsL);
        for(i=0; i<numObj; i++)
        {
            free(modelsR[i].name);
            free(modelsR[i].buffer);
        }

        free(modelsR);
        NobjectNames=0;
        free(objectNames);
        return objectdetection_ether;
    }
}
