/*!
 * \file new_valve.h
 * \brief  Interface of a GNU Radio block that sends a STOP message to the
 * control queue right after a specific number of samples have passed through it.
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
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


#ifndef NEW_VALVE_H_
#define NEW_VALVE_H_

#include <cstring>
#include <gnuradio/sync_block.h>
#include <boost/shared_ptr.hpp>

class new_valve;

typedef boost::shared_ptr<new_valve> new_valve_sptr;

new_valve_sptr new_make_valve (size_t sizeof_stream_item,
                                                  unsigned long long nitems);
/*!
 * \brief Implementation of a GNU Radio block that sends a STOP message to the
 * control queue right after a specific number of samples have passed through it.
 */
class new_valve : public gr::sync_block
{
    friend new_valve_sptr new_make_valve(size_t sizeof_stream_item,
                                                            unsigned long long nitems);
    new_valve (size_t sizeof_stream_item,
                    unsigned long long nitems);
    unsigned long long    d_nitems;
    unsigned long long    d_ncopied_items;

public:
    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items);
    void reset_valve()
    {
        d_ncopied_items=0;
    }
};

#endif /*NEW_VALVE_H_*/
