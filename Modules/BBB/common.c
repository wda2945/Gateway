/*
Copyright (c) 2013 Adafruit

Original RPi.GPIO Author Ben Croston
Modified for BBIO Author Justin Cooper

This file incorporates work covered by the following copyright and 
permission notice, all modified code adopts the original license:

Copyright (c) 2013 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#define __USE_BSD
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

//#include "syslog/syslog.h"
#include "common.h"

#define PATHLEN 50

int build_path(const char *partial_path, const char *prefix, char *full_path, size_t full_path_len)
{
    DIR *dp;
    struct dirent *ep;

    dp = opendir (partial_path);
    if (dp != NULL) {
        while ((ep = readdir (dp))) {
            // Enforce that the prefix must be the first part of the file
            char* found_string = strstr(ep->d_name, prefix);

            if (found_string != NULL && (ep->d_name - found_string) == 0) {
                snprintf(full_path, full_path_len, "%s/%s", partial_path, ep->d_name);
                (void) closedir (dp);
                return 1;
            }
        }
        (void) closedir (dp);
    } else {
        return 0;
    }

    return 0;
}

//1 on success, 0 on fail
int load_device_tree(const char *name)
{
    FILE *file = NULL;
    char line[256];

    file = fopen("/sys/devices/platform/bone_capemgr/slots", "r+");
    if (!file) {
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        //if the device is already loaded, return 1
        if (strstr(line, name)) {
            fclose(file);
            return 1;
        }
    }

    //if the device isn't already loaded, load it, and return
    fprintf(file,"%s",name);
    fclose(file);

    //0.2 second delay
    usleep(200000);

    //check
    file = fopen("/sys/devices/platform/bone_capemgr/slots", "r+");
    if (!file) {
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        //if the device is loaded, return 1
        if (strstr(line, name)) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int unload_device_tree(const char *name)
{
    FILE *file = NULL;
    char line[256];
    char *slot_line;

    file = fopen("/sys/devices/platform/bone_capemgr/slots", "r+");
    if (!file) {
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        //the device is loaded, let's unload it
        if (strstr(line, name)) {
            slot_line = strtok(line, ":");
            //remove leading spaces
            while(*slot_line == ' ')
                slot_line++;

            fprintf(file, "-%s", slot_line);
            fclose(file);
            return 1;
        }
    }

    //not loaded, close file
    fclose(file);
    return 1;
}


int set_pinmux(const char *key, const char *mode)
{
	char ocp_dir[60];
	char path[60]; // "/sys/devices/platform/ocp/ocp:P#_##_pinmux/state"
	char pinmux_dir[20]; // "ocp:P#_##_pinmux"
	char pin[6]; //"P#_##"
	FILE *f = NULL;
	
	if (strlen(key) == 4)	// Key P#_# format, must inject '0' to be P#_0#
		snprintf(pin, sizeof(pin), "%.3s0%c", key,key[3]);
	else	//copy string
		snprintf(pin, sizeof(pin), "%s", key);

	strncpy(ocp_dir, "/sys/devices/platform/ocp", sizeof(ocp_dir));

	snprintf(pinmux_dir, sizeof(pinmux_dir), "ocp:%s_pinmux", pin);
	snprintf(path, sizeof(path), "%s/%s/state", ocp_dir, pinmux_dir);

	f = fopen(path, "w");
	if (NULL == f) {
		return 0;
	} 

	fprintf(f, "%s", mode);
	fclose(f);

	return 1;
}
