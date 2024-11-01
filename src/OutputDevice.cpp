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

#include <string>
#include "EALog.h"
#include "OutputDevice.hpp"

#define TAG "OUTDEV"

namespace earlyapp
{
    DeviceParameter::DeviceParameter(void)
    {
    }

    DeviceParameter::DeviceParameter(const char* filePath)
    {
        setFileToPlay(filePath);
    }

    DeviceParameter::DeviceParameter(const std::string& filePath)
    {
        setFileToPlay(filePath);
    }

    void DeviceParameter::setFileToPlay(const char* filePath)
    {
        m_sFilePath = std::string(filePath);
    }

    void DeviceParameter::setFileToPlay(const std::string& filePath)
    {
        m_sFilePath = filePath;
    }

    // Returns file path.
    std::string& DeviceParameter::fileToPlay(void)
    {
        return m_sFilePath;
    }

    // Initialize.
    void OutputDevice::init(std::shared_ptr<Configuration> pConf)
    {
        std::ostringstream oss;

        int gpioNumber = pConf->gpioNumber();
        unsigned int sustainTime = pConf->gpioSustain();
        if(gpioNumber > 0)
        {
            LINF_(TAG, oss.str());

            oss << "Setting GPIO " << gpioNumber << "with default sleep time";
            m_pGPIOCtrl = new GPIOControl(gpioNumber, sustainTime);
        }
    }

    // Preapare for play.
    void OutputDevice::preparePlay(std::shared_ptr<DeviceParameter> playParam)
    {
        LINF_(TAG, "preparePlay()");
    }

    // Play
    void OutputDevice::play(void)
    {
        LINF_(TAG, "play()");
    }

    // Prepare for stop.
    void OutputDevice::prepareStop(void)
    {
        LINF_(TAG, "prepareStop()");
    }

    // Stop.
    void OutputDevice::stop(void)
    {
        LINF_(TAG, "stop()");
    }

    // Terminate.
    void OutputDevice::terminate(void)
    {
        LINF_(TAG, "terminate()");
    }

    // Destructor.
    OutputDevice::~OutputDevice(void)
    {
        if(m_pGPIOCtrl != nullptr)
        {
            delete m_pGPIOCtrl;
            m_pGPIOCtrl = nullptr;
        }
    }

    // GPIO output KPI measurements.
    void OutputDevice::outputGPIOPattern(void)
    {
        if(m_pGPIOCtrl == nullptr)
        {
            // No GPIO control.
            return;
        }
        m_pGPIOCtrl->outputPattern();
    }

    // Device name.
    const char* OutputDevice::deviceName(void) const
    {
        if(m_pDevName == nullptr)
        {
            return "UNKNOWN";
        }
        return m_pDevName;
    }
} // namespace
