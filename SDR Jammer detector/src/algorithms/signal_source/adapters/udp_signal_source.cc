/*!
 * \file udp_signal_source.cc
 * \brief Implementation of a class that reads signals samples from a file
 * and adapts it to a SignalSourceInterface
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *         Javier Arribas, 2011 jarribas(at)cttc.es
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

#include "udp_signal_source.h"
#include <cstdlib>
#include <iomanip>
#include <exception>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <volk/volk.h>
#include "configuration_interface.h"

using google::LogMessage;

/*
DEFINE_string(signal_source, "-",
        "If defined, path to the file containing the signal samples (overrides the configuration file)");
*/

UdpSignalSource::UdpSignalSource(ConfigurationInterface* configuration,
        std::string role, unsigned int in_streams, unsigned int out_streams,
        boost::shared_ptr<gr::msg_queue> queue) :
                        role_(role), in_streams_(in_streams), out_streams_(out_streams), queue_(queue)
{
    std::string default_item_type = "short";
    std::string default_listen_ipaddr = "0.0.0.0";

    item_type_ = configuration->property(role + ".item_type", default_item_type);
    sampling_frequency_ = configuration->property(role + ".sampling_frequency", 0);
    enable_throttle_control_ = configuration->property(role + ".enable_throttle_control", false);
    port_ = configuration->property(role + ".udp_port", 1234);
    packet_size_ = configuration->property(role + ".udp_packet_size", 1472);
    eof_ = configuration->property(role + ".udp_eof", true);
    listen_ipaddr_ = configuration->property(role + ".udp_listen_ipaddr", default_listen_ipaddr);


    if (item_type_.compare("gr_complex") == 0)
        {
            item_size_ = sizeof(gr_complex);
        }
    else if (item_type_.compare("float") == 0)
        {
            item_size_ = sizeof(float);
        }
    else if (item_type_.compare("short") == 0)
        {
            item_size_ = sizeof(int16_t);
        }
    else if (item_type_.compare("ishort") == 0)
        {
            item_size_ = sizeof(int16_t);
        }
    else if (item_type_.compare("byte") == 0)
        {
            item_size_ = sizeof(int8_t);
        }
    else if (item_type_.compare("ibyte") == 0)
        {
            item_size_ = sizeof(int8_t);
        }
    else
        {
            LOG(WARNING) << item_type_
                    << " unrecognized item type. Using gr_complex.";
            item_size_ = sizeof(gr_complex);
        }
    try
    {
            udp_source_ = gr::blocks::udp_source::make(item_size_, listen_ipaddr_.c_str(), port_, packet_size_, eof_);
                    std::cout
                    << "Started UDP source listening to ip address " << listen_ipaddr_ << " at port " << port_ << " packet size " << packet_size_ << " eof " << eof_
                    << std::endl;
            udp_source_->set_thread_priority(100);

    }
    catch (const std::exception &e)
    {
                    std::cerr
                    << "The receiver was configured to work with a udp signal source "
                    << std::endl
                    << "but the specified udp socket is unreachable by SDRJD."
                    << std::endl
                    << " UDP source listening to ip address " << listen_ipaddr_ << " at port " << port_ << " packet size " << packet_size_ << " eof " << eof_
                    << std::endl
                    <<  "Please modify your configuration file"
                    << std::endl
                    <<  "and point SignalSource.filename to a valid raw data file. Then:"
                    << std::endl
                    << "$ sdrjd --config_file=/path/to/my_SDRJD_configuration.conf"
                    << std::endl
                    << "Examples of configuration files available at:"
                    << std::endl
                    << GNSSSDR_INSTALL_DIR "/share/gnss-sdr/conf/"
                    << std::endl;

            LOG(INFO) << "udp_signal_source: Unable to open the udp socket to listen to ip address " << listen_ipaddr_ << " on port " << port_ << ", exiting the program.";
            throw(e);
    }

    DLOG(INFO) << "udp_source(" << udp_source_->unique_id() << ")";

    if (enable_throttle_control_)
        {
            throttle_ = gr::blocks::throttle::make(item_size_, sampling_frequency_);
	    std::cout << "Throttle enabled" << std::endl;
        }

}

UdpSignalSource::~UdpSignalSource()
{}


void UdpSignalSource::set_rf_freq(double new_rf_freq_hz)
{
    //nothing to do here
}
void UdpSignalSource::set_gain(int new_gain_db){
    //nothing to do here
}



void UdpSignalSource::connect(gr::top_block_sptr top_block)
{
	//top_block->connect(udp_source_, 0);
            if (enable_throttle_control_ == true)
                {
                    top_block->connect(udp_source_, 0, throttle_, 0);
                    DLOG(INFO) << "connected file source to throttle";
		}
}


void UdpSignalSource::disconnect(gr::top_block_sptr top_block)
{
	//top_block->disconnect(udp_source_, 0);
            if (enable_throttle_control_ == true)
                {
                    top_block->disconnect(udp_source_, 0, throttle_, 0);
                    DLOG(INFO) << "disconnected file source to throttle";
		}
}


gr::basic_block_sptr UdpSignalSource::get_left_block()
{
    LOG(WARNING) << "Left block of a signal source should not be retrieved";
    return gr::blocks::udp_source::sptr();
}


gr::basic_block_sptr UdpSignalSource::get_right_block()
{

        if (enable_throttle_control_ == true)
            {
                return throttle_;
            }
        else
            {
                return udp_source_;
            }
}
