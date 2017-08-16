/*!
 * \file sdrjd.h
 * \brief Flowgraph engine of the SDR Jammer Detector.
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

#ifndef GNSS_SDR_SDRJD_H
#define GNSS_SDR_SDRJD_H

#include <gnuradio/msg_queue.h>
#include <gnuradio/block.h>
#include "file_configuration.h"
#include "jammer_detector_interface.h"
#include "new_valve.h"

/*!
 * \brief This class implements the flowgraph engine of the SDR Jammer Detector.
 *
 */
class sdrjd {
private:
    std::shared_ptr<ConfigurationInterface> configuration;
    gr::top_block_sptr top_block;
    bool stop_;
    bool run_continuous;
    new_valve_sptr valve;
    std::shared_ptr<JammerDetectorInterface> detector1;
    std::shared_ptr<JammerDetectorInterface> detector2;
    std::shared_ptr<JammerDetectorInterface> detector3;
    std::shared_ptr<JammerDetectorInterface> detector4;
    
    /*!
     * \brief Listens the control key for stoping the flowgraph
     *
     */
    void keyboard_listener();

public:

    /*!
     * \brief Delay (in microseconds) between algorithms iterations
     *
     */
    long int processing_delay_us;
    
    /*!
     * \brief Sets the configuration data
     *
     */
    void set_configuration(std::shared_ptr<ConfigurationInterface> config);

    /*!
     * \brief GNURadio msg events handler
     *
     */
    void msg_handler_events(pmt::pmt_t msg);

    /*!
     * \brief Capture signal from source and process it
     *
     */
    bool sdrjd_capture_and_process();
    
    /*!
     * \brief Stop the flowgraph
     *
     */
    void stop();

    /*!
     * \brief Restarts all the algorithms in every iteration
     *
     */
    void reset_detectors();

    /*!
     * \brief Reset the valve block in order to supply samples to the detectors
     *
     */
    void reset_valve();

    /*!
     * \brief Set the frequency to be analized for all the algorithms
     *
     */
    void set_detectors_frequency(double freq_hz);

    /*!
     * \brief Set d_noise_floor_estimated to false for Pulsed, WB and LFM algorithms
     *
     */
    void reset_noise_estimations();

    /*!
     * \brief Set d_noise_power estimation for Pulsed, WB and LFM algorithms
     *
     */
    void set_noise_power(std::vector<float> noise_power);

    /*!
     * \brief Get noise power estimations for Pulsed, WB and LFM algorithms
     *
     */
    std::vector<float> get_noise_power();

    /*!
     * \brief Default constructor
     *
     */
    sdrjd();

    /*!
     * \brief Default destructor
     *
     */
    ~sdrjd();

};


#endif //GNSS_SDR_SDRJD_H
