/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/time.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <modbus.h>

#define G_MSEC_PER_SEC 1000

static uint32_t gettime_ms(void)
{
    struct timeval tv;
#if !defined(_MSC_VER)
    gettimeofday(&tv, NULL);
    return (uint32_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return GetTickCount();
#endif
}

enum {
    TCP,
    RTU
};

/* Tests based on PI-MBUS-300 documentation */
int main(int argc, char *argv[])
{
    uint8_t *tab_bit;
    uint16_t *tab_reg;
    modbus_t *ctx;
    int i;
    int nb_points;
    double elapsed;
    uint32_t start;
    uint32_t end;
    uint32_t bytes;
    uint32_t rate;
    int rc;
    int n_loop;
    int use_backend;

    ctx = modbus_new_tcp("127.0.0.1", 1502);
    if (modbus_connect(ctx) == -1) {
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the status */
    tab_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));
    memset(tab_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    tab_reg = (uint16_t *) malloc(MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));
    memset(tab_reg, 0, MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

    nb_points = MODBUS_MAX_READ_BITS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_read_bits(ctx, 0, nb_points, tab_bit);
        if (rc == -1) {
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);
    
    bytes = n_loop * (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
   
    /* TCP: Query and response header and values */
    bytes = 12 + 9 + (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
  
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);

    nb_points = MODBUS_MAX_READ_REGISTERS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_read_registers(ctx, 0, nb_points, tab_reg);
        if (rc == -1) {
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);

    bytes = n_loop * nb_points * sizeof(uint16_t);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    
    /* TCP:Query and response header and values */
    bytes = 12 + 9 + (nb_points * sizeof(uint16_t));
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);

    nb_points = MODBUS_MAX_WR_WRITE_REGISTERS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_write_and_read_registers(ctx,
                                             0, nb_points, tab_reg,
                                             0, nb_points, tab_reg);
        if (rc == -1) {
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);

    bytes = n_loop * nb_points * sizeof(uint16_t);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);

    /* TCP:Query and response header and values */
    bytes = 12 + 9 + (nb_points * sizeof(uint16_t));
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    /* Free the memory */
    free(tab_bit);
    free(tab_reg);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
