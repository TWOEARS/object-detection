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

#include "misc.h"
#include <stdio.h>

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

triangulation_world_coordinates triangulation(double f, double T, int leftX, int leftY, int rightX)
{
    double x, y, z;
    int disparity;
    triangulation_world_coordinates result;

    /*printf("Fx: %f\n", f);
    printf("T: %f\n", T);
    printf("leftX: %d\n", leftX);
    printf("leftY: %d\n", leftY);
    printf("rightX: %d\n", rightX);*/

    disparity = rightX - leftX;
    //printf("disparity: %d\n", disparity);
    z = (float) ((f*T) / disparity);
    x = (float) ((leftX*z) / f);
    y = (float) ((leftY*z) / f);

    //printf("triangulation: %f %f %f\n", x, y, z);
    result.x = (float) x;
    result.y = (float) y;
    result.z = (float) z;

    return result;
}
