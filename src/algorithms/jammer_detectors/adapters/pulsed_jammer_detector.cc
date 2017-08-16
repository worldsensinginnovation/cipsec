/*!
 * \file pulsed_jammer_detector.cc
 * \brief Adapts a Pulsed Jammer Detector block to a JammerDetectorInterface
 * \authors Javier Arribas, 2016. jarribas(at)cttc.es
 *          Luis Esteve, 2016. luis(at)epsilon-formacion.com
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

#include "pulsed_jammer_detector.h"
#include <glog/logging.h>
#include "configuration_interface.h"


using google::LogMessage;

PulsedJammerDetector::PulsedJammerDetector(
        ConfigurationInterface* configuration, std::string role,
        unsigned int in_streams, unsigned int out_streams) :
    role_(role), in_streams_(in_streams), out_streams_(out_streams)
{
    configuration_ = configuration;
    std::string default_item_type = "gr_complex";
    std::string default_dump_filename = "./data/pulsed_jammer_detector.dat";

    DLOG(INFO) << "role " << role;

    item_type_ = configuration_->property(role + ".item_type", default_item_type);

    fs_in_ = configuration_->property("SDRJD.internal_fs_hz", 2048000.0);
    dump_ = configuration_->property(role + ".dump", false);
    dump_filename_ = configuration_->property(role + ".dump_filename", default_dump_filename);
    vector_length_ =  configuration_->property(role + ".block_length", 1000);
    fft_samples_ = configuration_->property(role + ".fft_samples", 8);
    noise_estimations_ = configuration_->property(role + ".noise_estimations", 100);
    A_ = configuration_->property(role + ".A", 2);
    B_ = configuration_->property(role + ".B", 1);
    max_dwells_ = configuration_->property(role + ".max_dwells", 10);

    item_size_ = sizeof(gr_complex);
    detector_cc_ = pulsed_jammer_make_detector_cc(fs_in_, vector_length_, fft_samples_,
                        noise_estimations_, A_, B_, max_dwells_, dump_, dump_filename_);

    DLOG(INFO) << "detector(" << detector_cc_->unique_id() << ")";

    stream_to_vector_ = gr::blocks::stream_to_vector::make(item_size_, vector_length_);
    DLOG(INFO) << "stream_to_vector(" << stream_to_vector_->unique_id() << ")";
}


PulsedJammerDetector::~PulsedJammerDetector()
{
}


void PulsedJammerDetector::set_threshold(float threshold)
{

    threshold_ = threshold;

    DLOG(INFO) << "Pulsed Jammer detector Threshold = " << threshold_;

    detector_cc_->set_threshold(threshold_);
}


void PulsedJammerDetector::init()
{

            detector_cc_->init();
}

void PulsedJammerDetector::reset()
{
            detector_cc_->set_active(true);
}


void PulsedJammerDetector::set_state(int state)
{
    detector_cc_->set_state(state);
}

void PulsedJammerDetector::connect(gr::top_block_sptr top_block)
{
    if (item_type_.compare("gr_complex") == 0)
        {
            top_block->connect(stream_to_vector_, 0, detector_cc_, 0);
        }
    else
        {
            LOG(WARNING) << item_type_ << " unknown acquisition item type";
        }
}


void PulsedJammerDetector::disconnect(gr::top_block_sptr top_block)
{
    if (item_type_.compare("gr_complex") == 0)
        {
            top_block->disconnect(stream_to_vector_, 0, detector_cc_, 0);
        }
    else
        {
            LOG(WARNING) << item_type_ << " unknown acquisition item type";
        }
}


gr::basic_block_sptr PulsedJammerDetector::get_left_block()
{
    if (item_type_.compare("gr_complex") == 0)
        {
            return stream_to_vector_;
        }
    else
        {
            LOG(WARNING) << item_type_ << " unknown acquisition item type";
            return nullptr;
        }
}


gr::basic_block_sptr PulsedJammerDetector::get_right_block()
{
            return detector_cc_;
}

