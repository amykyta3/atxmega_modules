#ifndef FIXEDPT_H
#define FIXEDPT_H

/// Convert a floating point constant to fixed point QN
#define CONST_QN(x,N)   ((x)*(1ULL<<(N)))

/// Convert a floating point constant to fixed point Q0.8
#define CONST_Q8(x)     CONST_QN(x,8)

/// Convert a floating point constant to fixed point Q0.16
#define CONST_Q16(x)    CONST_QN(x,16)

/// Convert a floating point constant to fixed point Q0.32
#define CONST_Q32(x)    CONST_QN(x,32)

/// Scale an unsigned 8-bit value by an unsigned Q0.8
__inline__ uint8_t mpy_Q8(uint8_t x, uint8_t Q8){
    uint16_t P;
    P = (uint16_t)x * Q8;
    P >>= 8;
    return(P);
}

/// Scale an unsigned 16-bit value by an unsigned Q0.16
__inline__ uint16_t mpy_Q16(uint16_t x, uint16_t Q16){
    uint32_t P;
    P = (uint32_t)x * Q16;
    P >>= 16;
    return(P);
}

/// Scale an unsigned 32-bit value by an unsigned Q0.32
__inline__ uint32_t mpy_Q32(uint32_t x, uint32_t Q32){
    uint64_t P;
    P = (uint64_t)x * Q32;
    P >>= 32;
    return(P);
}

/// Scale a signed 8-bit value by an unsigned Q0.8
__inline__ int8_t mpys_Q8(int8_t x, uint8_t Q8){
    int16_t P;
    P = (int16_t)x * Q8;
    P >>= 8;
    return(P);
}

/// Scale a signed 16-bit value by an unsigned Q0.16
__inline__ int16_t mpys_Q16(int16_t x, uint16_t Q16){
    int32_t P;
    P = (int32_t)x * Q16;
    P >>= 16;
    return(P);
}

/// Scale a signed 32-bit value by an unsigned Q0.32
__inline__ int32_t mpys_Q32(int32_t x, uint32_t Q32){
    int64_t P;
    P = (int64_t)x * Q32;
    P >>= 32;
    return(P);
}

#endif
