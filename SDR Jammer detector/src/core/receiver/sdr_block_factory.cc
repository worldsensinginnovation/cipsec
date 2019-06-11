/*!
 * \file sdr_block_factory.cc
 * \brief Interface of a factory that returns smart pointers to SDR blocks.
 * \authors Javier Arribas, 2016. jarribas(at)cttc.es
 *          Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *
 * This class encapsulates the complexity behind the instantiation
 * of SDR blocks.
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


#include "sdr_block_factory.h"
#include <string>
#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include "configuration_interface.h"
#include "in_memory_configuration.h"
#include "sdr_block_interface.h"
#include "pass_through.h"
#include "file_signal_source.h"
#include "udp_signal_source.h"
#include "signal_conditioner.h"
#include "byte_to_short.h"
#include "ibyte_to_cbyte.h"
#include "ibyte_to_cshort.h"
#include "ibyte_to_complex.h"
#include "ishort_to_cshort.h"
#include "ishort_to_complex.h"
#include "direct_resampler_conditioner.h"
#include "fir_filter.h"
#include "freq_xlating_fir_filter.h"

#include "pulsed_jammer_detector.h"
#include "wb_jammer_detector.h"
#include "cw_jammer_detector.h"
#include "lfm_jammer_detector.h"


#if OSMOSDR_DRIVER
#include "osmosdr_signal_source.h"
#endif

#if UHD_DRIVER
#include "uhd_signal_source.h"
#endif


using google::LogMessage;


SDRBlockFactory::SDRBlockFactory()
{}


SDRBlockFactory::~SDRBlockFactory()
{}


std::unique_ptr<SDRBlockInterface> SDRBlockFactory::GetSignalSource(
        std::shared_ptr<ConfigurationInterface> configuration, int ID)
{
    std::string default_implementation = "File_Signal_Source";
    std::string role = "SignalSource";
    if (ID != -1)
        {
            role = "SignalSource" + boost::lexical_cast<std::string>(ID);
        }
    std::string implementation = configuration->property(role + ".implementation", default_implementation);
    LOG(INFO) << "Getting SignalSource with implementation " << implementation;
    return GetBlock(configuration, role, implementation, 0, 1);
}

std::unique_ptr<SDRBlockInterface> SDRBlockFactory::GetJammerDetector(
        std::shared_ptr<ConfigurationInterface> configuration, int ID)
{
    std::string default_implementation = "Pulsed_Jammer_Detector";
    std::string role = "JammerDetector";
    if (ID != -1)
    {
        role = "JammerDetector" + boost::lexical_cast<std::string>(ID);
    }
    std::string implementation = configuration->property(role + ".implementation", default_implementation);
    LOG(INFO) << "Getting JammerDetector with implementation " << implementation;
    return GetBlock(configuration, role, implementation, 0, 1);
}




std::unique_ptr<SDRBlockInterface> SDRBlockFactory::GetSignalConditioner(
        std::shared_ptr<ConfigurationInterface> configuration, int ID)
{
    std::string default_implementation = "Pass_Through";
    std::string role_conditioner = "SignalConditioner" ;
    std::string role_datatypeadapter = "DataTypeAdapter";
    std::string role_inputfilter = "InputFilter";
    std::string role_resampler = "Resampler";

    if (ID != -1)
        {
            role_conditioner = "SignalConditioner" + boost::lexical_cast<std::string>(ID);
            role_datatypeadapter = "DataTypeAdapter" + boost::lexical_cast<std::string>(ID);
            role_inputfilter = "InputFilter" + boost::lexical_cast<std::string>(ID);
            role_resampler = "Resampler" + boost::lexical_cast<std::string>(ID);
        }

    std::string signal_conditioner = configuration->property(role_conditioner + ".implementation", default_implementation);

    std::string data_type_adapter;
    std::string input_filter;
    std::string resampler;
    if(signal_conditioner.compare("Pass_Through") == 0)
        {
            data_type_adapter = "Pass_Through";
            input_filter = "Pass_Through";
            resampler = "Pass_Through";
        }
    else
        {
            data_type_adapter = configuration->property(role_datatypeadapter + ".implementation", default_implementation);
            input_filter = configuration->property(role_inputfilter + ".implementation", default_implementation);
            resampler = configuration->property(role_resampler + ".implementation", default_implementation);
        }

    LOG(INFO) << "Getting SignalConditioner with DataTypeAdapter implementation: "
            << data_type_adapter << ", InputFilter implementation: "
            << input_filter << ", and Resampler implementation: "
            << resampler;

    std::unique_ptr<SDRBlockInterface> conditioner_(new SignalConditioner(configuration.get(),
        std::move(GetBlock(configuration, role_datatypeadapter, data_type_adapter, 1, 1)),
        std::move(GetBlock(configuration, role_inputfilter, input_filter, 1, 1)),
        std::move(GetBlock(configuration, role_resampler, resampler, 1, 1)),
        role_conditioner, "Signal_Conditioner"));
    return conditioner_;
}


/*
 * Returns the block with the required configuration and implementation
 *
 * PLEASE ADD YOUR NEW BLOCK HERE!!
 *
 */

