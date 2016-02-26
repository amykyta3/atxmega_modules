/*
* Copyright (c) 2016, Alexander I. Mykyta
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer. 
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <uart_io.h>
#include <string_ext.h>

void uart_put_x8(uint8_t num){
    char buf[3];
    snprint_x8(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_x16(uint16_t num){
    char buf[5];
    snprint_x16(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_x32(uint32_t num){
    char buf[9];
    snprint_x32(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_d8(uint8_t num){
    char buf[4];
    snprint_d8(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_d16(uint16_t num){
    char buf[6];
    snprint_d16(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_d32(uint32_t num){
    char buf[11];
    snprint_d32(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_sd8(int8_t num){
    char buf[5];
    snprint_sd8(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_sd16(int16_t num){
    char buf[7];
    snprint_sd16(buf, sizeof(buf), num);
    uart_puts(buf);
}

//------------------------------------------------------------------------------
void uart_put_sd32(int32_t num){
    char buf[12];
    snprint_sd32(buf, sizeof(buf), num);
    uart_puts(buf);
}
