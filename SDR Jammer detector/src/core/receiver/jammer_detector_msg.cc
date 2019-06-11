/*!
 * \file jammer_detector_msg.cc
 * \brief Class that handles detection message structure
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

#include "jammer_detector_msg.h"


jammer_detector_msg::jammer_detector_msg()
{
    jammer_type=0; //Jammer type (1-Pulsed 2-WB 3-CW 4-FM)
    rf_freq_hz=0.0; //Jammer detected frequency
    jnr_db=0.0; //Jammer-to-Noise ratio dB
    test_value=0.0; // Test statistics value
    sample_counter=0;
    timestamp=0;
    detection=false;
}
