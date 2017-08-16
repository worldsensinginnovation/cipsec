/*!
 * \file detector_msg_receiver.cc
 * \brief Handles messages of detection blocks and make decissions
 * \authors Javier Arribas, 2016. jarribas(at)cttc.es
 *          Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *
 * This block handles messages of all detection blocks and makes decisions
 * on the presence or absence of jammers
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


#include <gnuradio/io_signature.h>
#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "detector_msg_receiver.h"

using google::LogMessage;


detector_msg_receiver_sptr detector_msg_receiver_make()
{
    return detector_msg_receiver_sptr(new detector_msg_receiver());
}



const std::string detector_msg_receiver::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%d-%m-%Y-%H-%M-%S", &tstruct);

    return buf;
}

bool detector_msg_receiver::send_sys_v_msg(jammer_detector_msg jammer)
{

    /* Fill Sys V message structures */
    int msgsend_size;
    jammer_msgbuf msg;
    msg.jammer_msg=jammer;
    msgsend_size=sizeof(msg.jammer_msg);
    msg.mtype=1; /* default message ID */


    /* SEND SOLUTION OVER A MESSAGE QUEUE */
    /* non-blocking Sys V message send */
    msgsnd(sysv_msqid, &msg, msgsend_size, IPC_NOWAIT);
    return true;

}

