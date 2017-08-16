/*!
 * \file udp_signal_source.h
 * \brief Interface of a class that reads signals samples from a file
 * and adapts it to a SignalSourceInterface
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *
 * This class represents a file signal source. Internally it uses a GNU Radio's
 * gr_file_source as a connector to the data.
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

#ifndef GNSS_SDR_UDP_SIGNAL_SOURCE_H_
#define GNSS_SDR_UDP_SIGNAL_SOURCE_H_

#include <string>
#include <gnuradio/blocks/udp_source.h>
#include <gnuradio/hier_block2.h>
#include <gnuradio/msg_queue.h>
#include <gnuradio/blocks/throttle.h>
#include "source_interface.h"


class ConfigurationInterface;

/*!
 * \brief Class that reads signals samples from a file
 * and adapts it to a SignalSourceInterface
 */
class UdpSignalSource: public SourceInterface
{
public:
    UdpSignalSource(ConfigurationInterface* configuration, std::string role,
            unsigned int in_streams, unsigned int out_streams,
            boost::shared_ptr<gr::msg_queue> queue);

    virtual ~UdpSignalSource();
    std::string role()
    {
        return role_;
    }

    /*!
     * \brief Returns "Udp_Signal_Source".
     */
    std::string implementation()
    {
        return "Udp_Signal_Source";
    }
    size_t item_size()
    {
        return item_size_;
    }
    void connect(gr::top_block_sptr top_block);
    void disconnect(gr::top_block_sptr top_block);
    gr::basic_block_sptr get_left_block();
    gr::basic_block_sptr get_right_block();
    std::string item_type()
    {
        return item_type_;
    }
    void set_rf_freq(double new_rf_freq_hz);
    void set_gain(int new_gain_db);

private:
    unsigned long long samples_;
    std::string item_type_;
    std::string role_;
    gr::blocks::udp_source::sptr udp_source_;
    boost::shared_ptr<gr::block> valve_;
    boost::shared_ptr<gr::msg_queue> queue_;
    size_t item_size_;
    unsigned int in_streams_;
    unsigned int out_streams_;
    gr::blocks::throttle::sptr  throttle_;
    long sampling_frequency_;
    // Throttle control
    bool enable_throttle_control_;
    unsigned int port_;
    unsigned int packet_size_;
    bool eof_;
    std::string listen_ipaddr_;
};

#endif /*GNSS_SDR_UDP_SIGNAL_SOURCE_H_*/
