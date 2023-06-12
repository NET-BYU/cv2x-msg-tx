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

#include <stdbool.h>    // Give our C code macro names for 'bool', 'true', and 'false'
#include <stdio.h>      // Get printf() and file-reading capabilities
#include <stdlib.h>     // For calling exit() and strtol()
#include <unistd.h>     // Get access to the 'getopt()' function, a common tool for processing user-arguments


#include "/usr/include/srsran/phy/common/phy_common_sl.h"
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
    char* message_body;
    char* input_csv_name;
    int ms_between_messages
} prog_args_t;

/**
 * Takes this program's argument object (prog_args_t) and initializes it to default values.
*/
void args_default(prog_args_t* args) {
    args->message_body = NULL;
    args->input_csv_name = NULL;
    args->ms_between_messages = 10;
}

// Create a global args object for storing user/default arguments, but 'static' to make it 'private' to other files.
static prog_args_t prog_args;

/**
 * Take an argument object, the standard `argc` and `argv` passed to main(), and mutate that argument object to contain whatever was specified by the user.
*/
void parse_args(prog_args_t* args, int argc, char** argv) {
    int option;
    args_default(args);

    while ((option = getopt(argc, argv, "m:i:t:")) != -1) {
        switch(option) {
            case 'm':
                args->message_body = optarg; //optarg is a special variable set by getopt() that points at the value of a provided argument.
                break;
            case 'i':
                args->input_csv_name = optarg;
                break;
            case 't':
                // optind is a special var set by getopt() that is the index of the next element of the argv array
                // strtol converts a string to an integer long. I've specified a NULL object to store leftover bits in, and the number-base 10.
                // https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm
                args->ms_between_messages = (int)strtol(argv[optind], NULL, 10);
                break;
            default:
                printf("Unknown parameter provided: %s\n", option);
                exit(-1);
        }
    }
    if (args->message_body == NULL && args->input_csv_name == NULL) {
        printf("Error: Please specify either a message body (in hex) or an input .csv containing message bodies");
        exit(-1);
    }
}

// TODO - Define a method that can read a `.csv` file and store the contents into an array of hex values

// TODO - Define some kind of helper function to convert the hex into an array of 1's and 0's

// === Primary code ===
int main(int argc, char** argv) {
    
    parse_args(&prog_args, argc, argv);

    printf("Arguments parsed\n");
    printf("message_body is: %s\n", prog_args.message_body);
    printf("input_csv_name is: %s\n", prog_args.input_csv_name);
    printf("ms_between_messages is: %i\n", prog_args.ms_between_messages);

}