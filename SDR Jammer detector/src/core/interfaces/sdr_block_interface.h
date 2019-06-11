/*!
 * \file sdr_block_interface.h
 * \brief This interface represents a SDR block.
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


#ifndef SDR_BLOCK_INTERFACE_H
#define SDR_BLOCK_INTERFACE_H

#include <cassert>
#include <string>
#include <gnuradio/top_block.h>

/*!
 * \brief This abstract class represents an interface to SDR blocks.
 *
 * Abstract class for SDR block interfaces. Since all its methods are virtual,
 * this class cannot be instantiated directly, and a subclass can only be
 * instantiated directly if all inherited pure virtual methods have been
 * implemented by that class or a parent class.
 */
class SDRBlockInterface
{
public:
    virtual ~SDRBlockInterface()
    {}
    virtual std::string role() = 0;
    virtual std::string implementation() = 0;
    virtual size_t item_size() = 0;
    virtual void connect(gr::top_block_sptr top_block) = 0;
    virtual void disconnect(gr::top_block_sptr top_block) = 0;

    virtual gr::basic_block_sptr get_left_block() = 0;
    virtual gr::basic_block_sptr get_right_block() = 0;

    virtual gr::basic_block_sptr get_left_block(int RF_channel)
    {
        assert(RF_channel >= 0);
        if (RF_channel == 0){}; // avoid unused param warning
        return nullptr; // added to support raw array access (non pure virtual to allow left unimplemented)= 0;
    }
    virtual gr::basic_block_sptr get_right_block(int RF_channel)
    {
        assert(RF_channel >= 0);
        if (RF_channel == 0){};  // avoid unused param warning
        return nullptr; // added to support raw array access (non pure virtual to allow left unimplemented)= 0;
    }
};

#endif /*SDR_BLOCK_INTERFACE_H*/
