/*!
 * \file sdrjd.cc
 * \brief Flowgraph engine of the SDR Jammer Detector.
 * \authors Javier Arribas, 2016. jarribas(at)cttc.es
 *          Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *
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

#include "sdrjd.h"
#include <cmath>
#include <ctime>
#include <memory>
#include <exception>
#include <boost/filesystem.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/file_source.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include "new_valve.h"
#include "sdr_block_interface.h"
#include "sdr_block_factory.h"
#include "detector_msg_receiver.h"


sdrjd::sdrjd()
{
    stop_=false;
    run_continuous=false;
}

sdrjd::~sdrjd()
{}


void sdrjd::stop()
{

    top_block->stop();
    top_block->disconnect_all();
}

void sdrjd::set_configuration(std::shared_ptr<ConfigurationInterface> config)
{
    configuration = config;
}

void sdrjd::keyboard_listener()
{
    bool read_keys = true;
    char c = '0';
    while(read_keys && !stop_)
        {
            std::cin.get(c);
            if (c == 'q')
                {
                    std::cout << "Quit keystroke order received, stopping SDRJD !!" << std::endl;
                    stop();
                    usleep(5000);
                }
        }
}

void sdrjd::reset_valve()
{
    if (run_continuous==true)
    {
        valve->reset_valve();
    }
}
void sdrjd::reset_detectors()
{
    detector1->init();
    detector1->reset();
    detector2->init();
    detector2->reset();
    detector3->init();
    detector3->reset();
    detector4->init();
    detector4->reset();
}

void sdrjd::set_detectors_frequency(double freq_hz)
{
    detector1->set_rf_freq(freq_hz);
    detector2->set_rf_freq(freq_hz);
    detector3->set_rf_freq(freq_hz);
    detector4->set_rf_freq(freq_hz);    
}

void sdrjd::reset_noise_estimations()
{
    detector1->set_noise_estimation_done(false);
    detector2->set_noise_estimation_done(false);
    detector4->set_noise_estimation_done(false);     
}

void sdrjd::set_noise_power(std::vector<float> noise_power)
{
    detector1->set_noise_power_estimation(noise_power.at(0));
    detector2->set_noise_power_estimation(noise_power.at(1));
    detector4->set_noise_power_estimation(noise_power.at(2));
}

std::vector<float> sdrjd::get_noise_power()
{
    std::vector<float> noise_estimations;
    noise_estimations.push_back(detector1->get_noise_power_estimation());
    noise_estimations.push_back(detector2->get_noise_power_estimation());
    noise_estimations.push_back(detector4->get_noise_power_estimation());
    return noise_estimations;    
}

bool sdrjd::sdrjd_capture_and_process()
{


    SDRBlockFactory block_factory;

    stop_ = false;

    top_block = gr::make_top_block("SDRJD Capture");

    std::shared_ptr<SDRBlockInterface> source;
    std::shared_ptr<SourceInterface> source_ptr;
    try
    {
        source = std::move(block_factory.GetSignalSource(configuration));
        source_ptr=std::dynamic_pointer_cast<SourceInterface>(source);
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating source " << e << std::endl;
        return 0;
    }

    std::unique_ptr<SDRBlockInterface> conditioner;
    try
    {
        conditioner = block_factory.GetSignalConditioner(configuration);
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating signal conditioner " << e << std::endl;
        return 0;
    }

    std::shared_ptr<SDRBlockInterface> pulsed_jammer_detector_block_sptr;
    try
    {
        pulsed_jammer_detector_block_sptr = std::move(block_factory.GetJammerDetector(configuration,1));//ID 1
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating pulsed jammer detector " << e << std::endl;
        return 0;
    }

    std::shared_ptr<SDRBlockInterface> wb_jammer_detector_block_sptr;
    try
    {
        wb_jammer_detector_block_sptr = std::move(block_factory.GetJammerDetector(configuration,2));//ID 2
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating WB jammer detector " << e << std::endl;
        return 0;
    }

    std::shared_ptr<SDRBlockInterface> cw_jammer_detector_block_sptr;
    try
    {
        cw_jammer_detector_block_sptr = std::move(block_factory.GetJammerDetector(configuration,3));//ID 3
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating CW jammer detector " << e << std::endl;
        return 0;
    }

    std::shared_ptr<SDRBlockInterface> lfm_jammer_detector_block_sptr;
    try
    {
        lfm_jammer_detector_block_sptr = std::move(block_factory.GetJammerDetector(configuration,4));//ID 4
    }
    catch(const boost::exception_ptr & e)
    {
        std::cout << "Exception caught in creating LFM jammer detector " << e << std::endl;
        return 0;
    }

    long fs_in = configuration->property("SDRJD.internal_fs_hz", 2048000);
    double seconds = configuration->property("SignalSource.num_seconds", 10.0);
    double rf_freq_hz = configuration->property("SignalSource.freq_hz", 2412000000.0);
    double start_freq_hz = configuration->property("SDRJD.start_freq_hz", 2412000000.0);
    double stop_freq_hz = configuration->property("SDRJD.stop_freq_hz", 2432000000.0);
    bool enable_sweep = configuration->property("SDRJD.enable_sweep", false);
    run_continuous =configuration->property("SDRJD.run_continuous", false);
    processing_delay_us = configuration->property("SDRJD.processing_delay_us", 10000);
    long nsamples = seconds * fs_in;
    float threshold1 = configuration->property("JammerDetector1.Threshold", 1500.0);
    float threshold2 = configuration->property("JammerDetector2.Threshold", 1.0);
    float threshold3 = configuration->property("JammerDetector3.Threshold", 100.0);
    float threshold4 = configuration->property("JammerDetector4.Threshold", 0.85);

    //Stream valve
    valve = new_make_valve(sizeof(gr_complex), nsamples);
    //Mesage receiver
    detector_msg_receiver_sptr detector_msg_rx;
    detector_msg_rx= detector_msg_receiver_make();

    //configure Jammer detector frequency sweep
    detector_msg_rx->set_signal_source(source_ptr);
    detector_msg_rx->set_enable_sweep(enable_sweep);
    if (enable_sweep==false)
    {
        detector_msg_rx->set_search_space(rf_freq_hz, rf_freq_hz, fs_in);
        std::cout<<"Frequency Sweep is disabled."<<std::endl;
    }else{
        detector_msg_rx->set_search_space(start_freq_hz, stop_freq_hz, fs_in);
        if ((stop_freq_hz - start_freq_hz) < fs_in)
            rf_freq_hz = start_freq_hz + std::floor((stop_freq_hz - start_freq_hz)/2.0);
        else
            rf_freq_hz = start_freq_hz + std::floor(fs_in/2.0);
        source_ptr->set_rf_freq(rf_freq_hz);
        std::cout<<"Frequency sweep enabled."<<std::endl;
    }

    std::cout<<"RF search frequency set to: "<<rf_freq_hz<<" [Hz]"<<std::endl;

    detector_msg_rx->set_sdrjd_engine(this);

    detector1 = std::dynamic_pointer_cast<JammerDetectorInterface>(pulsed_jammer_detector_block_sptr);
    detector1->init();
    detector1->reset();
    detector1->set_threshold(threshold1);
    detector1->set_rf_freq(rf_freq_hz);

    detector2 = std::dynamic_pointer_cast<JammerDetectorInterface>(wb_jammer_detector_block_sptr);
    detector2->init();
    detector2->reset();
    detector2->set_threshold(threshold2);
    detector2->set_rf_freq(rf_freq_hz);

    detector3 = std::dynamic_pointer_cast<JammerDetectorInterface>(cw_jammer_detector_block_sptr);
    detector3->init();
    detector3->reset();
    detector3->set_threshold(threshold3);
    detector3->set_rf_freq(rf_freq_hz);

    detector4 = std::dynamic_pointer_cast<JammerDetectorInterface>(lfm_jammer_detector_block_sptr);
    detector4->init();
    detector4->reset();
    detector4->set_threshold(threshold4);
    detector4->set_rf_freq(rf_freq_hz);

    // start the keyboard_listener thread
    boost::thread keyboard_thread_ = boost::thread(&sdrjd::keyboard_listener, this);
    try
    {
        source->connect(top_block);
        conditioner->connect(top_block);
        detector1->connect(top_block);
        detector2->connect(top_block);
        detector3->connect(top_block);
        detector4->connect(top_block);        
        top_block->connect(source->get_right_block(), 0, conditioner->get_left_block(), 0);
        top_block->connect(conditioner->get_right_block(), 0, valve, 0);
        //connect here the jammer detector blocks, in parallel to the head source------
        //connect also here the asynchronous message ports
        top_block->connect(valve,0, detector1->get_left_block(),0);
        top_block->msg_connect(detector1->get_right_block(), pmt::mp("jammers"), detector_msg_rx, pmt::mp("jammers"));
        top_block->connect(valve,0, detector2->get_left_block(),0);
        top_block->msg_connect(detector2->get_right_block(), pmt::mp("jammers"), detector_msg_rx, pmt::mp("jammers"));
        top_block->connect(valve,0, detector3->get_left_block(),0);
        top_block->msg_connect(detector3->get_right_block(), pmt::mp("jammers"), detector_msg_rx, pmt::mp("jammers"));
        top_block->connect(valve,0, detector4->get_left_block(),0);
        top_block->msg_connect(detector4->get_right_block(), pmt::mp("jammers"), detector_msg_rx, pmt::mp("jammers"));

        top_block->run();

        stop_ = true;
    }
    catch(const std::exception & e)
    {
        std::cout << "Failure connecting the GNU Radio blocks " << e.what() << std::endl;
    }
    catch(const boost::exception & e)
    {
        std::cout << "Failure connecting the GNU Radio blocks " << std::endl;
    }

    std::cin.putback('\n'); //to exit from blocking cin in keyboard listening thread
    //Join keyboard thread
    #ifdef OLD_BOOST
        keyboard_thread_.timed_join(boost::posix_time::seconds(1));
    #endif
    #ifndef OLD_BOOST
        keyboard_thread_.try_join_until(boost::chrono::steady_clock::now() + boost::chrono::milliseconds(1000));
    #endif
    return true;
}
