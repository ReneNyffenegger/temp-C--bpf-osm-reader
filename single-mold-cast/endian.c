/* endianness */

#define BIG_ENDIAN      0
#define LITTLE_ENDIAN   1

static int determine_endianness() {

/* checks the current CPU endianness */
    readosm_endian4 endian4;
    endian4.bytes[0] = 0x01;
    endian4.bytes[1] = 0x00;
    endian4.bytes[2] = 0x00;
    endian4.bytes[3] = 0x00;

    if (endian4.uint32_value == 1) return LITTLE_ENDIAN;

    return BIG_ENDIAN;
}