std::unique_ptr<SDRBlockInterface> SDRBlockFactory::GetBlock(
        std::shared_ptr<ConfigurationInterface> configuration,
        std::string role,
        std::string implementation, unsigned int in_streams,
        unsigned int out_streams, boost::shared_ptr<gr::msg_queue> queue)
{
    std::unique_ptr<SDRBlockInterface> block;

    //PASS THROUGH ----------------------------------------------------------------
    if (implementation.compare("Pass_Through") == 0)
        {
            std::unique_ptr<SDRBlockInterface> block_(new Pass_Through(configuration.get(), role, in_streams, out_streams));
            block = std::move(block_);
        }

    // SIGNAL SOURCES -------------------------------------------------------------
    else if (implementation.compare("Udp_Signal_Source") == 0)
        {
            try
            {
                    std::unique_ptr<SDRBlockInterface> block_(new UdpSignalSource(configuration.get(), role, in_streams,
                            out_streams, queue));
                    block = std::move(block_);
            }

            catch (const std::exception &e)
            {
                    std::cout << "SDRJD program ended." << std::endl;
                    exit(1);
            }
        }
    else if (implementation.compare("File_Signal_Source") == 0)
        {
            try
            {
                    std::unique_ptr<SDRBlockInterface> block_(new FileSignalSource(configuration.get(), role, in_streams,
                            out_streams, queue));
                    block = std::move(block_);
            }

            catch (const std::exception &e)
            {
                    std::cout << "SDRJD program ended." << std::endl;
                    exit(1);
            }
        }
#if UHD_DRIVER
    else if (implementation.compare("UHD_Signal_Source") == 0)
        {
            std::unique_ptr<SDRBlockInterface> block_(new UhdSignalSource(configuration.get(), role, in_streams,
                    out_streams, queue));
            block = std::move(block_);
        }
#endif

#if OSMOSDR_DRIVER
    else if (implementation.compare("Osmosdr_Signal_Source") == 0)
        {
            std::unique_ptr<SDRBlockInterface> block_(new OsmosdrSignalSource(configuration.get(), role, in_streams,
                    out_streams, queue));
            block = std::move(block_);
        }
#endif

    // DATA TYPE ADAPTER -----------------------------------------------------------
    else if (implementation.compare("Byte_To_Short") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new ByteToShort(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Ibyte_To_Cbyte") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new IbyteToCbyte(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Ibyte_To_Cshort") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new IbyteToCshort(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Ibyte_To_Complex") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new IbyteToComplex(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Ishort_To_Cshort") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new IshortToCshort(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Ishort_To_Complex") == 0)
        {
            std::unique_ptr<SDRBlockInterface>block_(new IshortToComplex(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }

    // INPUT FILTER ----------------------------------------------------------------
    else if (implementation.compare("Fir_Filter") == 0)
        {
            std::unique_ptr<SDRBlockInterface> block_(new FirFilter(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }
    else if (implementation.compare("Freq_Xlating_Fir_Filter") == 0)
        {
            std::unique_ptr<SDRBlockInterface> block_(new FreqXlatingFirFilter(configuration.get(), role, in_streams,
                    out_streams));
            block = std::move(block_);
        }

    // RESAMPLER -------------------------------------------------------------------
    else if (implementation.compare("Direct_Resampler") == 0) {
        std::unique_ptr<SDRBlockInterface> block_(new DirectResamplerConditioner(configuration.get(), role,
                                                                                  in_streams, out_streams));
        block = std::move(block_);

    }// JAMMER DETECTORS --------------------------------------------------------
    else if (implementation.compare("Pulsed_Jammer_Detector") == 0)
    {
        std::unique_ptr<SDRBlockInterface> block_(new PulsedJammerDetector(configuration.get(), role,
                                                                                  in_streams, out_streams));
        block = std::move(block_);
    }
    else if (implementation.compare("WB_Jammer_Detector") == 0)
    {
        std::unique_ptr<SDRBlockInterface> block_(new WBJammerDetector(configuration.get(), role,
                                                                                  in_streams, out_streams));
        block = std::move(block_);
    }
    else if (implementation.compare("CW_Jammer_Detector") == 0)
    {
        std::unique_ptr<SDRBlockInterface> block_(new CWJammerDetector(configuration.get(), role,
                                                                                  in_streams, out_streams));
        block = std::move(block_);
    }
    else if (implementation.compare("LFM_Jammer_Detector") == 0)
    {
        std::unique_ptr<SDRBlockInterface> block_(new LFMJammerDetector(configuration.get(), role,
                                                                                  in_streams, out_streams));
        block = std::move(block_);
    }
    else
        {
            // Log fatal. This causes execution to stop.
            LOG(ERROR) << role << "." << implementation << ": Undefined implementation for block";
        }
    return std::move(block);
}
