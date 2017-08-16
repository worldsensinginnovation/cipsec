/*!
 * \file cw_jammer_detector.h
 * \brief Adapts a Continuous Wave (cw) Jammer Detector block to a
 * JammerDetectorInterface
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

#ifndef CW_JAMMER_DETECTOR_H_
#define CW_JAMMER_DETECTOR_H_

#include <string>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/float_to_complex.h>
#include "jammer_detector_interface.h"
#include "cw_jammer_detector_cc.h"
#include "complex_byte_to_float_x2.h"



class ConfigurationInterface;

/*!
 * \brief This class adapts a CW Jammer Detector block to
 *  a JammerDetectorInterface
 */

class CWJammerDetector: public JammerDetectorInterface
{
public:
    CWJammerDetector(ConfigurationInterface* configuration,
            std::string role, unsigned int in_streams,
            unsigned int out_streams);

    virtual ~CWJammerDetector();

    std::string role()
    {
        return role_;
    }

    /*!
     * \brief Returns "CW_Jammer_Detector"
     */
    std::string implementation()
    {
        return "CW_Jammer_Detector";
    }
    size_t item_size()
    {
        return item_size_;
    }

    void set_threshold(float threshold);


    void connect(gr::top_block_sptr top_block);
    void disconnect(gr::top_block_sptr top_block);
    gr::basic_block_sptr get_left_block();
    gr::basic_block_sptr get_right_block();

    /*!
     * \brief Initializes detection algorithm.
     */
    void init();


    /*!
     * \brief Restart detection algorithm
     */
    void reset();

    /*!
     * \brief If state = 1, it forces the block to start detecting from the first sample
     */
    void set_state(int state);

    void set_rf_freq(double rf_freq_hz)
    {
        detector_cc_->set_rf_freq(rf_freq_hz);
    }

    void set_noise_estimation_done(bool)
    {}

    void set_noise_power_estimation(float)
    {}

    float get_noise_power_estimation()
    {}    

private:
    ConfigurationInterface* configuration_;
    cw_jammer_detector_cc_sptr detector_cc_;
    gr::blocks::stream_to_vector::sptr stream_to_vector_;
    size_t item_size_;
    std::string item_type_;
    unsigned int vector_length_;

    // A_ and B_ are variables of Tong algorithm used to decrease
    // false alarm probability. See Kaplan's book:
    // E. Kaplan, Ch. Hegarty "Understanding GPS: Principles and Applications"
    // Artech House, 2005, pp 223-227
    unsigned int A_;
    unsigned int B_;
    unsigned int max_dwells_;
    float threshold_;
    long fs_in_;
    bool dump_;
    std::string dump_filename_;
    std::string role_;
    unsigned int in_streams_;
    unsigned int out_streams_;
};

#endif /* CW_JAMMER_DETECTOR_H_ */
