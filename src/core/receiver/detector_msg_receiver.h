/*!
 * \file detector_msg_receiver.h
 * \brief Handles messages of detection blocks and make decissions
 * \authors <ul>
 *          <li> Javier Arribas, 2016. jarribas(at)cttc.es
 *          <li> Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *          <ul>
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

#ifndef SDRJD_DETECTOR_MSG_RECEIVER_H
#define SDRJD_DETECTOR_MSG_RECEIVER_H

#include <gnuradio/block.h>
#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <vector>
#include "source_interface.h"
#include "jammer_detector_msg.h"
#include "sdrjd.h"


class detector_msg_receiver;


typedef boost::shared_ptr<detector_msg_receiver> detector_msg_receiver_sptr;
detector_msg_receiver_sptr detector_msg_receiver_make();

/*!
 * \brief This class performs control messages of the algorithms of detection
 *
 */
class detector_msg_receiver : public gr::block
{
    private:

        /*!
         * \brief Returns a string filled with the current Date Time
         *
         */
        const std::string currentDateTime();

        friend detector_msg_receiver_sptr detector_msg_receiver_make();

        /*!
         * \brief Handles mwessages send by the detectors
         *
         */        
        void msg_handler_events(pmt::pmt_t msg);

        /*!
         * \brief Default constructor
         *
         */
        detector_msg_receiver();
        
        FILE* log_file;
        time_t last_detection_time;
        int jammer_reporting_interval_s;

        /*!
         * \brief Sends information of jammer detection through Linux cue
         *
         */
        bool send_sys_v_msg(jammer_detector_msg jammer);

        /*!
         * \brief Writes information of jammer detection in log file
         *
         */
        bool write_new_log_entry(jammer_detector_msg* jammer);

        /*!
         * \brief Make the final decision of what jammers are ON or OFF
         *
         */
        void make_and_send_decision(jammer_detector_msg* current_jammers);
        key_t sysv_msg_key;
        int sysv_msqid;

        double start_freq_hz;
        double stop_freq_hz;
        double step_hz;
        bool enable_sweep;

        jammer_detector_msg current_jammers[4];

        /*!
         * \brief Vector of vectors containing the last status for all the detectors in all the frequency bins
         *
         */
        std::vector<std::vector<bool>> last_jammer_detection_status;
        
        /*!
         * \brief Vector that indicates if the particular detector has scanned the current frequency bin
         *
         */
        std::vector<bool> current_bin_scan_done;

        /*!
         * \brief Vector of vectors containing the noise power estimations of every frequency
         *
         */
        std::vector<std::vector<float>> noise_power_estimations;
        
        std::vector<double> freq_bins_hz;
        unsigned int current_freq_bin_idx;

        std::shared_ptr<SourceInterface> signal_source;

        sdrjd *sdrjd_engine;

    public:
        
        /*!
         * \brief Set a pointer to the the SDRJD flowgraph
         *
         */
        void set_sdrjd_engine(sdrjd *sdrjd_)
        {
            sdrjd_engine=sdrjd_;
        }

        /*!
         * \brief Set a pointer to the Signal Source in order to control the sweep frequency
         *
         */
        void set_signal_source(std::shared_ptr<SourceInterface> source)
        {
            signal_source=source;
        }

        /*!
         * \brief Calculates the search space of the frequency sweep
         *
         */
        bool set_search_space(double start_freq_hz_, double stop_freq_hz_, double step_hz_);

        /*!
         * \brief Sets the sweep frequency option
         *
         */
        void set_enable_sweep(bool enable_sweep_)
        {
            enable_sweep=enable_sweep_;
        }

        /*!
         * \brief Default destructor
         *
         */        
        ~detector_msg_receiver (); //!< Default destructor
};


#endif //SDRJD_DETECTOR_MSG_RECEIVER_H
