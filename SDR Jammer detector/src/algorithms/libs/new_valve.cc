/*!
 * \file new_valve.cc
 * \brief Implementation of a GNU Radio block that sends a STOP message to the
 * control queue right after a specific number of samples have passed through it.
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "new_valve.h"
#include <algorithm> // for min
#include <gnuradio/io_signature.h>

new_valve::new_valve (size_t sizeof_stream_item,
                                unsigned long long nitems) : gr::sync_block("new_valve",
                                                                            gr::io_signature::make(1, 1, sizeof_stream_item),
                                                                            gr::io_signature::make(1, 1, sizeof_stream_item) ),
                                                             d_nitems(nitems), d_ncopied_items(0)
{}


new_valve_sptr new_make_valve (size_t sizeof_stream_item, unsigned long long nitems)
{
    new_valve_sptr valve_(new new_valve(sizeof_stream_item, nitems));
    return valve_;
}



int new_valve::work (int noutput_items,
                          gr_vector_const_void_star &input_items,
                          gr_vector_void_star &output_items)
{
    if (d_ncopied_items >= d_nitems)
    {
        consume_each(noutput_items);
        return 0;    // Done!
    }
    unsigned long long n = std::min(d_nitems - d_ncopied_items, (long long unsigned int)noutput_items);
    if (n == 0) return 0;
    memcpy (output_items[0], input_items[0], n * input_signature()->sizeof_stream_item(0));
    d_ncopied_items += n;
    return n;
}
