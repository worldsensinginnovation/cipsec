/*!
 * \file jammer_detector_interface.h
 * \brief This interface represents a Jammer Detector block.
 * \authors <ul>
 *          <li> Javier Arribas, 2016. jarribas(at)cttc.es
 *          <li> Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *          <ul>
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

#ifndef JAMMER_DETECTOR_INTERFACE_H_
#define JAMMER_DETECTOR_INTERFACE_H_

#include "sdr_block_interface.h"

/*!
 * \brief This abstract class represents an interface to Jammer Detector blocks.
 *
 */
class JammerDetectorInterface: public SDRBlockInterface
{
public:
    virtual void set_threshold(float threshold) = 0;
    virtual void set_rf_freq(double rf_freq_hz) = 0;
    virtual void set_noise_estimation_done(bool done) = 0;
    virtual void set_noise_power_estimation(float noise_power) = 0; 
    virtual float get_noise_power_estimation() = 0;
    virtual void init() = 0;
    virtual void reset() = 0;
};

#endif
