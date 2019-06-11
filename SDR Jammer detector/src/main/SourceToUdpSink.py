#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Top Block
# Generated: Mon Jun 12 17:20:55 2017
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import osmosdr

#Works (tested) on Ubuntu (x86_64 and aarch64) and CentOS (aarch64) distributions.

class SourceUdpSource(gr.top_block):

    def __init__(self, addr, port, block, samplerate, filepath, repeat, hackrf, freq):
        gr.top_block.__init__(self, "SourceUdpSource")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = samplerate
        self.block_size = block_size = block
        self.ip_addr = ip_addr = addr
        self.port = port
        self.path = filepath
        self.repeat = repeat
        self.hackrf = hackrf 
        self.freq = freq 

        ##################################################
        # Blocks
        ##################################################
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_gr_complex*1, ip_addr, port, block_size, False)

        if hackrf:
            self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + "" )
            self.osmosdr_source_0.set_time_source("gpsdo", 0)
            self.osmosdr_source_0.set_sample_rate(samp_rate)
            self.osmosdr_source_0.set_center_freq(freq, 0)
            self.osmosdr_source_0.set_freq_corr(0, 0)
            self.osmosdr_source_0.set_dc_offset_mode(0, 0)
            self.osmosdr_source_0.set_iq_balance_mode(0, 0)
            self.osmosdr_source_0.set_gain_mode(False, 0)
            self.osmosdr_source_0.set_gain(0, 0)
            self.osmosdr_source_0.set_if_gain(20, 0)
            self.osmosdr_source_0.set_bb_gain(20, 0)
            self.osmosdr_source_0.set_antenna("", 0)
            self.osmosdr_source_0.set_bandwidth(0, 0)
            ##################################################
            # Connections
            ##################################################
            self.connect((self.osmosdr_source_0, 0), (self.blocks_udp_sink_0, 0))
        else:
            self.blocks_throttle_0_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate, True)
            self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, filepath, repeat)
            ##################################################
            # Connections
            ##################################################
            self.connect((self.blocks_file_source_0, 0), (self.blocks_throttle_0_0, 0))    
            self.connect((self.blocks_throttle_0_0, 0), (self.blocks_udp_sink_0, 0)) 

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0_0.set_sample_rate(self.samp_rate)

    def get_block_size(self):
        return self.block_size

    def set_block_size(self, block_size):
        self.block_size = block_size


def main(options):
    tb = SourceUdpSource(options.addr, options.port, options.block, options.samplerate, options.filepath, options.repeat, options.hackrf, options.freq)
    tb.start()
    tb.wait()


if __name__ == '__main__':
    DEST_IP_DEFAULT = "127.0.0.1"
    DEST_PORT_DEFAULT = 1234
    BLOCK_SIZE_DEFAULT = 2944
    FILE_PATH_DEFAULT = "./captured_data/demo_jammer1_2412MHz_5Msps.dat"
    SAMPLE_RATE_DEFAULT = 5000000
    REPEAT_DEFAULT = False
    USE_HACKRF_DEFAULT = False
    FREQUENCY_DEFAULT = 2412000000
    parser = OptionParser()
    parser.add_option("-a", "--addr", dest="addr", default=DEST_IP_DEFAULT, help="IP address of the destination (SDR Jammer Detector application). Default: "+DEST_IP_DEFAULT)
    parser.add_option("-p", "--port", dest="port", type="int", default=DEST_PORT_DEFAULT, help="Port of the destination. Default: "+str(DEST_PORT_DEFAULT))
    parser.add_option("-b", "--block", dest="block", type="int", default=BLOCK_SIZE_DEFAULT, help="Block size to transfer to the destination. Default: "+str(BLOCK_SIZE_DEFAULT))
    parser.add_option("-s", "--samplerate", dest="samplerate", type="int", default=SAMPLE_RATE_DEFAULT, help="Sample rate. Default: "+str(SAMPLE_RATE_DEFAULT))
    parser.add_option("-f", "--filepath", dest="filepath", default=FILE_PATH_DEFAULT, help="File path to the dump file that will be transferred. ONLY USED IF NO -H OPTION IS PRESENT. Default: "+FILE_PATH_DEFAULT)
    parser.add_option("-r", "--repeat", action="store_true", dest="repeat", default=REPEAT_DEFAULT, help="Wheter to repeat continuously transferring the file or not. Default: "+str(REPEAT_DEFAULT))
    parser.add_option("-H", "--hackrf", action="store_true", dest="hackrf", default=USE_HACKRF_DEFAULT, help="Wheter to use HackRF One as input instead of dump file. Default: "+str(USE_HACKRF_DEFAULT))
    parser.add_option("-F", "--freq", dest="freq", type="int", default=FREQUENCY_DEFAULT, help="Frequency to set HackRF One (ONLY USED IF -H OPTION IS NOT PRESENT). Default: "+str(FREQUENCY_DEFAULT))
    (options, args) = parser.parse_args()
    print options
    main(options)
