////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
// SPDX-License-Identifier: MIT
//
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "EALog.h"
#include "GPIOControl.hpp"

// Log tag.
#define TAG "GPIO"

// GPIO directory path.
#define GPIO_DIRPATH "/sys/class/gpio"


namespace earlyapp
{
    // Constructor.
    GPIOControl::GPIOControl(int gpioNumber, unsigned int peakSustainTime)
    {
        std::ostringstream oss;
        // Disregards for wrong GPIO settings.
        if(gpioNumber > 0)
        {
            m_Valid = true;
            m_GPIONumber = gpioNumber;
            oss << "GPIO output to " << gpioNumber;
            LERR_(TAG, oss.str());
        }
        else
        {
            m_Valid = false;
            LINF_(TAG, "Not controlling GPIO.");
        }

        // Peak time. x 1000 to make it ms.
        m_SustainTime = peakSustainTime * 1000;
        oss.str("");
        oss.clear();
        oss << "Peak sustaining time(us): " << m_SustainTime;
        LINF_(TAG, oss.str());
    }

    // Output GPIO.
    bool GPIOControl::output(eGPIOValue highLow)
    {
        std::string strGPIO = std::to_string(m_GPIONumber);
        const char* cstrGPIO = strGPIO.c_str();
        size_t gpioStrLen = strlen(cstrGPIO);
        std::ostringstream oss;

        // Value path.
        int valueFd = open(valuePath()->c_str(), O_WRONLY);
        if(valueFd < 0)
        {
            // Set export / direction path when failed to open the valuePath().
            int exportFd = open(exportPath()->c_str(), O_WRONLY);
            if(exportFd < 0)
            {
                oss << "Failed to open export path:" << exportPath();
                LERR_(TAG, oss.str());
                return false;
            }
            write(exportFd, cstrGPIO, gpioStrLen);
            close(exportFd);

            // set the direction.
            int dirFd = open(directionPath()->c_str(), O_WRONLY);
            if (dirFd < 0)
            {
                oss.str("");
                oss.clear();
                oss << "Failed to open direction:" << directionPath();
                LERR_(TAG, oss.str());
                return false;
            }
            write(dirFd, "out", (size_t)3);
            close(dirFd);

            //Retry set the high low
            valueFd = open(valuePath()->c_str(), O_WRONLY);
            if(valueFd < 0)
            {
                oss.str("");
                oss.clear();
                oss << "Failed to open GPIO value: " << valuePath();
                LERR_(TAG, oss.str());
                return false;
            }
        }

        // Succeed to open the path.
        const char v = (highLow == eGPIOValue::HIGH) ? '1' : '0';
        write(valueFd, &v, (size_t)1);
        close(valueFd);

        return true;
    }

    // Sustain current output for given time.
    void GPIOControl::sustain(void)
    {
        usleep(m_SustainTime);
    }

    // GPIO Output Pattern.
    void GPIOControl::outputPattern(void)
    {
        output(HIGH);
        sustain();
        output(LOW);
    }

    // GPIO export path.
    std::shared_ptr<std::string> GPIOControl::exportPath(void) const
    {
        std::shared_ptr<std::string> r(new std::string(GPIO_DIRPATH "/export"));
        return r;
    }

    // GPIO direction path
    std::shared_ptr<std::string> GPIOControl::directionPath(void) const
    {
        std::ostringstream ss;
        ss << GPIO_DIRPATH;
        ss << "/gpio";
        ss << std::to_string(m_GPIONumber);
        ss << "/direction";
        std::shared_ptr<std::string> r(new std::string(ss.str()));
        return r;
    }

    // GPIO value path.
    std::shared_ptr<std::string> GPIOControl::valuePath(void) const
    {
        std::ostringstream ss;
        ss << GPIO_DIRPATH;
        ss << "/gpio";
        ss << std::to_string(m_GPIONumber);
        ss << "/value";
        std::shared_ptr<std::string> r(new std::string(ss.str()));
        return r;
    }

    /**
       C interfaces.
     */

    // Create GPIO control.
    void* GPIOControl_create(int gpioNumber, unsigned int peakSustainTime)
    {
        return new GPIOControl(gpioNumber, peakSustainTime);
    }

    // Release GPOI control.
    void GPIOControl_release(void* pGPIOClass)
    {
        if(pGPIOClass != NULL)
        {
            delete static_cast<GPIOControl*>(pGPIOClass);
        }
    }

    // Output GPIO pattern.
    void GPIOControl_outputPattern(void* pGPIOClass)
    {
        if(pGPIOClass != NULL)
        {
            static_cast<GPIOControl*>(pGPIOClass)->outputPattern();
        }
    }
} // namespace
