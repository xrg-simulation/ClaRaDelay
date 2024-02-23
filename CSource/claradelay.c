/* BSD 3-Clause License
 *
 * Copyright (c) 2022, XRG Simulation GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "claradelay.h"
#include "External/ModelicaUtilities.h"

//GLOBAL CONSTANT
#define MAX_DELAYSTEPS 300000                               //max size of data array in length

typedef struct DelayValue
{
    double *data;
    double *time;
    int currentStep;
    int lastPossibleStep;
    int latestStep;
} DelayValue;

typedef struct DelayValues
{
    int size;
    DelayValue** delayValues;
} DelayValues;

//GLOBAL VARIABLES
static int totalDelayValues;//............total length of data-array so far, starting with 500
static double epsilonStepTime=1e-10;//.........if a time intervall is smaller than this number it gets saved anyway, even if minStepTime is set (userset)
static int max_DelayValues=500;

//------------------------------------------------------------------------------------------------------//
//------------------    INTERNAL    FUNCTIONS   (NOT    IN  .H-FILE)    --------------------------------//
//------------------------------------------------------------------------------------------------------//
static int getStepForInterpolation(DelayValue * delayData, double delayTime, int startStep)
{
    int i=0;
    int step=0;
    //////////////////////////////////////////////////////////////////////
    //  for-loop iterating downwoards until time from step is less than //
    //  the wanted delay-time. break as soon as step becomes negative   //
    //////////////////////////////////////////////////////////////////////
    for (i = 0; i < startStep; i++)
    {
        step = startStep-i;
        if (step < 0)
        {
            ModelicaFormatError("ERROR: step=%i . step<0", 0);
            return 0;
        }
        if (delayTime >= delayData->time[step])
        {
            return step;
        }
    }
    return 0;
}

static int testDoubleForEquality(double left, double right)
{
    //////////////////////////////////////////////////////////////////
    //  test if two given values vary from each other within the    //
    //  limits of epsilon                                           //
    //////////////////////////////////////////////////////////////////
    return (left < right + epsilonStepTime && left > right - epsilonStepTime);
}

static double interpolate(double time1, double value1, double time2, double value2, double wantedTime)
{
    double result;
    //////////////////////////////////////////////////////////////////
    //  interpolating within given limits a value to a wanted time  //
    //  asserting that time-limits are set properly                 //
    //////////////////////////////////////////////////////////////////
    if (time1 <= wantedTime && time2 >= wantedTime)
    {
        if (testDoubleForEquality(time1, time2) || testDoubleForEquality(time1, wantedTime))
        {
            return value1;
        }
        result = value1 + (value2 - value1) / (time2 - time1) * (wantedTime - time1);
        return result;
    }
    else
    {
        ModelicaFormatError("ERROR: time1<=getValueAtTime AND time2>=getValueAtTime is NOT true\n time1=%f\n value1=%f\n time2=%f\n value2=%f\n getValueAtTime=%f", time1, value1, time2, value2, wantedTime);
        return 0;
    }
}

static int findStepOfTime(DelayValue * delayData, double time, int startStep)
{
    int step;
    int i;
    for(i = 0; i <= startStep; i++)
    {
        //////////////////////////////////////////////////////////////////////
        //  going down from startStep to find a step with smaller or equal  //
        //  time to the given time. this function became necessary since    //
        //  DASSL moves for- und backwards which results in massive inter-  //
        //  polation mistakes and now we're going to rewrite the value at   //
        //  it's very first position instead of appending                   //
        //////////////////////////////////////////////////////////////////////
        step = startStep - i;
        if(testDoubleForEquality(delayData->time[step], time))
        {
            return step;
        }
        else if (delayData->time[step] < time)
        {
            if (delayData->time[step + 1] > time || testDoubleForEquality(time, delayData->time[step + 1])) //strangly a double of test for equality became necessary
            {
                return step + 1;
            }
            else
            {
                ModelicaFormatMessage("WARNING: findStepOfTime(). Wasn't able to find appropriate step for time %f. Overwritten step %i with time %f instead of step %i with time %f\nThis might effect accuracy of your simulation.\n",time, step, delayData->time[step], step+1, delayData
                                      ->time[step+1]);
                return step;
            }

        }
        if(step < 0)
        {
            ModelicaFormatError("findStepOfTime(): step < 0 . Iteration-Error\n");
        }
    }
    ModelicaFormatMessage("findStepOfTime(): Couldn't find appropriate time. Investigated entire stored data.\n");
    return 0;
}

static int insertData(DelayValue * delayData, double time, double value)
{
    int i;
    int insertStep;
    ///////////////////////////////////////////////////////////////////////////////////////
    //  time given by function is smaller than latest saved time, so insertion is needed //
    ///////////////////////////////////////////////////////////////////////////////////////
    insertStep = findStepOfTime(delayData, time, delayData->latestStep);
    if (testDoubleForEquality(time, delayData->time[insertStep]))
    {
        delayData->data[insertStep] = value;
    }
    else
    {
        //////////////////////////////////////////////////////////////////////////
        //  move every time step before insert step one step up and then insert //
        //  "new" time at the step it should be at                              //
        //////////////////////////////////////////////////////////////////////////
        for (i = delayData->currentStep; i > insertStep; i--)
        {
            delayData->data[i] = delayData->data[i - 1];
            delayData->time[i] = delayData->time[i - 1];
        }
        delayData->data[insertStep] = value;
        delayData->time[insertStep] = time;
        return 1;
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------//
//-------------------------------    FUNCTIONS FROM .H-FILE    -----------------------------------------//
//------------------------------------------------------------------------------------------------------//

void * clara_initDelay()
{
    //////////////////////////////////////////////////////////////////////////////////////
    //  the correct way for Modelica to deal with external objects (like this table)    //
    //  is to create a pseudo object, that creates a pointer to the external object.    //
    //  the pointer to the external object is created from an external function and     //
    //  received by Modelica.                                                           //
    //  this is the creation of such a pointer. pointing to the table-struct            //
    //////////////////////////////////////////////////////////////////////////////////////
    DelayValue * ptr = (DelayValue *)malloc(sizeof(DelayValue));
    ptr->time = (double *)malloc(max_DelayValues*sizeof(double));
    ptr->data = (double *)malloc(max_DelayValues*sizeof(double));
    ptr->lastPossibleStep = max_DelayValues;
    ptr->currentStep = -1;
    ptr->latestStep = -1;
    return ptr;
}

void clara_deleteDelay(void *ptr_to_table)
{
    //////////////////////////////////////////////////////////////////////////////
    //  Modelica then also needs a destructor to the pseudo object which frees  //
    //  the data. this is the destructor function.                              //
    //////////////////////////////////////////////////////////////////////////////
    DelayValue * delayData = (DelayValue *)ptr_to_table;
    free(delayData->data);
    free(delayData->time);
    free(delayData);
}

void * clara_initDelayArray(int size)
{
    DelayValues* ptr = (DelayValues*) malloc(sizeof(DelayValues));
    ptr->delayValues = (DelayValue**)malloc(size*sizeof(DelayValue*));
    ptr->size = size;
    for(int i=0;i<size;i++)
    {
        ptr->delayValues[i] = clara_initDelay();
    }
    return ptr;
}

void clara_deleteDelayArray(void *ptr_to_tables)
{
    DelayValues * delayValues = (DelayValues*)ptr_to_tables;
    for(int i=0;i<delayValues->size;i++)
    {
        clara_deleteDelay(delayValues->delayValues[i]);
    }
    free(delayValues->delayValues);
}

void clara_setDelayValue(void * ptr_to_table, double time, double value)
{
    int step;
    DelayValue * delayData = (DelayValue *)ptr_to_table;
    ///////////////////////
    //  safety-requests  //
    ///////////////////////
    if (!ptr_to_table)  //nullptr received
    {
        ModelicaFormatMessage("getDelayValuesAtTimes: Use initDelay function befor call getDelayValuesAtTimes!\n");
        return;
    }
    if (time < 0)       //negative time given
    {
        ModelicaError("ERROR: time<0");
    }
    if (delayData->currentStep < 0) //first value written
    {
        delayData->currentStep = 0;
    }
    //////////////////////////////////////////////////////////
    //  reallocating memory in case of reaching close to    //
    //  the end of current memory                           //
    //////////////////////////////////////////////////////////
    if(delayData->lastPossibleStep - delayData->currentStep <= 10)
    {
        delayData->lastPossibleStep += 500;
        delayData->time = (double*) realloc(delayData->time, delayData->lastPossibleStep*sizeof(double));
        if (!delayData->time) ModelicaFormatError("getDelayID(): out of memory error.\nPossible Solution:\tTry bigger step size, shorter simulation time, bigger interval length or lesser number of intervals!\n");
        delayData->data=(double*) realloc(delayData->data, delayData->lastPossibleStep*sizeof(double));
        if (!delayData->data) ModelicaFormatError("getDelayID(): out of memory error.");
    }
    //////////////////////
    //  safety request  //
    //////////////////////
    if (delayData->currentStep >= delayData->lastPossibleStep)
    {
        ModelicaFormatError("ERROR: currentDelayStep>MAX_DELAYSTEPS\tstep %i", delayData->currentStep);
    }
    //////////////////////////////////////////
    //  saving current time at current step //
    //////////////////////////////////////////
    if (delayData->latestStep >= 0 && testDoubleForEquality(delayData->time[delayData->latestStep], time))  //overwrite latest step if times are equal
    {
        delayData->data[delayData->latestStep] = value;
    }
    else if (delayData->currentStep == 0 || delayData->time[delayData->latestStep] < time)  //append, if everything is allright
    {
        delayData->time[delayData->currentStep] = time;
        delayData->data[delayData->currentStep] = value;
        delayData->latestStep = delayData->currentStep;
        delayData->currentStep++;
    }
    else                                                                                    //else: find step to overwrite and reset list to that step
    {
        step = findStepOfTime(delayData, time, delayData->latestStep);
        delayData->time[step] = time;
        delayData->data[step] = value;
        delayData->latestStep = step;
        delayData->currentStep = step + 1;
        //  the following code does not work! Inserting is not possible since equality isn't properly testable. Also referencing for Modelica does not work then.
        //        delayData->currentStep += insertData(delayData, time, value);
        //        delayData->latestStep = delayData->currentStep - 1;
    }
}

void clara_getDelayValuesAtTimes(void * ptr_to_table, double time, double value,  double wantedDelayTimes[], int getTimes_size, double *result, int result_size)
{
    int* step;
    int lastRoundsStep=-1;
    double value1=0.;
    double time1=0.;
    double value2=0.;
    double time2=0.;
    int i;
    DelayValue * delayData = (DelayValue *)ptr_to_table;

    ///////////////////////
    //  safety-requests  //
    ///////////////////////
    if (!ptr_to_table)
    {
        ModelicaFormatMessage("getDelayValuesAtTimes: Use initDelay function befor call getDelayValuesAtTimes!\n");
        return;
    }
    if (getTimes_size <= 0 || getTimes_size != result_size)
    {
        ModelicaFormatError("getDelayValuesAtTimes(): size error\n");
    }
    /////////////////////////////////////////////////////////////
    //  setting size of step-for-interpolatin-result-array    //
    /////////////////////////////////////////////////////////////
    step = malloc(getTimes_size*sizeof(int));
    if (!step)
    {
        ModelicaFormatError("getDelayValuesAtTimes(): out of memory error\n");
    }
    ///////////////////////////////////
    //  writing values to data set   //
    ///////////////////////////////////
    clara_setDelayValue(delayData, time, value);
    //////////////////////////////////////////
    //  getting-value-for-result-array-loop //
    //////////////////////////////////////////
    for (i = 0; i < getTimes_size; i++)
    {
        //////////////////////
        //  for debugging   //
        //////////////////////
        step[i] = -1;
        //////////////////////////////////////////////////////////////////////////////////
        //  evaluating the result in three cases:                                       //
        //  first case: delayTime is current simulating time . result=current value    //
        //  second case: delayTime is negative .   result= starting value              //
        //  third case: else . result is computed with interpolation                   //
        //////////////////////////////////////////////////////////////////////////////////
        if (testDoubleForEquality(time, wantedDelayTimes[i]))
        {
            result[i] = value;
        }
        else if (wantedDelayTimes[i] < delayData->time[0] && delayData->currentStep >= 0)
        {
            result[i] = delayData->data[0];
        }
        else
        {
            //////////////////////////////////////////////////////////////////////////////////
            //  getting step for interpolation, either initial starting from currentStep    //
            //  or going backwards from there with step from last iteration. step gets      //
            //  saved as lastRoundStep for next round                                       //
            //////////////////////////////////////////////////////////////////////////////////
            if (lastRoundsStep <= 0)
            {
                step[i] = getStepForInterpolation(delayData, wantedDelayTimes[i], delayData->latestStep);
                lastRoundsStep = step[i];
            }
            else
            {
                step[i] = getStepForInterpolation(delayData, wantedDelayTimes[i], lastRoundsStep);
                lastRoundsStep = step[i];
            }
            //////////////////////////////////////////////////////////////////////////////
            //  interpolating either around currentStep and simulating time in case of  //
            //  future-related requests or interpolating around step and follow-up      //
            //////////////////////////////////////////////////////////////////////////////
            if (lastRoundsStep == delayData->latestStep)
            {
                value1 = delayData->data[lastRoundsStep];
                time1 = delayData->time[lastRoundsStep];
                value2 = value;
                time2 = time;
            }
            else
            {
                value1 = delayData->data[lastRoundsStep];
                time1 = delayData->time[lastRoundsStep];
                value2 = delayData->data[lastRoundsStep + 1];
                time2 = delayData->time[lastRoundsStep + 1];
            }
            if (time1 <= wantedDelayTimes[i] && time2 >= wantedDelayTimes[i])
            {
                result[i] = interpolate(time1, value1, time2, value2, wantedDelayTimes[i]);
            }
            else if (time1 >= time)
            {
                result[i] = delayData->data[delayData->latestStep];
            }
            else
            {
                result[i] = interpolate(0, delayData->data[0], time, delayData->data[delayData->latestStep], wantedDelayTimes[i]);
            }
        }
    }
    free(step);
}

double clara_getDelayValuesAtTime(void * ptr_to_table, double time, double value,  double getTime)
{
    //////////////////////////////////////////////////////////////////
    //  call getDelayValuesAtTimes() with "faked" lists, in case //
    //  one just wants to know one value                            //
    //////////////////////////////////////////////////////////////////
    double getTimes[1]={getTime};
    double result = 0.0;
    clara_getDelayValuesAtTimes(ptr_to_table, time, value, getTimes, 1, &result, 1);
    return result;
}

double clara_getDelayValuesAtTimeArray(void * ptr_to_tables, double time, double value, double getTime, int index)
{
    ///////////////////////
    //  safety-requests  //
    ///////////////////////
    if (!ptr_to_tables)
    {
        ModelicaFormatError("getDelayValuesAtTimes: Use initDelay function befor call getDelayValuesAtTimes!\n");
    }
    double result = 0.0;
    DelayValues* delayValues = (DelayValues*) ptr_to_tables;
    if (index - 1 >= delayValues->size)
    {
        ModelicaFormatError("Index %i is out of bound %i", index - 1, delayValues->size);
    }
    DelayValue * ptr = delayValues->delayValues[index - 1];
    result = clara_getDelayValuesAtTime(ptr, time, value, getTime);
    return result;
}
