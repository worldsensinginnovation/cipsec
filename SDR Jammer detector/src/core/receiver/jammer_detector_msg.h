/*!
 * \file jammer_detector_msg.h
 * \brief Class that handles detection message structure
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

#ifndef SDRJD_JAMMER_DETECTOR_MSG_H
#define SDRJD_JAMMER_DETECTOR_MSG_H

#include <ctime>

/*!
 * \brief This class that handles detection message structure
 *
 */
class jammer_detector_msg {

public:

    int jammer_type; //Jammer type (1-Pulsed 2-WB 3-CW 4-FM)
    double rf_freq_hz; //Jammer detected frequency
    float jnr_db; //Jammer-to-Noise ratio dB
    float test_value; // Test statistics value
    long unsigned int sample_counter; //sample counter to track the reception time
    time_t timestamp; //jammer detection timestamp
    bool detection; //true if positive detection

/*!
 * Default constructor
 */
    jammer_detector_msg();

};

/*!
 * \brief This struct ads mtype to detection message structure
 *
 */
typedef struct  {
        long mtype;//required by sys v message
        jammer_detector_msg jammer_msg;
    } jammer_msgbuf;

#endif //SDRJD_JAMMER_DETECTOR_MSG_H
