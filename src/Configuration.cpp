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

#include <iostream>
#include <string>
#include "EALog.h"
#include "EAVersion.h"
#include "Configuration.hpp"


// A tag for configuration.
#define TAG "CFG"


namespace earlyapp
{
    // Definitions.
    const int Configuration::DONT_CARE = 0;
    const int Configuration::NOT_SET = -1;


    // Default values.
    const char* Configuration::DEFAULT_AUDIO_SPLASHSOUND_PATH = "/usr/share/earlyapp/jingle.wav";
    const char* Configuration::DEFAULT_AUDIO_RVCSOUND_PATH = "/usr/share/earlyapp/beep.wav";
    const char* Configuration::DEFAULT_CAMERA_INPUTSOURCE = "icam";
    const char* Configuration::DEFAULT_VIDEO_SPLASH_PATH = "/usr/share/earlyapp/splash_video.h264";
    const char* Configuration::DEFAULT_CBCDEVICE_PATH = "/dev/cbc-early-signals";
    const char* Configuration::DEFAULT_RESUME_SYNC_PATH = "/usr/share/earlyapp/resume_sync";
    const char* Configuration::DEFAULT_TESTCBCDEVICE_PATH = "";
    const unsigned int Configuration::DEFAULT_DISPLAY_WIDTH = DONT_CARE;
    const unsigned int Configuration::DEFAULT_DISPLAY_HEIGHT = DONT_CARE;
    const int Configuration::DEFAULT_GPIONUMBER = NOT_SET;
    const unsigned int Configuration::DEFAULT_GPIOSUSTAIN = 1;
    const char* Configuration::DEFAULT_USE_GSTREAMER = "false";
    const char* Configuration::DEFAULT_USE_CSICAM = "false";
    const char* Configuration::DEFAULT_GSTCAMCMD = "";


    // Configuration keys.
    const char* Configuration::KEY_BOOTUPSOUND = "bootup-sound";
    const char* Configuration::KEY_RVCSOUND = "rvc-sound";
    const char* Configuration::KEY_CAMERASOURCE = "camera-input";
    const char* Configuration::KEY_SPLASHVIDEO = "splash-video";
    const char* Configuration::KEY_CBCDEVICE = "cbc-device";
    const char* Configuration::KEY_RESUMESYNC = "resume-sync";
    const char* Configuration::KEY_TESTCBCDEVICE = "test-cbc-device";
    const char* Configuration::KEY_DISPLAYWIDTH = "width";
    const char* Configuration::KEY_DISPLAYHEIGHT = "height";
    const char* Configuration::KEY_GPIONUMBER = "gpio-number";
    const char* Configuration::KEY_GPIOSUSTAIN = "gpio-sustain";
    const char* Configuration::KEY_USEGSTREAMER = "use-gstreamer";
    const char* Configuration::KEY_USECSICAM = "use-csicam";
    const char* Configuration::KEY_GSTCAMCMD = "gstcamcmd";



    // Return Configuration object.
    std::shared_ptr<Configuration> Configuration::makeConfiguration(int argc, char** argv)
    {
        std::shared_ptr<Configuration> conf = std::make_shared<Configuration>();

        if(conf != nullptr && conf.get() != nullptr)
        {
            // Initialize program options.
            conf->initProgramOptions(argc, argv);

            return conf;
        }
        return nullptr;
    }

    // Is valid.
    bool Configuration::isValid(void)
    {
        return m_Valid;
    }

    // Print version.
    void Configuration::printVersion(char* programName)
    {
        if(programName != nullptr)
        {
            std::cout << programName << " : ";
        }
        std::cout << "version " << EARLYAPP_PROGRAM_VERSION << std::endl;
    }

    // Audio splash sound path
    const std::string& Configuration::audioSplashSoundPath(void)
    {
        return stringMappedValueOf(Configuration::KEY_BOOTUPSOUND);
    }

    // Audio beep sound path
    const std::string& Configuration::audioRVCSoundPath(void)
    {
        return stringMappedValueOf(Configuration::KEY_RVCSOUND);
    }

    // Camera input source.
    const std::string& Configuration::cameraInputSource(void)
    {
        return stringMappedValueOf(Configuration::KEY_CAMERASOURCE);
    }

    // Video splash path.
    const std::string& Configuration::videoSplashPath(void)
    {
        return stringMappedValueOf(Configuration::KEY_SPLASHVIDEO);
    }

    // CBC device path
    const std::string& Configuration::cbcDevicePath(void)
    {
        return stringMappedValueOf(Configuration::KEY_CBCDEVICE);
    }

    //resume sync
    const std::string& Configuration::resumesyncPath(void)
    {
	    return stringMappedValueOf(Configuration::KEY_RESUMESYNC);
    }

    // A test device.
    const std::string& Configuration::testCBCDevicePath(void)
    {
        return stringMappedValueOf(Configuration::KEY_TESTCBCDEVICE);
    }

    const std::string& Configuration::stringMappedValueOf(const char* key)
    {
        static const std::string nullStr = std::string("");
        const std::string* valueStr;

        try
        {
            valueStr = &(m_presult[key].as<std::string>());
        }
        catch(std::bad_any_cast&)
        {
            LERR_(TAG, "Map error for key " << key);
            valueStr = &nullStr;
        }
        return *valueStr;
    }

    // Display width
    unsigned int Configuration::displayWidth(void) const
    {
        return m_presult[Configuration::KEY_DISPLAYWIDTH].as<unsigned int>();
    }

