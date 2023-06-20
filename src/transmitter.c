/*
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

extern "C" { // Necessary because C++ does "name wrangling", which breaks these includes. 

#include <stdbool.h>    // Give our C code macro names for 'bool', 'true', and 'false'
#include <stdio.h>      // Get printf() and file-reading capabilities
#include <stdlib.h>     // For calling exit() and strtol()
#include <unistd.h>     // Get access to the 'getopt()' function, a common tool for processing user-arguments
#include <signal.h>


#include <srsran/phy/rf/rf.h> // For accessing the USRP
#include <srsran/phy/utils/debug.h> // for the ERROR messages
#include <srsran/phy/common/timestamp.h>
#include <srsran/phy/common/phy_common_sl.h>
// #include <srsran/srsran.h>
// #include <srsran/phy/phch/sci.h>
#include "ue_sl.h"

}
/**
 * The overall structure of the program should be:
 * - Initialize variables with defaults
 * - Read user-provided parameters and overwrite variables accordingly
 * - Create a sidelink resource pool
 * - Tune the radio to the correct frequency and sampling rate
 * - Create a sidelink "vue" (virtual Ue?)
 * - Prepare TX data
 * - Transmit the messsage, according to the number of times and the delay-between-messages specified
*/


// === Program argument-related code ===

/**
 * The user parameters I'd like to have are:
 * -m : Message body (in hex)
 * -i : input .csv file with messages to send
 * -t : time between messages (in ms)
*/

/**
 * Define a data structure to contain the arguments set by the user.
 * (i.e. a "class" without any methods)
*/
typedef struct {
    char* rf_args;
    char* message_body;
    char* input_csv_name;
    int ms_between_messages;
    double rf_freq;
    float rf_gain;
} prog_args_t;

/**
 * Takes this program's argument object (prog_args_t) and initializes it to default values.
*/
void args_default(prog_args_t* args) {
    args->rf_args = NULL;
    args->message_body = NULL;
    args->input_csv_name = NULL;
    args->ms_between_messages = 10;
    args->rf_freq = 5915000000; // i.e. 5.915 GHz, the default frequency for our purposes.
    args->rf_gain = 50;
}

// Create a global args object for storing user/default arguments, but 'static' to make it 'private' to other files.
static prog_args_t prog_args;

/**
 * Take an argument object, the standard `argc` and `argv` passed to main(), and mutate that argument object to contain whatever was specified by the user.
*/
void parse_args(prog_args_t* args, int argc, char** argv) {
    int option;
    args_default(args);

    while ((option = getopt(argc, argv, "a:m:i:t:")) != -1) {
        switch(option) {
            case 'a':
                args->rf_args = optarg;
            case 'i':
                args->input_csv_name = optarg;
                break;
            case 'm':
                args->message_body = optarg; //optarg is a special variable set by getopt() that points at the value of a provided argument.
                break;
            case 't':
                // optind is a special var set by getopt() that is the index of the next element of the argv array
                // strtol converts a string to an integer long. I've specified a NULL object to store leftover bits in, and the number-base 10.
                // https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm
                args->ms_between_messages = (int)strtol(argv[optind], NULL, 10);
                break;
            //TODO - Add args for rf_freq
            default:
                printf("Unknown parameter provided: %c\n", option);
                exit(-1);
        }
    }
    if (args->message_body == NULL && args->input_csv_name == NULL) {
        printf("Error: Please specify either a message body (in hex) with `-m` or an input .csv with `-i`\n");
        exit(-1);
    }
}

// Running flag for our main program loop. Set to false upon Ctrl-C or other interrupt so the code can exit gracefully.
bool keep_running = true;

void signal_interrupt_handler(int signal_number) {
    printf("SIGINT received. Exiting...\n");
    if (signal_number == SIGINT) {
        keep_running = false;
    }
    else if (signal_number == SIGSEGV) {
        exit(-1);
    }
}

/**
 * Originally written by Eckermann. Appears to get a starting time from the radio.
*/
void get_start_time(srsran_rf_t* rf, srsran_timestamp_t* t)
{
  uint32_t start_time_full_ms;
  double   start_time_frac_ms;

  srsran_rf_get_time(rf, &t->full_secs, &t->frac_secs);

  fprintf(stdout, "start time: %f\n", srsran_timestamp_real(t));
  fflush(stdout);

  // make sure the fractional transmit time is ms-aligned
  start_time_full_ms = floor(t->frac_secs * 1e3);
  start_time_frac_ms = t->frac_secs - (start_time_full_ms / 1e3);
  if (start_time_frac_ms > 0.0) {
    srsran_timestamp_sub(t, 0, start_time_frac_ms);
  }
  // Add computing-time offset
  srsran_timestamp_add(t, 0, 3 * 1e-3);
}

// TODO - Define a method that can read a `.csv` file and store the contents into an array of hex values