bool detector_msg_receiver::write_new_log_entry(jammer_detector_msg* jammer)
{
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&jammer->timestamp);
    strftime(buf, sizeof(buf), "%d-%m-%Y-%H-%M-%S", &tstruct);
    std::string str_time = std::string(buf);
    std::stringstream log_line;
    log_line.str("");
    log_line << str_time;
    log_line << std::setfill(' ');
    log_line << std::fixed;
    log_line.precision(3);
    if (jammer->detection == true) {
        log_line << "  " << "JammerON";
    }else{
        log_line << "  " << "JammerOFF";
    }
    log_line << "  " << jammer->jammer_type
             << "  " << jammer->rf_freq_hz
             << "  " << jammer->jnr_db
             << "  " << jammer->test_value << std::endl;
    fprintf(log_file, "%s", log_line.str().c_str());
    return true;
}
void detector_msg_receiver::make_and_send_decision(jammer_detector_msg* current_jammers)
{
    struct tm  tstruct;
    char       buf[80];
    
    if ( current_jammers[2].detection == true )
    {
        current_jammers[0].detection = false;
        current_jammers[0].jnr_db = log10(0.0);
        current_jammers[1].detection = false;
        current_jammers[1].jnr_db = log10(0.0);
        current_jammers[3].detection = false;
        current_jammers[3].jnr_db = log10(0.0);
    }
    else if ( current_jammers[3].detection == true )
    {
        current_jammers[0].detection = false;
        current_jammers[0].jnr_db = log10(0.0);
        current_jammers[1].detection = false;
        current_jammers[1].jnr_db = log10(0.0);
    }
    else if ( current_jammers[0].detection == true )
    {
        current_jammers[1].detection = false;
        current_jammers[1].jnr_db = log10(0.0);   
    }

    unsigned int MAX_DETECTORS = 4;

    for (unsigned int i = 0; i < MAX_DETECTORS; i++)
    {
        tstruct = *localtime(&current_jammers[i].timestamp);
        strftime(buf, sizeof(buf), "%d-%m-%Y-%H-%M-%S", &tstruct);
 
        // if (current_jammers[i].detection == true)
        // {
        //     if (current_jammers[i].jnr_db < 1.0)
        //         current_jammers[i].detection = false;
        // }

        if (current_jammers[i].detection == true)
        {
            
            switch(current_jammers[i].jammer_type)
            {
                case 1:
                    std::cout << "Pulsed jammer detected at time " << buf << std::endl;
                    break;
                case 2:
                    std::cout << "WB jammer detected at time " << buf << std::endl;
                    break;
                case 3:
                    std::cout << "CW jammer detected at time " << buf << std::endl;
                    break;
                case 4:
                    std::cout << "LFM jammer detected at time " << buf << std::endl;
                    break;                           
                default:
                    std::cout << "Unknown jammer detected message" << std::endl;
                    break;
            }                      

            std::cout << "Jammer RF frequency: " << current_jammers[i].rf_freq_hz << " Hz" << std::endl;
            std::cout << "Jammer JNR: " << current_jammers[i].jnr_db << " dB" << std::endl;
            std::cout << "Jammer test value: " << current_jammers[i].test_value << std::endl;
            std::cout << "Jammer samplecounter: " << current_jammers[i].sample_counter << std::endl;
            last_detection_time = current_jammers[i].timestamp;

            try{
                if (last_jammer_detection_status.at(current_freq_bin_idx).at(i)== false)
                {
                    write_new_log_entry(&current_jammers[i]); //write new log file entry
                    //last_detection_time = current_jammers[i].timestamp;
                    last_jammer_detection_status.at(current_freq_bin_idx).at(i) = true;
                }
            }catch(std::exception &e)
            {
                std::cout<<"unexpected error 2!"<<std::endl;
            }

        }else
        {
            if ((current_jammers[i].timestamp-last_detection_time)>jammer_reporting_interval_s)
            {
                std::cout << "No jammer detection at time " << buf << std::endl;
                std::cout << "Jammer samplecounter: " << current_jammers[i].sample_counter << std::endl;
                last_detection_time = current_jammers[i].timestamp;
            }
            try{
                if (last_jammer_detection_status.at(current_freq_bin_idx).at(i) == true)
                {
                    write_new_log_entry(&current_jammers[i]); //write new log file entry
                    last_detection_time = current_jammers[i].timestamp;
                    last_jammer_detection_status.at(current_freq_bin_idx).at(i) = false;
                }
            }catch(std::exception &e)
            {
                std::cout<<"unexpected error 3!"<<std::endl;
            }

        }
        try{
            send_sys_v_msg(current_jammers[i]);//send Sys V queue message
        }catch(std::exception &e)
        {
            std::cout<<"unexpected error 4!"<<std::endl;
        }
    }
    //return true;
}
void detector_msg_receiver::msg_handler_events(pmt::pmt_t msg)
{

    try {
        if( pmt::any_ref(msg).type() == typeid(std::shared_ptr<jammer_detector_msg>) )
        {
            // ### NEW JAMMER DETECTION ###
            std::shared_ptr<jammer_detector_msg> jammer;
            jammer = boost::any_cast<std::shared_ptr<jammer_detector_msg>>(pmt::any_ref(msg));
            try{
                current_bin_scan_done.at(jammer->jammer_type-1)=true;
            }catch(std::exception &e)
            {
                std::cout<<"unexpected error 1! "<<e.what()<<std::endl;
            }
            current_jammers[jammer->jammer_type-1] = *jammer;
        }
        else
        {
            LOG(WARNING) << "jammer msg handler unknown object type!";
        }
    }
    catch(boost::bad_any_cast& e)
    {
        LOG(WARNING) << "jammer msg handler Bad any cast!";
    }


    //check if the detectors have been scanned the current frequency bin and decide move to next bin
    if (enable_sweep == true)
    {
        bool move_next_freq = true;
        for (unsigned int i = 0; i < current_bin_scan_done.size(); i++)
        {
            if (current_bin_scan_done.at(i) == false)
                {
                    move_next_freq = false;
                }

        }
        if (move_next_freq == true)
        {
            for (unsigned int i = 0; i < noise_power_estimations.at(current_freq_bin_idx).size(); i++)
                {
                    if(noise_power_estimations.at(current_freq_bin_idx).at(i) == 0.0)
                    {
                        noise_power_estimations.at(current_freq_bin_idx) = sdrjd_engine->get_noise_power();
                        break;
                    }
                }

            make_and_send_decision(&current_jammers[0]);
            
            for (unsigned int i = 0; i < current_bin_scan_done.size(); i++)
            {
                current_bin_scan_done.at(i) = false;
            }
            current_freq_bin_idx++;
            if (current_freq_bin_idx == freq_bins_hz.size()) current_freq_bin_idx = 0;

            try{
                bool reset_noise = false;
                for (unsigned int i = 0; i < noise_power_estimations.at(current_freq_bin_idx).size(); i++)
                {
                    if(noise_power_estimations.at(current_freq_bin_idx).at(i) == 0.0)
                    {
                        reset_noise = true;
                        break;
                    }
                }
                if (reset_noise)
                {
                    sdrjd_engine->reset_noise_estimations();
                }else{
                    sdrjd_engine->set_noise_power(noise_power_estimations.at(current_freq_bin_idx));
                }

            }catch(std::exception &e)
            {
                std::cout << "unexpected error 4bis!" << std::endl;
            }
                            
            try{
                signal_source->set_rf_freq(freq_bins_hz.at(current_freq_bin_idx));
                std::cout<< "RF frequency set to " << freq_bins_hz.at(current_freq_bin_idx) << " [Hz]\n";
                usleep(50000);// This delay is set in order to avoid transitories when frequency changes
		        usleep(sdrjd_engine->processing_delay_us);
                sdrjd_engine->set_detectors_frequency(freq_bins_hz.at(current_freq_bin_idx));
                sdrjd_engine->reset_detectors();
                sdrjd_engine->reset_valve();
            }catch(std::exception &e)
            {
                std::cout << "unexpected error 5!" << std::endl;
            }
        }
    }else{
        //single frequency, just reenable detectors after finished all the detection processes
        bool reenable_detectors = true;
        for (unsigned int i = 0; i < current_bin_scan_done.size(); i++)
        {
            if (current_bin_scan_done.at(i) == false)
                {
                    reenable_detectors = false;
                }
        }
        if (reenable_detectors == true)
        {
            make_and_send_decision(&current_jammers[0]);

            for (unsigned int i = 0; i < current_bin_scan_done.size(); i++)
            {
                current_bin_scan_done.at(i) = false;
            }

            try{
		       usleep(sdrjd_engine->processing_delay_us);
               sdrjd_engine->reset_detectors();
               sdrjd_engine->reset_valve();
            }catch(std::exception &e)
            {
                std::cout << "unexpected error 6!" << std::endl;
            }
         }
    }

}

