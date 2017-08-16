/*!
 * \file sdr_signal_processing.cc
 * \brief This library implements various functions for process sdr signals
 * \authors Javier Arribas, 2016. jarribas(at)cttc.es
 *          Luis Esteve, 2016. luis(at)epsilon-formacion.com
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

#include "sdr_signal_processing.h"
#include <volk/volk.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/fft/fft.h>
#include <glog/logging.h>


void diff(float* _signal_out, const float* _signal_in,
                 unsigned int _size)
{
    unsigned int num_points = _size - 1;
    float* delayed_input = static_cast<float*>(volk_malloc(num_points * sizeof(float), volk_get_alignment()));
    memcpy(delayed_input, _signal_in, sizeof(float) * num_points);
    volk_32f_x2_subtract_32f(_signal_out, &_signal_in[1], delayed_input, num_points);
    volk_free(delayed_input);

    return;
}


void periodogram(float* _signal_out,
                 const std::complex<float>* _signal_in, unsigned int _size,
                 gr::filter::firdes::win_type _window)
{
    gr::fft::fft_complex* fft_ = new gr::fft::fft_complex(_size, true);
    float* magnitude_squared = static_cast<float*>(volk_malloc(_size * sizeof(float), volk_get_alignment()));
//    float* periodogram = static_cast<float*>(volk_malloc(_size * sizeof(float), volk_get_alignment()));

    float* window = static_cast<float*>(volk_malloc(_size * sizeof(float), volk_get_alignment()));
    static std::vector<float> win_taps = gr::filter::firdes::window(_window, _size, 0.5);
    float* w = &win_taps[0];

    DLOG(INFO) << "First taps of window: " << win_taps[0] << " " << win_taps[1] << " " << win_taps[2] << " " << win_taps[3];
    //DLOG(INFO) << "First samples of signal: " << _signal_in[0] << " " << _signal_in[1] << " " << _signal_in[2] << " " << _signal_in[3];
    volk_32fc_32f_multiply_32fc(fft_->get_inbuf(), _signal_in, w, _size);
    //gr_complex* aux;
    //aux = fft_->get_inbuf();
    //DLOG(INFO) << "First samples after mul: " << aux[0] << " " << aux[1] << " " << aux[2] << " " << aux[3];
    fft_->execute();
    //aux = fft_->get_outbuf();
    //DLOG(INFO) << "First samples after fft: " << aux[0] << " " << aux[1] << " " << aux[2] << " " << aux[3];

    volk_32fc_magnitude_squared_32f(magnitude_squared, fft_->get_outbuf(), _size);
    //DLOG(INFO) << "First samples after mag: " << magnitude_squared[0] << " " << magnitude_squared[1] << " " << magnitude_squared[2] << " " << magnitude_squared[3];

    volk_32f_s32f_multiply_32f(_signal_out, magnitude_squared, static_cast<const float>(1.0/_size), _size);
    //DLOG(INFO) << "First samples after per: " << _signal_out[0] << " " << _signal_out[1] << " " << _signal_out[2] << " " << _signal_out[3];
    
//    memcpy(_signal_out, periodogram, sizeof(float) * _size); //TODO: check if can avoid this memcpy

    volk_free(window);
//    volk_free(periodogram);
    volk_free(magnitude_squared);
    delete fft_;

    return;
}

void spectrogram(std::complex<float>** _signal_out,
                 const std::complex<float>* _signal_in, unsigned int _size,
                 gr::filter::firdes::win_type _window, unsigned int _fft_samples, 
                 unsigned int _overlap)
{   
    gr::fft::fft_complex* sft = new gr::fft::fft_complex(_fft_samples, true);
    gr_complex* signal = static_cast<gr_complex*>(volk_malloc(_fft_samples * sizeof(gr_complex), volk_get_alignment()));
    float* window = static_cast<float*>(volk_malloc(_fft_samples * sizeof(float), volk_get_alignment()));

    static std::vector<float> win_taps = gr::filter::firdes::window(_window, _fft_samples, 0.5);
    
    float* w = &win_taps[0];

    unsigned int _num_of_bins = floor((2*_size)/static_cast<double>(_fft_samples))-1; //TODO: put in function of overlap
    unsigned int _displ = _fft_samples - _overlap;

    for(unsigned int segment = 0; segment < _num_of_bins; segment++)
    {
        memcpy(signal, &_signal_in[segment*_displ], sizeof(gr_complex) * _fft_samples);
        volk_32fc_32f_multiply_32fc(sft->get_inbuf(), signal, w, _fft_samples);
        sft->execute();
        memcpy(_signal_out[segment], sft->get_outbuf(), sizeof(gr_complex) * _fft_samples);
    }

    delete sft;
    volk_free(signal);
    volk_free(window);

    return;
}