// TODO - Define some kind of helper function to convert the hex into an array of 1's and 0's

// === Primary code ===
int main(int argc, char** argv) {
    
    // Setup signal handling to exit the program gracefully when user hits Ctrl-C to quit.
    signal(SIGINT, signal_interrupt_handler);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    
    parse_args(&prog_args, argc, argv);

    printf("Arguments parsed\n");
    printf("message_body is: %s\n", prog_args.message_body);
    printf("input_csv_name is: %s\n", prog_args.input_csv_name);
    printf("ms_between_messages is: %i\n", prog_args.ms_between_messages);

    //TODO - If an input_csv_name specified, read the .csv, otherwise read the message_body


    //Create a celular sidelink object with some default parameters
    srsran_cell_sl_t cell_sl = {
        .tm = SRSRAN_SIDELINK_TM4,  //tm is probably Transmission Mode 4 (where paramaters are self-selected without EnodeB's governance)
        .N_sl_id = 19,               // Not sure what this is either
        .nof_prb = 100,             // number of physical resource blocks. Should be 50 if 10 MHz channel, 100 if 20 MHz channel.
        .cp = SRSRAN_CP_NORM,       // Not sure what CP is. Copied Twardokus
    };

    //Create a sidelink resource pool and initialize with default parameters.
    srsran_sl_comm_resource_pool_t sl_comm_resource_pool;
    if (srsran_sl_comm_resource_pool_get_default_config(&sl_comm_resource_pool, cell_sl)) {
        ERROR("Error initializing sl_comm_resource_pool\n");
        return SRSRAN_ERROR;
    }
    
    //Attempt to find and connect to a radio (in our case, the EttusResearch USRP X410), passing in any provided arguments.
    srsran_rf_t radio;
    printf("Opening RF device...\n");
    if (srsran_rf_open(&radio, prog_args.rf_args)) {
        printf("Error opening rf\n");
        exit(-1);
    }

    printf("Attempting to set TX gain with prog_args of: %f\n", prog_args.rf_gain);

    //- Attempt to tune the radio to the user-provided frequency and sampling rate
    printf("Set TX freq: %.6f MHz\n",
         srsran_rf_set_tx_freq(&radio, 0, prog_args.rf_freq) / 1e6);
    srsran_rf_set_tx_gain(&radio, prog_args.rf_gain);
    printf("Set TX gain: %.1f dB\n", srsran_rf_get_tx_gain(&radio));
    
    int srate = srsran_sampling_freq_hz(cell_sl.nof_prb);
    if (srate != -1) {
        fprintf(stdout, "Setting sampling rate %.2f MHz\n", (float)srate / 1000000);
        fflush(stdout);
        float srate_rf = srsran_rf_set_tx_srate(&radio, (double)srate);
        if (srate_rf != srate) {
        ERROR("Could not set sampling rate\n");
        exit(-1);
        }
    } else {
        ERROR("Invalid number of PRB %d\n", cell_sl.nof_prb);
        exit(-1);
    }
    sleep(1);

    //TODO - Create a sidelink "vue" (Virtual Ue?)
    // (i.e. the special object designed by Eckermann)
    srsran_ue_sl_t srsue_vue_sl;
    srsran_ue_sl_init(&srsue_vue_sl, cell_sl, sl_comm_resource_pool, 0);

    //TODO - === Prepare TX data ===
    
    //- Initialize Sidelink Control Information
    //- (function definition is in ue_sl.c line 351)
    //- `&srsue_vue_sl.sc_tx` - store result in the transmit portion of this sidelink object
    //- `1` = "priority"
    //- `REP_INTERVL` (default was 100) = "Resource reservation interval, in ms" TODO - LOOK UP WHAT THIS IS
    //- `0` = "time gap"
    //- `false` = "retransmission" TODO - FIXME - It's possible we need to set this (and the accompanying transmission format) if this is about re-sending the same message with a 3ms delay
    //- `0` = "transmission format" - 0 sets to: "rate-matching and TBS scaling", 1 sets to: "puncturing and no TBS scaling"
    //- `4` = "mcs index" - Modulation and Coding Scheme index
    srsran_set_sci(&srsue_vue_sl.sci_tx, 1, 100, 3, true, 0, 4);
    
    uint8_t transport_block[SRSRAN_SL_SCH_MAX_TB_LEN] = {};
    //uint8_t transport_block[320] = {};

    // TODO - my_v2x_message needs to be assigned the value provided from earlier. (currently using this hard-coded value)
    uint8_t my_v2x_message[320] = {0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,1,1,1,0,0,0,0,1,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,1,1,0,1,1,0,1,0,0,1,0,0,1,0,1,0,0,1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,0,0,0,1,0,1,0,1,0,0,0,1,1,0,1,1,1,1,0,1,1,0,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,1,0,1,1,0,0,1,0,0,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,0,1,0,1,1,1,0,1,1,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,1,1,0,0,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,0,1,1,1,0,0,0,1,1,1,0,0,0};
    for (int i = 0; i < 319; i++) {
        transport_block[i] = my_v2x_message[i];
    }
    // for (int i = 320; i < srsue_vue_sl.pssch_tx.sl_sch_tb_len; i++) {
    //     transport_block[i] = 0;
    // }

    srsran_pssch_data_t data;
    data.ptr = transport_block;

    srsran_sl_sf_cfg_t sf; //- sf probably stands for "subframe", so Sidelink Subframe Configuration

    printf("creating signal buffer...\n");

    cf_t* signal_buffer_tx[1] = {}; //- Create a "signal buffer" of size 1 (for now)

    //-?? Probably "allocate memory for our subframe's data"
    signal_buffer_tx[0] = srsran_vec_cf_malloc(srsue_vue_sl.sf_len);
    if (!signal_buffer_tx[0]) {
        perror("malloc");
        exit(-1);
    }

    data.sub_channel_start_idx = 0; //Default to "0" for now...
    data.l_sub_channel = 2; //Default to "2". Still not totally sure what this is. current guess is "l" is "length". So the "length" of the sub channel "array", i.e. the number of subchannels in our channel we'd like to be writing to.

    //- tti is probably "transmission time interval". I thought this was 1ms but in Eckermann's code, it is from 0 to 100.
    //- It's possible that this time interval is a specific time duration, and is based off of some base time.
    sf.tti = 0;

    //- Attempt to encode a sidelink mesage (probably storing it in srsue_vue_sl) using our subframe (sf) and data.
    //-   The source code seems to deal with both the shared and control channel stuff.
    if (srsran_ue_sl_encode(&srsue_vue_sl, &sf, &data)) { 
        ERROR("Error encoding sidelink\n");
        exit(-1);
    }

    //- Take our encoded information and copy it over into a transmission buffer, which we will pull from when we want to send a message.
    memcpy(signal_buffer_tx[0], srsue_vue_sl.signal_buffer_tx, sizeof(cf_t) * srsue_vue_sl.sf_len);

    //TODO - Transmit the message, according to the number of times and the delay-between-messages specified
    
    // === Timing ===
    srsran_timestamp_t startup_time, tx_time, now;

    get_start_time(&radio, &startup_time); //- Retrieve the starting time from the radio and store it in &startup_time

    uint32_t tx_sec_offset = 0;
    uint32_t tx_msec_offset = 0;

    while (keep_running) {
        srsran_timestamp_copy(&tx_time, &startup_time);                         //- Overwrite tx_time with start_time. Why? Not sure. Possibly this value gets modified by `srsran_rf_send_timed2()`
        srsran_timestamp_add(&tx_time, tx_sec_offset, tx_msec_offset * 1e-3);   //- Add a specific amount of offset to our start_time. Offset gets progressively larger as `while (keep_running)` loops.

        srsran_rf_get_time(&radio, &now.full_secs, &now.frac_secs); //- Get the current time from the radio and store it in `now`

        // Check if tx_time is in the past. If so, reset time.
        if (srsran_timestamp_uint64(&now, srate) > srsran_timestamp_uint64(&tx_time, srate)) {
            //- This is cause for an error because it indicates the code ran super slow since tx_time was last calculated.
            //- We need this so we don't attempt to schedule a transmission with the radio at a time that is in the past.
            ERROR("tx_time is in the past (tx_time: %f, now: %f). Setting new start time.\n", 
                srsran_timestamp_real(&tx_time), srsran_timestamp_real(&now));
            get_start_time(&radio, &startup_time);

            tx_sec_offset = 0;
            tx_msec_offset = 0;
        }
        // Things look good, proceed with scheduling transmission
        else {
            int tx_result = srsran_rf_send_timed2(&radio,
                                            signal_buffer_tx[0], //-"data"
                                            srsue_vue_sl.sf_len, //-"nsamples"
                                            tx_time.full_secs, // -"secs"
                                            tx_time.frac_secs, //-"frac_secs"
                                            true,              //-"is_start_of_burst"
                                            true);             //-"is_end_of_burst"
            if (tx_result < 0) {
            ERROR("Error sending data: %d\n", tx_result);
            }
        }


        //- Increment transmission time offsets, which will be added to tx_time (after being reset to our start_time) to determine our next scheduled transmission time
        tx_msec_offset += 100;
        if (tx_msec_offset == 1000) {
            tx_sec_offset++;
            tx_msec_offset = 0;
        }
    }

    // Close connections to the USRP radio and free up memory.
    srsran_rf_close(&radio);
    srsran_ue_sl_free(&srsue_vue_sl);

    free(signal_buffer_tx[0]);

    return SRSRAN_SUCCESS;
}