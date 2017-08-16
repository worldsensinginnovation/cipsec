/*!
 * \file main.cc
 * \brief Main file of the SDR Jammer Detector program.
 * \authors <ul>
 *          <li> Javier Arribas, 2016. jarribas(at)cttc.es
 *          <li> Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *          <ul>
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2016
 *
 * SDRJD is a software defined Jammer Detection software,
 * inspired by the GNSS-SDR architecture and partially using
 * original and modified GNSS-SDR files.
 *
 * This file is part of SDRJD.
 *
 * SDRJD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDRJD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SDRJD. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#ifndef SDR_JD_VERSION
#define SDR_JD_VERSION "0.0.1"
#endif

#include <ctime>
#include <exception>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/exception/detail/exception_ptr.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "configuration_interface.h"
#include "file_configuration.h"

#include "sdrjd.h"

using google::LogMessage;

DECLARE_string(log_dir);

std::string s1_(GSDRJD_INSTALL_DIR);
std::string s2_("/share/sdrjd/conf/sdrjd.conf");
std::string s3_ = s1_ + s2_;

DEFINE_string(config_file, s3_,
        "Path to the file containing the configuration parameters");

int main(int argc, char** argv)
{

    const std::string intro_help(
            std::string("\n Software Defined Radio Jammer Detector (SDRJD)\n")
    +
    "Copyright (C) 2016 (see AUTHORS file for a list of contributors)\n"
    +
    "This program comes with ABSOLUTELY NO WARRANTY;\n"
    +
    "See COPYING file to see a copy of the General Public License\n \n");

    google::SetUsageMessage(intro_help);
    google::SetVersionString(SDR_JD_VERSION);
    google::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "Initializing... Please wait." << std::endl;

    google::InitGoogleLogging(argv[0]);
    if (FLAGS_log_dir.empty())
        {
            std::cout << "Logging will be done at "

                     << "/tmp"
                     << std::endl
                     << "Use sdrjd --log_dir=/path/to/log to change that."
                     << std::endl;
        }
    else
        {
            const boost::filesystem::path p (FLAGS_log_dir);
            if (!boost::filesystem::exists(p))
                {
                    std::cout << "The path "
                              << FLAGS_log_dir
                              << " does not exist, attempting to create it"
                              << std::endl;
                    boost::filesystem::create_directory(p);
                }
            std::cout << "Logging with be done at "
                      << FLAGS_log_dir << std::endl;
        }

    // 1. Load configuration parameters from config file

    std::shared_ptr<ConfigurationInterface> configuration = std::make_shared<FileConfiguration>(FLAGS_config_file);

    sdrjd sdrjd_engine;

    sdrjd_engine.set_configuration(configuration);

    // 2. Capture some front-end samples and process
    try
    {
        sdrjd_engine.sdrjd_capture_and_process();
    }
    catch(const boost::bad_lexical_cast & e)
    {
        std::cout << "Exception caught while capturing samples (bad lexical cast)" << std::endl;
    }
    catch(const boost::io::too_few_args & e)
    {
        std::cout << "Exception caught while capturing samples (too few args)" << std::endl;
    }
    catch(const boost::exception & e)
    {
        std::cout << "Failure connecting the GNU Radio blocks " << std::endl;
        return false;
    }

    // 3. Stop the flowgraph

    std::cout << "Stopping SDR Jammer Detector, please wait!" << std::endl;

    google::ShutDownCommandLineFlags();
    std::cout << "SDR Jammer Detector program ended." << std::endl;
}
