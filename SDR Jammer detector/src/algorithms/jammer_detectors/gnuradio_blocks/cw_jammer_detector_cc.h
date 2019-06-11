/*!
 * \file cw_jammer_detector_cc.h
 * \brief This class implements an algorithm to detect continuous jammers
 *
 *  Continuous Wave Jammer Detection strategy
 *  <ol>
 *  <li> Calculate Periodogram
 *  <li> Calculate the maximum
 *  <li> Calculate noise floor
 *  <li> Statistic = Maximum/Noise Floor
 *  <li> Compare with threshold
 *  <li> Declare positive or negative detection using a message
 *  </ol>
 *
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


#ifndef CW_JAMMER_DETECTOR_CC_H
#define CW_JAMMER_DETECTOR_CC_H

#include <fstream>
#include <string>
#include <gnuradio/block.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/fft/fft.h>
#include "jammer_detector_msg.h"


class cw_jammer_detector_cc;

typedef boost::shared_ptr<cw_jammer_detector_cc> cw_jammer_detector_cc_sptr;

cw_jammer_detector_cc_sptr
cw_jammer_make_detector_cc(long fs_in_hz,
                         unsigned int block_samples,
                         unsigned int A,
                         unsigned int B,
                         unsigned int max_dwells,
                         bool dump,
                         std::string dump_filename);

/*!
 * \brief This class implements a CW Jammer Detection algorithm.
 *
 */
class cw_jammer_detector_cc: public gr::block
{
private:
    friend cw_jammer_detector_cc_sptr
    cw_jammer_make_detector_cc(long fs_in_hz,
                                   unsigned int block_samples,
                                   unsigned int A,
                                   unsigned int B,
                                   unsigned int max_dwells,
                                   bool dump,
                                   std::string dump_filename);

    cw_jammer_detector_cc(long fs_in_hz,
                              unsigned int block_samples,
                              unsigned int A,
                              unsigned int B,
                              unsigned int max_dwells,
                              bool dump,
                              std::string dump_filename);

    long d_fs_in;
    double d_rf_freq;
    unsigned int d_block_samples;
    float d_threshold;
    jammer_detector_msg jammer;
    unsigned long int d_sample_counter;
    float* d_spectrum;
    float* d_spectrum_no_dc;
    float d_noise_floor;
    float d_test_statistics;
    float d_test_statistics_acum;
    bool d_active;
    int d_state;
    bool d_dump;
    std::ofstream d_dump_file;
    std::string d_dump_filename;
    // d_k, d_A and d_B are variables of Tong algorithm used to decrease
    // false alarm probability. See Kaplan's book:
    // E. Kaplan, Ch. Hegarty "Understanding GPS: Principles and Applications"
    // Artech House, 2005, pp 223-227
    unsigned int d_k; // Detection counter
    unsigned int d_A; // Final value of detector counter for positive detection
    unsigned int d_B; // Initial value of detector counter
    unsigned int d_dwell_counter;
    unsigned int d_max_dwells;

public:
    /*!
     * \brief Default destructor.
     */
     ~cw_jammer_detector_cc();


     void set_rf_freq(double rf_freq_hz)
     {
         d_rf_freq = rf_freq_hz;
     }

     /*!
      * \brief Initializes detection algorithm.
      */
     void init();

     /*!
      * \brief If set to 1, ensures that detection starts at the
      * first available sample.
      * \param state - int=1 forces start of detection
      */
     void set_state(int state);

    /*!
     * \brief Starts detection algorithm, turning from standby mode to
     * active mode
     * \param active - bool that activates/deactivates the block.
     */
    void set_active(bool active)
    {
        d_active = active;
    }


     /*!
      * \brief Set statistics threshold of detection algorithm
      * \param threshold - Threshold for jamming detection.
      */
     void set_threshold(float threshold)
     {
         d_threshold = threshold;
     }
     /*!
      * \brief CW Jammer Detector signal processing.
      */
     int general_work(int noutput_items, gr_vector_int &ninput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items);
};

#endif /* cw_jammer_detector_cc_H*/