    // Display height
    unsigned int Configuration::displayHeight(void) const
    {
        return m_presult[Configuration::KEY_DISPLAYHEIGHT].as<unsigned int>();
    }

    // GPIO output number
    int Configuration::gpioNumber(void) const
    {
        return m_presult[Configuration::KEY_GPIONUMBER].as<int>();
    }

    // GPIO peak sustaining time in ms.
    unsigned int Configuration::gpioSustain(void) const
    {
        return m_presult[Configuration::KEY_GPIOSUSTAIN].as<unsigned int>();
    }

    // Use GStreamer
    bool Configuration::useGStreamer(void) const
    {
        return m_presult[Configuration::KEY_USEGSTREAMER].as<bool>();
    }

    // Use CsiCam
    bool Configuration::useCsicam(void) const
    {
        return m_presult[Configuration::KEY_USECSICAM].as<bool>();
    }

    // GStreamer camera custom command.
    const std::string& Configuration::gstCamCmd(void)
    {
        return stringMappedValueOf(Configuration::KEY_GSTCAMCMD);
    }

    // Destructor.
    Configuration::~Configuration(void)
    {
    }

    // Initialize program options.
    bool Configuration::initProgramOptions(int argc, char** argv)
    {
        try
        {
            m_pDesc = std::make_unique<cxxopts::Options>("Early App", "Allowed options");
            m_pDesc->add_options()
                // Help.
                ("help", "Print usages.")

                // Version.
                ("version,v", "Print version number.")

                // Camera input source.
                // NOTE: Camera source option is supported with GStreamer.
                ("camera-input,c",
                "Camera input source selection. Only supported with use-gstreamer option.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_CAMERA_INPUTSOURCE))

                // Splash video.
                ("splash-video,s",
                "File path for splash video.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_VIDEO_SPLASH_PATH))

                // CBC device path.
                ("cbc-device,d",
                "CBC device path.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_CBCDEVICE_PATH))

                ("resume-sync,r",
                "resume sync path.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_RESUME_SYNC_PATH))

                // Test CBC device path.
                ("test-cbc-device,t",
                "A test CBC file path.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_TESTCBCDEVICE_PATH))

                // Audio path - bootup sound.
                (Configuration::KEY_BOOTUPSOUND,
                "Audio file path for boot up sound.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_AUDIO_SPLASHSOUND_PATH))

                // Audio path - RVC sound.
                (Configuration::KEY_RVCSOUND,
                "Audio file path for RVC sound.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_AUDIO_RVCSOUND_PATH))

                // Display width
                ("width,w",
                "Display width.",
                cxxopts::value<unsigned int>())

                // Display height
                ("height,h",
                "Display height.",
                cxxopts::value<unsigned int>())

                // GPIO number
                (Configuration::KEY_GPIONUMBER,
                "GPIO number for KPI measurements. Negative values will be ignored.",
                cxxopts::value<int>()->default_value(std::to_string(Configuration::DEFAULT_GPIONUMBER)))

                // GPIO sustaining time.
                (Configuration::KEY_GPIOSUSTAIN,
                "GPIO sustaining time in ms for KPI measurements.",
                cxxopts::value<unsigned int>()->default_value(std::to_string(Configuration::DEFAULT_GPIOSUSTAIN)))

                // Use GStreamer
                (Configuration::KEY_USEGSTREAMER,
                "Use GStreamer for auido, camera and video.",
                cxxopts::value<bool>()->default_value(Configuration::DEFAULT_USE_GSTREAMER))

		// Custome CSI camera command
                (Configuration::KEY_USECSICAM,
                "Use GStreamer for auido, camera and video.",
                cxxopts::value<bool>()->default_value(Configuration::DEFAULT_USE_CSICAM))

		// Custom GStreamer camera command.
                (Configuration::KEY_GSTCAMCMD,
                "Custom GStreamer camera command. Only supported with use-gstreamer option.",
                cxxopts::value<std::string>()->default_value(Configuration::DEFAULT_GSTCAMCMD));

            m_presult = m_pDesc->parse(argc, argv);
            checkCameraParameter(m_presult["camera-input"].as<std::string>());

             // Help.
            if(m_presult.count("help"))
            {
                m_Valid = false;
                std::cout << m_pDesc->help() << std::endl;
            }
            else if(m_presult.count("version"))
            {
                printVersion(argv[0]);
            }
            // Configuration is valid. Run it.
            else
            {
                // Configuration is ready.
                m_Valid = true;
            }
        }
        catch(const cxxopts::exceptions::exception& e)
        {
            handleProgramOptionException(e);
            return false;
        }
        return true;
    }


    // Camera option checker.
    void Configuration::checkCameraParameter(std::string optStr)
    {
        // Supported camera options.
        if(
            optStr.compare("icam") != 0
            && optStr.compare("v4l2") != 0
            && optStr.compare("test") != 0)
        {
            // Given option is not supported.
            cxxopts::exceptions::exception e(
                std::string("Undefined camera input value: ")
                .append(optStr));
            throw e;
        }
    }

    // Program option parsing exception handler.
    void Configuration::handleProgramOptionException(const std::exception& e)
    {
        const char* errMsg = e.what();

        LERR_(TAG, "Option parse error");
        std::cerr << "ERROR: ";
        if(errMsg)
        {
            std::cerr << errMsg << std::endl;
        }

        // Not a valid option.
        m_Valid = false;

    }

} // namespace
