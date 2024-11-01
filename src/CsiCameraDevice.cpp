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
#include "CsiCameraDevice.hpp"
#include "Configuration.hpp"

// A log tag for Camera device
#define TAG "CAMERA"

extern int m_CSIEnabled;

namespace earlyapp
{
    /*
      Define a device instance variable.
    */
    CsiCameraDevice* CsiCameraDevice::m_pCDev = nullptr;

    /*
      Destructor.
     */
    CsiCameraDevice::~CsiCameraDevice(void)
    {
        if(m_pCDev != nullptr)
        {
            delete m_pCDev;
        }
    }

    /*
      A static function to get an instance(singleton).
    */
    CsiCameraDevice* CsiCameraDevice::getInstance(void)
    {
        if(m_pCDev == nullptr)
        {
            LINF_(TAG, "Creating a CameraDevice instance");
#ifdef USE_DMESGLOG
	    dmesgLogPrint("EA: Csi CameraDevice getInstance\n");
#endif
            m_pCDev = new CsiCameraDevice();
        }

        return m_pCDev;
    }


    /*
      Intialize
     */
    void CsiCameraDevice::init(std::shared_ptr<Configuration> pConf)
    {
        OutputDevice::init(pConf);
	m_CSIEnabled = -1;
        m_pConf = pConf;

#ifdef USE_DMESGLOG
	dmesgLogPrint("EA: Csi CameraDevice init\n");
#endif

	// Set output width/height with user set values.
        // Set default if not set.
        m_csiParam.ow = m_pConf->displayWidth();
        m_csiParam.oh = m_pConf->displayHeight();
        if((int) m_csiParam.ow == Configuration::DONT_CARE)
            m_csiParam.ow = DEFAULT_CAMERA_WIDTH;
        if((int) m_csiParam.oh == Configuration::DONT_CARE)
            m_csiParam.oh = DEFAULT_CAMERA_HEIGHT;

	/* gpio creation */
        if(m_pConf->gpioNumber() != m_pConf->NOT_SET)
        {
            m_pGPIOClass = GPIOControl_create(m_pConf->gpioNumber(), m_pConf->gpioSustain());
        }

        LINF_(TAG, "CSI Camerea intialized.");
    }

    /*
      Play the video device.
     */
    void CsiCameraDevice::play(void)
    {
        LINF_(TAG, "CSI CameraDevice play");

#ifdef USE_DMESGLOG
	dmesgLogPrint("EA: Csi displayCamera play\n");
#endif
	m_CSIEnabled = 0;
	// Create a thread for the camera dispaly and run.
    m_pThreadGrpCsiRVC.emplace_back(std::bind(&displayCamera, m_csiParam, m_pGPIOClass));
    m_pThreadCsiRVC = &m_pThreadGrpCsiRVC.back();
    }

    /*
      Stop.
    */
    void CsiCameraDevice::stop(void)
    {
        LINF_(TAG, "Stopping camera...");
	if(!m_CSIEnabled)
        {
            CsiStopDisplay(0);
            // Wait for thread join.
            if(!m_pThreadGrpCsiRVC.empty())
            {
                for (auto& thread : m_pThreadGrpCsiRVC) {
                    if (thread.joinable()) {
                        thread.join();
                    }
                }
                m_pThreadGrpCsiRVC.clear();
                m_pThreadCsiRVC = nullptr;
            }
        }
        else
            LINF_(TAG, "Fail Stopping camera...");
    }

    /*
      Terminate.
    */
    void CsiCameraDevice::terminate(void)
    {
        LINF_(TAG, "CameraDevice terminate");
        if(m_pGPIOClass)
        {
            GPIOControl_release(m_pGPIOClass);
        }

    }

    void CsiCameraDevice::displayCamera(set_up m_csiParam, void *GPIOClass)
    {
        LINF_(TAG, "Display loop.");

#ifdef USE_DMESGLOG
	dmesgLogPrint("EA: Csi displayCamera\n");
#endif

        CsiStartDisplay(m_csiParam, GPIOClass, 1);
    }

} // namespace

