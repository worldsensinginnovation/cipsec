/*!
 * \file sdr_block_factory.h
 * \brief Interface of a factory that returns smart pointers to SDR blocks.
 * \authors <ul>
 *          <li> Javier Arribas, 2016. jarribas(at)cttc.es
 *          <li> Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *          <ul>
 *
 * This class encapsulates the complexity behind the instantiation
 * of SDR blocks.
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

#ifndef SDR_BLOCK_FACTORY_H_
#define SDR_BLOCK_FACTORY_H_

#include <memory>
#include <string>
#include <vector>
#include <gnuradio/msg_queue.h>

class ConfigurationInterface;
class SDRBlockInterface;

/*!
 * \brief Class that produces all kinds of SDR blocks
 */
class SDRBlockFactory
{
public:
    SDRBlockFactory();
    virtual ~SDRBlockFactory();
    std::unique_ptr<SDRBlockInterface> GetSignalSource(std::shared_ptr<ConfigurationInterface> configuration, int ID = -1);
    std::unique_ptr<SDRBlockInterface> GetJammerDetector(std::shared_ptr<ConfigurationInterface> configuration, int ID = -1);
    std::unique_ptr<SDRBlockInterface> GetSignalConditioner(std::shared_ptr<ConfigurationInterface> configuration, int ID = -1);

    /*
     * \brief Returns the block with the required configuration and implementation
     */
    std::unique_ptr<SDRBlockInterface> GetBlock(std::shared_ptr<ConfigurationInterface> configuration,
            std::string role, std::string implementation,
            unsigned int in_streams, unsigned int out_streams,
            boost::shared_ptr<gr::msg_queue> queue = nullptr);

private:
};

#endif /*SDR_BLOCK_FACTORY_H_*/

