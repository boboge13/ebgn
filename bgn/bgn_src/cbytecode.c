/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "type.h"
#include "log.h"
#include "cmpic.inc"
#include "cbytecode.h"
#include "ccode.h"

static UINT32 dbg_hton_uint32(const UINT32 x)
{
    UINT32 y;
#if (32 == WORDSIZE)
    y = __bswap_32(x);
#endif /*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    y = __bswap_64(x);
#endif /*(64 == WORDSIZE)*/

    sys_log(LOGSTDOUT, "hton_uint32: %lx => %lx vs %lx => %lx\n", x, y, x, htonl(x));
    return (y);
}

static UINT32 dbg_ntoh_uint32(const UINT32 x)
{
    UINT32 y;

#if (32 == WORDSIZE)
    y = __bswap_32(x);
#endif /*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    y = __bswap_64(x);
#endif /*(64 == WORDSIZE)*/

    sys_log(LOGSTDOUT, "ntoh_uint32: %lx => %lx vs %lx => %lx\n", x, y, x, ntohl(x));
    return (y);
}

static UINT16 dbg_hton_uint16(const UINT16 x)
{
    UINT16 y;

    y = __bswap_16(x);
    sys_log(LOGSTDOUT, "hton_uint16: %x => %x vs %x => %x\n", x, y, x, htons(x));
    return (y);
}

static UINT16 dbg_ntoh_uint16(const UINT16 x)
{
    UINT16 y;

    y = __bswap_16(x);
    sys_log(LOGSTDOUT, "ntoh_uint16: %x => %x vs %x => %x\n", x, y, x, ntohs(x));
    return (y);
}

