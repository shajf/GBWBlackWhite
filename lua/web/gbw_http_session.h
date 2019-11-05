/*
 * =====================================================================================
 *
 *       Filename:  gbw_http_session.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/05/2019 04:55:27 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef GBW_HTTP_SESSION_H
#define GBW_HTTP_SESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct gbw_http_session_t gbw_http_session_t;

struct gbw_http_session_t {

     uint32_t srcIP;
     uint32_t dstIP;
     const char *method;
     const char *uri;
     const char *host;
 };

#endif /*GBW_HTTP_SESSION_H*/
