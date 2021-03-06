/*!
 * \file osmosdr_signal_source.cc
 * \brief Signal source for the Realtek RTL2832U USB dongle DVB-T receiver
 * (see http://sdr.osmocom.org/trac/wiki/rtl-sdr for more information)
 * \author Javier Arribas, 2012. jarribas(at)cttc.es
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "osmosdr_signal_source.h"
#include <iostream>
#include <boost/format.hpp>
#include <glog/logging.h>
#include <gnuradio/blocks/file_sink.h>
#include <boost/throw_exception.hpp>
#include "configuration_interface.h"

using google::LogMessage;


OsmosdrSignalSource::OsmosdrSignalSource(ConfigurationInterface* configuration,
        std::string role, unsigned int in_stream, unsigned int out_stream,
        boost::shared_ptr<gr::msg_queue> queue) :
                role_(role), in_stream_(in_stream), out_stream_(out_stream),
                queue_(queue)
{
    // DUMP PARAMETERS
    std::string empty = "";
    std::string default_dump_file = "./data/signal_source.dat";
    std::string default_item_type = "gr_complex";
    samples_ = configuration->property(role + ".samples", 0);
    dump_ = configuration->property(role + ".dump", false);
    dump_filename_ = configuration->property(role + ".dump_filename",
            default_dump_file);

    // OSMOSDR Driver parameters

    AGC_enabled_ = configuration->property(role + ".AGC_enabled", true);
    freq_ = configuration->property(role + ".freq_hz", 2412.0e6);
    rf_gain_ = configuration->property(role + ".rf_gain", (double)0.0);
    if_gain_ = configuration->property(role + ".if_gain", (double)20.0);
    bb_gain_ = configuration->property(role + ".bb_gain", (double)20.0);
    sample_rate_ = configuration->property(role + ".sampling_frequency", (double)2.0e6);
    double default_bw = sample_rate_;
    bw_ = configuration->property(role + ".bandwidth", default_bw);
    item_type_ = configuration->property(role + ".item_type", default_item_type);
    osmosdr_args_ = configuration->property(role + ".osmosdr_args", std::string( ));

    if (item_type_.compare("short") == 0)
        {
            item_size_ = sizeof(short);
        }
    else if (item_type_.compare("gr_complex") == 0)
        {
            item_size_ = sizeof(gr_complex);
            // 1. Make the driver instance
            try
            {
                    if (!osmosdr_args_.empty())
                        {
                        std::cout << "OsmoSdr arguments: " << osmosdr_args_ << std::endl;
                        LOG(INFO) << "OsmoSdr arguments: " << osmosdr_args_;
                        }
                    osmosdr_source_ = osmosdr::source::make(osmosdr_args_);
            }
            catch( boost::exception & e )
            {
                    DLOG(FATAL) << "Boost exception: " << boost::diagnostic_information(e);
            }

            // 1.2 Check if there is hardware connected or not
            if (osmosdr_source_->get_num_channels()==0)
            {
                DLOG(FATAL) <<"No osmosdr hardware found";

               std::cout<<"No osmosdr hardware found"<<std::endl;
                boost::throw_exception(std::exception());
            }
            // 2 set sampling rate
            osmosdr_source_->set_sample_rate(sample_rate_);
            std::cout << boost::format("Actual RX Rate: %f [SPS]...") % (osmosdr_source_->get_sample_rate()) << std::endl ;
            LOG(INFO) << boost::format("Actual RX Rate: %f [SPS]...") % (osmosdr_source_->get_sample_rate());

            // 3. set rx frequency
            osmosdr_source_->set_center_freq(freq_);
            std::cout << boost::format("Actual RX Freq: %f [Hz]...") % (osmosdr_source_->get_center_freq()) << std::endl ;
            LOG(INFO) << boost::format("Actual RX Freq: %f [Hz]...") % (osmosdr_source_->get_center_freq());

            // TODO: Assign the remnant IF from the PLL tune error
            std::cout << boost::format("PLL Frequency tune error %f [Hz]...") % (osmosdr_source_->get_center_freq() - freq_) ;
            LOG(INFO) <<  boost::format("PLL Frequency tune error %f [Hz]...") % (osmosdr_source_->get_center_freq() - freq_) ;

            // 4. set rx bandwidth
            osmosdr_source_->set_bandwidth(bw_);
            std::cout << boost::format("Actual RX Bandwidth: %f [Hz]...") % (osmosdr_source_->get_bandwidth()) << std::endl ;
            LOG(INFO) << boost::format("Actual RX Bandwidth: %f [Hz]...") % (osmosdr_source_->get_bandwidth());


            // 5. set rx gain
            if (this->AGC_enabled_ == true)
                {
                    osmosdr_source_->set_gain_mode(true);
                    std::cout << "AGC enabled" << std::endl;
                    LOG(INFO) << "AGC enabled";
                }
            else
                {
                    osmosdr_source_->set_gain_mode(false);
                    osmosdr_source_->set_gain(rf_gain_, 0);
                    osmosdr_source_->set_if_gain(if_gain_, 0);
                    osmosdr_source_->set_bb_gain(bb_gain_, 0);
                    std::cout << boost::format("Actual RX Gain: %f dB...") % osmosdr_source_->get_gain() << std::endl;
                    LOG(INFO) << boost::format("Actual RX Gain: %f dB...") % osmosdr_source_->get_gain();
                }
        }
    else
        {
            LOG(WARNING) << item_type_ << " unrecognized item type. Using short.";
            item_size_ = sizeof(short);
        }


    if (dump_)
        {
            DLOG(INFO) << "Dumping output into file " << dump_filename_;
            file_sink_ = gr::blocks::file_sink::make(item_size_, dump_filename_.c_str());
            DLOG(INFO) << "file_sink(" << file_sink_->unique_id() << ")";
        }
}


void OsmosdrSignalSource::set_rf_freq(double new_rf_freq_hz)
{
    osmosdr_source_->set_center_freq(new_rf_freq_hz);
}

void OsmosdrSignalSource::set_gain(int new_gain_db){
    osmosdr_source_->set_gain(new_gain_db, 0);
}


OsmosdrSignalSource::~OsmosdrSignalSource()
{}



void OsmosdrSignalSource::connect(gr::top_block_sptr top_block)
{
    if (samples_ != 0)
        {
            if (dump_)
                {
                    top_block->connect(osmosdr_source_, 0, file_sink_, 0);
                    DLOG(INFO) << "connected osmosdr_source_ to file sink";
                }
        }
    else
        {
            if (dump_)
                {
                    top_block->connect(osmosdr_source_, 0, file_sink_, 0);
                    DLOG(INFO) << "connected osmosdr source to file sink";
                }
        }
}



void OsmosdrSignalSource::disconnect(gr::top_block_sptr top_block)
{

    if (dump_)
        {
            top_block->disconnect(osmosdr_source_, 0, file_sink_, 0);
        }
}



gr::basic_block_sptr OsmosdrSignalSource::get_left_block()
{
    LOG(WARNING) << "Trying to get signal source left block.";
    return gr::basic_block_sptr();
}



gr::basic_block_sptr OsmosdrSignalSource::get_right_block()
{
    return osmosdr_source_;
}