EC_BOOL cbytecode_pack_uint32(const UINT32 *in_buff, const UINT32 data_num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 *src_data;
    UINT32 *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(UINT32);
    if(end_pos > out_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_pack_uint32: would overflow where postion = %ld, data_num = %ld out_buff_max_len = %ld\n", (*position), data_num, out_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT32 *)(in_buff);
    des_data = (UINT32 *)(out_buff + (*position));
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = hton_uint32(*(src_data ++));
    }

    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_uint32_size(const UINT32 data_num, UINT32 *size)
{
    (*size) += (data_num * sizeof(UINT32));
    return (EC_TRUE);
}

EC_BOOL cbytecode_unpack_uint32(const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT32 *out_buff, const UINT32 data_num)
{
    UINT32 *src_data;
    UINT32 *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(UINT32);
    if(end_pos > in_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_unpack_uint32: would overflow where postion = %ld, data_num = %ld in_buff_max_len = %ld\n", (*position), data_num, in_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT32 *)(in_buff + (*position));
    des_data = (UINT32 *)(out_buff);
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = ntoh_uint32(*(src_data ++));
    }

    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_uint16(const UINT16 *in_buff, const UINT32 data_num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT16 *src_data;
    UINT16 *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(UINT16);
    if(end_pos > out_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_pack_uint16: would overflow where postion = %ld, data_num = %ld out_buff_max_len = %ld\n", (*position), data_num, out_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT16 *)in_buff;
    des_data = (UINT16 *)(out_buff + (*position));
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = hton_uint16(*(src_data ++));
    }

    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_uint16_size(const UINT32 data_num, UINT32 *size)
{
    (*size) += (data_num * sizeof(UINT16));
    return (EC_TRUE);
}

EC_BOOL cbytecode_unpack_uint16(const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT16 *out_buff, const UINT32 data_num)
{
    UINT16 *src_data;
    UINT16 *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(UINT16);
    if(end_pos > in_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_unpack_uint16: would overflow where postion = %ld, data_num = %ld in_buff_max_len = %ld\n", (*position), data_num, in_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT16 *)(in_buff + (*position));
    des_data = (UINT16 *)(out_buff);
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = ntoh_uint16(*(src_data ++));
    }

    (*position) = end_pos;
    return (EC_TRUE);
}


EC_BOOL cbytecode_pack_uint8(const UINT8 *in_buff, const UINT32 data_num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT8  *src_data;
    UINT8  *des_data;
    UINT32  end_pos;

    end_pos = (*position) + data_num;
    if(end_pos > out_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_pack_uint8: would overflow where postion = %ld, data_num = %ld out_buff_max_len = %ld\n", (*position), data_num, out_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT8 *)(in_buff);
    des_data = (UINT8 *)(out_buff + (*position));
#if 0
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = hton_uint8(*(src_data ++));
    }
#endif
#if 1
    //sys_log(LOGSTDOUT, "[DEBUG] pack: %lx (%ld) => %lx (%ld), len %ld\n", src_data, (((UINT32)src_data)& 0x3), des_data, (((UINT32)des_data)& 0x3), data_num);
    memcpy(des_data, src_data, data_num);
#endif
    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_uint8_size(const UINT32 data_num, UINT32 *size)
{
    (*size) += (data_num);
    return (EC_TRUE);
}

EC_BOOL cbytecode_unpack_uint8(const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *out_buff, const UINT32 data_num)
{
    UINT8  *src_data;
    UINT8  *des_data;
    UINT32  end_pos;

    end_pos = (*position) + data_num;

    if(end_pos > in_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_unpack_uint8: would overflow where postion = %ld, data_num = %ld in_buff_max_len = %ld\n", (*position), data_num, in_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT8 *)(in_buff + (*position));
    des_data = (UINT8 *)(out_buff);
#if 0
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = ntoh_uint8(*(src_data ++));
    }
#endif
#if 1
    //sys_log(LOGSTDOUT, "[DEBUG] unpack: %lx (%ld) => %lx (%ld), len %ld\n", src_data, (((UINT32)src_data)& 0x3), des_data, (((UINT32)des_data)& 0x3), data_num);
    memcpy(des_data, src_data, data_num);
#endif
    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_real(const REAL *in_buff, const UINT32 data_num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32   *src_data;
    UINT32   *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(REAL);
    if(end_pos > out_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_pack_real: would overflow where postion = %ld, data_num = %ld out_buff_max_len = %ld\n", (*position), data_num, out_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT32 *)(in_buff);
    des_data = (UINT32 *)(out_buff + (*position));
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = hton_uint32(*(src_data ++));
    }

#if (32 == WORDSIZE)/*sizeof(REAL) = 2 *sizeof(UINT32)*/
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = hton_uint32(*(src_data ++));
    }
#endif/*(32 == WORDSIZE)*/

    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack_real_size(const UINT32 data_num, UINT32 *size)
{
    (*size) += (data_num * sizeof(REAL));
    return (EC_TRUE);
}

EC_BOOL cbytecode_unpack_real(const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, REAL *out_buff, const UINT32 data_num)
{
    UINT32 *src_data;
    UINT32 *des_data;
    UINT32  data_idx;
    UINT32  end_pos;

    end_pos = (*position) + data_num * sizeof(REAL);
    if(end_pos > in_buff_max_len)
    {
        sys_log(LOGSTDOUT, "error:cbytecode_pack_real_size: would overflow where postion = %ld, data_num = %ld in_buff_max_len = %ld\n", (*position), data_num, in_buff_max_len);
        return (EC_FALSE);
    }

    src_data = (UINT32 *)(in_buff + (*position));
    des_data = (UINT32 *)(out_buff);
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = ntoh_uint32(*(src_data ++));
    }

#if (32 == WORDSIZE)/*sizeof(REAL) = 2 *sizeof(UINT32)*/
    for(data_idx = 0; data_idx < data_num; data_idx ++)
    {
        *(des_data ++) = ntoh_uint32(*(src_data ++));
    }
#endif/*(32 == WORDSIZE)*/

    (*position) = end_pos;
    return (EC_TRUE);
}

EC_BOOL cbytecode_pack(const UINT8 *in_buff, const UINT32 data_num, const UINT32 data_type, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position, const UINT32 comm)
{
    switch(data_type)
    {
        case CMPI_ULONG:
            return cbytecode_pack_uint32((UINT32 *)in_buff, data_num, out_buff, out_buff_max_len, position);
        case CMPI_USHORT:
            return cbytecode_pack_uint16((UINT16 *)in_buff, data_num, out_buff, out_buff_max_len, position);
        case CMPI_UCHAR:
            return cbytecode_pack_uint8((UINT8 *)in_buff, data_num, out_buff, out_buff_max_len, position);
        case CMPI_REAL:
            return cbytecode_pack_real((REAL *)in_buff, data_num, out_buff, out_buff_max_len, position);
    }

    sys_log(LOGSTDOUT, "error:cbytecode_pack: unknown data_type %ld\n", data_type);
    return (EC_FALSE);
}

EC_BOOL cbytecode_unpack(const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *out_buff, const UINT32 data_num, const UINT32 data_type, const UINT32 comm)
{
    switch(data_type)
    {
        case CMPI_ULONG:
            return cbytecode_unpack_uint32(in_buff, in_buff_max_len, position, (UINT32 *)out_buff, data_num);
        case CMPI_USHORT:
            return cbytecode_unpack_uint16(in_buff, in_buff_max_len, position, (UINT16 *)out_buff, data_num);
        case CMPI_UCHAR:
            return cbytecode_unpack_uint8(in_buff, in_buff_max_len, position, (UINT8 *)out_buff, data_num);
        case CMPI_REAL:
            return cbytecode_unpack_real(in_buff, in_buff_max_len, position, (REAL *)out_buff, data_num);
    }

    sys_log(LOGSTDOUT, "error:cbytecode_unpack: unknown data_type %ld\n", data_type);
    return (EC_FALSE);
}

EC_BOOL cbytecode_pack_size(const UINT32 data_num, const UINT32 data_type, UINT32 *size, const UINT32 comm)
{
    switch(data_type)
    {
        case CMPI_ULONG:
            return cbytecode_pack_uint32_size(data_num, size);
        case CMPI_USHORT:
            return cbytecode_pack_uint16_size(data_num, size);
        case CMPI_UCHAR:
            return cbytecode_pack_uint8_size(data_num, size);
        case CMPI_REAL:
            return cbytecode_pack_real_size(data_num, size);
    }

    sys_log(LOGSTDOUT, "error:cbytecode_pack_size: unknown data_type %ld\n", data_type);
    return (EC_FALSE);
}

int cbyte_code_test()
{
    UINT8   uint8_buff[]  = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    UINT16  uint16_buff[] = {0xe102, 0xe304, 0xe506, 0xe708, 0xe90a, 0xea0b, 0xeb0c, 0xec0d, 0xee0f, 0xc1c2, 0xc3c4, 0xc5c6, 0xc7c8, 0xc9ca, 0xcbcc};

    UINT32  uint32_buff[] = {0xf1020304, 0xf5060708, 0xf90a0b0c, 0xfd0e0fc1, 0xc2c3c4c5, 0xc6c7c8c9, 0xcacbcccd};

    REAL    real_buff[] = {0.1234, 0.5678, 112233.0, 445566.789};

    UINT8   encoding_buff[1024];

    UINT8   decode_uint8_buff[64];
    UINT16  decode_uint16_buff[64];
    UINT32  decode_uint32_buff[64];
    REAL    decode_real_buff[64];

    UINT32  uint8_data_num  = sizeof(uint8_buff)/sizeof(uint8_buff[0]);
    UINT32  uint16_data_num = sizeof(uint16_buff)/sizeof(uint16_buff[0]);
    UINT32  uint32_data_num = sizeof(uint32_buff)/sizeof(uint32_buff[0]);
    UINT32  real_data_num   = sizeof(real_buff)/sizeof(real_buff[0]);

    UINT32  encoding_buff_max_len = sizeof(encoding_buff)/sizeof(encoding_buff[0]);
    UINT32  encoding_buff_len;

    UINT32  position;

    init_host_endian();
    print_host_endian(LOGSTDOUT);

    sys_log(LOGSTDOUT, "--------------------------------------------------------------------------\n");
    sys_log(LOGSTDOUT, "original uint8 buff:\n");
    print_uint8_buff(LOGSTDOUT, uint8_buff, uint8_data_num);

    position = 0;
    cbytecode_pack((UINT8 *)uint8_buff, uint8_data_num, CMPI_UCHAR, encoding_buff, encoding_buff_max_len, &position, CMPI_ERROR_COMM);

    encoding_buff_len = position;

    sys_log(LOGSTDOUT, "encoding buff:\n");
    print_uint8_buff(LOGSTDOUT, encoding_buff, encoding_buff_len);

    position = 0;
    cbytecode_unpack(encoding_buff, encoding_buff_len, &position, (UINT8 *)decode_uint8_buff, uint8_data_num, CMPI_UCHAR, CMPI_ERROR_COMM);

    sys_log(LOGSTDOUT, "decoding result:\n");
    print_uint8_buff(LOGSTDOUT, decode_uint8_buff, uint8_data_num);

    sys_log(LOGSTDOUT, "--------------------------------------------------------------------------\n");
    sys_log(LOGSTDOUT, "original uint16 buff:\n");
    print_uint16_buff(LOGSTDOUT, uint16_buff, uint16_data_num);
    print_uint8_buff(LOGSTDOUT, (UINT8 *)uint16_buff, uint16_data_num * sizeof(UINT16));

    position = 0;
    cbytecode_pack((UINT8 *)uint16_buff, uint16_data_num, CMPI_USHORT, encoding_buff, encoding_buff_max_len, &position, CMPI_ERROR_COMM);

    encoding_buff_len = position;

    sys_log(LOGSTDOUT, "encoding buff:\n");
    print_uint8_buff(LOGSTDOUT, encoding_buff, encoding_buff_len);

    position = 0;
    cbytecode_unpack(encoding_buff, encoding_buff_len, &position, (UINT8 *)decode_uint16_buff, uint16_data_num, CMPI_USHORT, CMPI_ERROR_COMM);

    sys_log(LOGSTDOUT, "decoding result:\n");
    print_uint16_buff(LOGSTDOUT, decode_uint16_buff, uint16_data_num);

    sys_log(LOGSTDOUT, "--------------------------------------------------------------------------\n");
    sys_log(LOGSTDOUT, "original uint32 buff:\n");
    print_uint32_buff(LOGSTDOUT, uint32_buff, uint32_data_num);
    print_uint8_buff(LOGSTDOUT, (UINT8 *)uint32_buff, uint32_data_num * sizeof(UINT32));
    position = 0;
    cbytecode_pack((UINT8 *)uint32_buff, uint32_data_num, CMPI_ULONG, encoding_buff, encoding_buff_max_len, &position, CMPI_ERROR_COMM);

    encoding_buff_len = position;

    sys_log(LOGSTDOUT, "encoding buff:\n");
    print_uint8_buff(LOGSTDOUT, encoding_buff, encoding_buff_len);

    position = 0;
    cbytecode_unpack(encoding_buff, encoding_buff_len, &position, (UINT8 *)decode_uint32_buff, uint32_data_num, CMPI_ULONG, CMPI_ERROR_COMM);

    sys_log(LOGSTDOUT, "decoding result:\n");
    print_uint32_buff(LOGSTDOUT, decode_uint32_buff, uint32_data_num);

    sys_log(LOGSTDOUT, "--------------------------------------------------------------------------\n");
    sys_log(LOGSTDOUT, "original real buff:\n");
    print_real_buff(LOGSTDOUT, real_buff, real_data_num);
    print_uint8_buff(LOGSTDOUT, (UINT8 *)real_buff, real_data_num * sizeof(REAL));

    position = 0;
    cbytecode_pack((UINT8 *)real_buff, real_data_num, CMPI_REAL, encoding_buff, encoding_buff_max_len, &position, CMPI_ERROR_COMM);

    encoding_buff_len = position;

    sys_log(LOGSTDOUT, "encoding buff:\n");
    print_uint8_buff(LOGSTDOUT, encoding_buff, encoding_buff_len);

    position = 0;
    cbytecode_unpack(encoding_buff, encoding_buff_len, &position, (UINT8 *)decode_real_buff, real_data_num, CMPI_REAL, CMPI_ERROR_COMM);

    sys_log(LOGSTDOUT, "decoding result:\n");
    print_real_buff(LOGSTDOUT, decode_real_buff, real_data_num);
    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

