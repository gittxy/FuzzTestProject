/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

#if defined(_WIN32)
#define close closesocket
#endif

enum {
    TCP,
    RTU
};

int main(int argc, char *argv[])
{
    int s = -1;
    modbus_t *ctx = NULL;
    modbus_mapping_t *mb_mapping = NULL;
    int rc;
    int use_backend;
    ctx = modbus_new_tcp("127.0.0.1", 1502);
    s = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &s);
    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == NULL) {
        modbus_free(ctx);
        return -1;
    }
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    memset(query, 0, MODBUS_TCP_MAX_ADU_LENGTH);
    rc = modbus_receive(ctx, query); // 获取客户端的请求数据
    if (rc > 0) {
    	modbus_reply(ctx, query, rc, mb_mapping); // 处理并响应之
    } 
    modbus_mapping_free(mb_mapping);
    if (s != -1) {
        close(s);
    }
    /* For RTU, skipped by TCP (no TCP connect) */
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
