/*!
 * \file sdr_signal_processing.h
 * \brief This library implements various functions for process sdr signals
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

#ifndef SDR_SIGNAL_PROCESSING_H_
#define SDR_SIGNAL_PROCESSING_H_

#include <complex>
#include <gnuradio/filter/firdes.h>


/*!
 * \brief This function difference and approximates derivative of a signal.
 *
 */
void diff(float* _signal_out, const float* _signal_in,
                 unsigned int _size);

/*!
 * \brief This function generates periodogram of the incoming signal
 *
 */
void periodogram(float* _signal_out, const std::complex<float>* _signal_in,
                 unsigned int _size, gr::filter::firdes::win_type _window);

/*!
 * \brief This function generates spectrogram of the incoming signal
 *
 */
void spectrogram(std::complex<float>** _signal_out,
                 const std::complex<float>* _signal_in, unsigned int _size,
                 gr::filter::firdes::win_type _window,
                 unsigned int _fft_samples, unsigned int _overlap);


#endif /* SDR_SIGNAL_PROCESSING_H_ */
