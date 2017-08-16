/*!
 * \file sysv_msg_receiver.cc
 * \brief Example of standalone program message consumer
 * \authors <ul>
 *          <li> Javier Arribas, 2016. jarribas(at)cttc.es
 *          <li> Luis Esteve, 2016. luis(at)epsilon-formacion.com
 *          <ul>
 *
 * This is an example of standalone program message consumer. It has to 
 * be modified in order to manipulate messages consumed.
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

#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <iostream>
#include <unistd.h>
#include <thread>
#include <jammer_detector_msg.h>

#include <signal.h>

static volatile int keep_capturing = 1;

void intHandler(int dummy) {
    keep_capturing = 0;
}

void receive_msg()
{
    jammer_msgbuf msg;
    int msgrcv_size=sizeof(msg.jammer_msg);
    int msqid;
    key_t key=1101;

    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
        perror("JAMMER MSG QUEUE NOT AVAILABLE");
        exit(1);
    }
    jammer_detector_msg jammer;
    while (keep_capturing==1) {

        if (msgrcv(msqid, &msg, msgrcv_size, 1, 0) != -1)
        {
            jammer=msg.jammer_msg;
            struct tm  tstruct;
            char       buf[80];
            tstruct = *localtime(&jammer.timestamp);
            strftime(buf, sizeof(buf), "%d-%m-%Y-%H-%M-%S", &tstruct);

            switch(jammer.jammer_type) {
                case 1:

                    if(jammer.detection == true)
                    {
                        std::cout << "Pulsed jammer ON at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    else
                    {
                        std::cout << "Pulsed jammer OFF at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }

                    break;
                case 2:
                    if(jammer.detection == true)
                    {
                        std::cout << "WB jammer ON at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    else
                    {
                        std::cout << "WB jammer OFF at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    break;
                case 3:
                    if(jammer.detection == true)
                    {
                        std::cout << "CW jammer ON at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    else
                    {
                        std::cout << "CW jammer OFF at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    break;
                case 4:
                    if(jammer.detection == true)
                    {
                        std::cout << "LFM jammer ON at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    else
                    {
                        std::cout << "LFM jammer OFF at time " << buf << std::endl;
                        std::cout << "Jammer RF frequency: " << jammer.rf_freq_hz << " Hz" << std::endl;
                        std::cout << "Jammer JNR: " << jammer.jnr_db << " dB" << std::endl;
                        std::cout << "Jammer test value: " << jammer.test_value << std::endl;
                        std::cout << "Jammer samplecounter: " << jammer.sample_counter << std::endl;
                    }
                    break;                    
                default:
                    std::cout << "Unknown Jammer type"<<std::endl;
            }
        }

    }

    std::cout<<"RECEIVER MSG THREAD STOP.\n";
    return;
}

int main(int argc, char **argv)
{
    std::cout<<"SYS V JAMMER MESSAGE RECEIVER EXAMPLE v1.0\n";
    std::cout<<"PRESS CTRL + C to STOP\n";
    signal(SIGINT, intHandler);

    std::thread receive_msg_thread(receive_msg);

    while (keep_capturing==1)
    {
        sleep(1);
    }
    std::cout<<"Stop recording data!\n";

    receive_msg_thread.join();
    return 0;
}