bool detector_msg_receiver::set_search_space(double start_freq_hz_, double stop_freq_hz_, double step_hz_)
{
    std::cout<<"Start frequency "<< start_freq_hz_ <<" [Hz]\n";
    std::cout<<"Stop frequency "<< stop_freq_hz_ <<" [Hz]\n";
    start_freq_hz = start_freq_hz_;
    stop_freq_hz = stop_freq_hz_;
    step_hz = step_hz_;
    if (start_freq_hz > stop_freq_hz) return false;
    freq_bins_hz.clear();
    current_bin_scan_done.clear();
    noise_power_estimations.clear();

    double current_bin_hz = 0.0;
    if ((stop_freq_hz - start_freq_hz) < step_hz)
        current_bin_hz = start_freq_hz + std::floor((stop_freq_hz - start_freq_hz)/2.0);
    else
        current_bin_hz = start_freq_hz + std::floor(step_hz_/2.0);

    unsigned int MAX_DETECTORS=4;
    for (unsigned int num_detectors=0; num_detectors < MAX_DETECTORS ;num_detectors++)
    {
        current_bin_scan_done.push_back(false);
    }
    do{
        freq_bins_hz.push_back(current_bin_hz);
        std::cout << "Added new scan frequency " << current_bin_hz <<" [Hz]\n";
        std::vector<bool> det_status_vec;
        std::vector<float> det_noise_vec;
        //initialize vectors for jammer status in all bins
        for (unsigned int num_detectors=0; num_detectors < MAX_DETECTORS ;num_detectors++)
        {
            det_status_vec.push_back(false);
            if (num_detectors != 2) //All detectors except CW
                det_noise_vec.push_back(0.0);
        }
        last_jammer_detection_status.push_back(det_status_vec);
        noise_power_estimations.push_back(det_noise_vec);
        current_bin_hz += step_hz_;
    }while(current_bin_hz - std::floor(step_hz_/2.0) < stop_freq_hz);
    current_freq_bin_idx = 0;
    return true;
}

detector_msg_receiver::detector_msg_receiver() :
        gr::block("detector_msg_receiver", gr::io_signature::make(0, 0, 0), gr::io_signature::make(0, 0, 0))
{

    time(&last_detection_time);
    jammer_reporting_interval_s=1;

    this->message_port_register_in(pmt::mp("jammers"));
    this->set_msg_handler(pmt::mp("jammers"), boost::bind(&detector_msg_receiver::msg_handler_events, this, _1));


    std::string dump_filename("SDRJD_");
    dump_filename.append(currentDateTime());
    dump_filename.append(".log");
    log_file=fopen (dump_filename.c_str(),"w");
    if (log_file!=NULL)
    {
        std::cout << "SDRJD detection log enabled on file " << dump_filename.c_str() << std::endl;
    }else{
        std::cout << "Write error on file " << dump_filename.c_str() << std::endl;
        throw new std::exception();
    }

    //Create Sys V message queue
    sysv_msg_key=1101;
    int msgflg = IPC_CREAT | 0666;
    if ((sysv_msqid = msgget(sysv_msg_key, msgflg )) == -1) {
        std::cout<<"SDRJD can not create message queues!\n";
        perror("msgget");
        throw new std::exception();
    }

}

detector_msg_receiver::~detector_msg_receiver()
{}
