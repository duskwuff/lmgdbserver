/*
 * lmgdbserver 1.0
 *
 * Copyright (c) 2012 Dusk Wuff (dusk@woofle.net).
 * 
 * To the extent possible under law, the author has dedicated all copyright and
 * related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

#define LMICDI_VID 0x1cbe
#define LMICDI_PID 0x00fd

#define INTERFACE_NR 0x02
#define ENDPOINT_IN  0x83
#define ENDPOINT_OUT 0x02

libusb_device_handle *lph;
int trace = 0;
char buf[4096];
const char *appname;

void pump_gdb2icdi(int endpoint)
{
    int p = 0;
    int ch;

    do {
        ch = buf[p++] = fgetc(stdin);
        if (ch == -1) {
            libusb_close(lph);
            exit(0);
        }

        if (ch == 0x03 && p < 3)
            goto sendnow;

        if (p >= sizeof(buf) - 4) {
            fprintf(stderr, "ICDI ERROR: Line too long!!\n");
            exit(1);
        }
    } while (ch != '#');

    // Next two characters are the checksum
    buf[p++] = fgetc(stdin);
    buf[p++] = fgetc(stdin);

    buf[p] = 0;
    if (trace) fprintf(stderr, "ICDI <<- %s\n", buf);

sendnow: ;
    // Send it!
    int count = 0;
    int rc = libusb_bulk_transfer(lph, endpoint, (uint8_t *) buf, p, &count, 0);
    if (rc != 0 || count != p) {
        fprintf(stderr, "ICDI send error: %s\n", libusb_error_name(rc));
        exit(1);
    }
}

void pump_icdi2gdb(int endpoint)
{
    int count = 0;

again: ;
    int rc = libusb_bulk_transfer(lph, endpoint, (uint8_t *) buf, sizeof(buf), &count, 100);
    if (rc != 0) {
        fprintf(stderr, "ICDI recv error: %s\n", libusb_error_name(rc));
        return;
    }

    buf[count] = 0;
    if (trace) fprintf(stderr, "ICDI ->> %s\n", buf);

    fwrite(buf, count, 1, stdout);
    fflush(stdout);

    if(count == 1 && buf[0] == '+') goto again;
}

void usage()
{
    fprintf(stderr,
            "USAGE: (gdb) target remote | %s [opts]\n"
            "    -h : this message\n"
            "    -t : enable ICDI command tracing\n"
            "    -v : increase libusb verbosity\n"
            "    -q : decrease libusb verbosity\n"
            "    -D : specify alternate VID:PID\n"
            "         default: -D %04x:%04x\n"
            "    -P : specify alternate interface & in/out endpoints\n"
            "         default: -P %02x:%02x:%02x\n",
            appname,
            LMICDI_VID, LMICDI_PID,
            INTERFACE_NR, ENDPOINT_IN, ENDPOINT_OUT
            );
}

int main(int argc, char **argv)
{
    int lu_verbosity = 0;

    int icdi_vid = LMICDI_VID;
    int icdi_pid = LMICDI_PID;

    int icdi_interface = INTERFACE_NR;
    int icdi_ep_in  = ENDPOINT_IN;
    int icdi_ep_out = ENDPOINT_OUT;
    appname = argv[0];

    int ch;
    while ((ch = getopt(argc, argv, "htvqD:P:")) != -1) {
        switch (ch) {
            case 't':
                trace++;
                break;
            case 'h':
                usage();
                return 0;
            case 'v':
                lu_verbosity++;
                break;
            case 'q':
                lu_verbosity--;
                break;

            case 'D':
                if (sscanf(optarg, "%x:%x", &icdi_vid, &icdi_pid) != 2)
                    usage();
                break;
            case 'P':
                if (sscanf(optarg, "%x:%x:%x", &icdi_interface, &icdi_ep_in, &icdi_ep_out) != 3)
                    usage();
                break;

            default:
                usage();
                return 1;
        }
    }

    int rc;

    rc = libusb_init(NULL);
    if (rc != 0) {
        fprintf(stderr, "ICDI: libusb init failed: %s\n", libusb_error_name(rc));
        return 1;
    }

    libusb_set_debug(NULL, lu_verbosity);

    lph = libusb_open_device_with_vid_pid(NULL, icdi_vid, icdi_pid);
    if (lph == NULL) {
        fprintf(stderr, "ICDI: No LaunchPad found!\n");
        return 1;
    }

    rc = libusb_claim_interface(lph, icdi_interface);
    if (rc) {
        fprintf(stderr, "ICDI: Couldn't claim interface!\n");
        return 1;
    }

    fprintf(stderr, "ICDI connected\n");
    if (isatty(STDIN_FILENO))
        fprintf(stderr, "(Good luck using this thing interactively. You'll need it.)\n");

    for (;;) {
        pump_gdb2icdi(icdi_ep_out);
        pump_icdi2gdb(icdi_ep_in);
    }
}
