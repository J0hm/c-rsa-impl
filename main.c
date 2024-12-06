#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 16

// multiplication in GF(2^8)
// TODO check if a or b == 2 or 3 and use gmul2 or gmul3
uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;

    for (int i = 0; i < 8; i++) {
        if (b & 1) {
            p = p ^ a;
        }

        uint8_t carry = a & 0x80;
        a = a << 1;
        if (carry) {
            a = a ^ 0x1b;
        }
        b = b >> 1;
    }

    return p;
}

uint8_t gmul2(uint8_t a) {
    uint8_t h = a & 0x80;
    uint8_t b = a << 1;

    if (h == 0x80) {
        b = b ^ 0x1b;
    }

    return b;
}

uint8_t gmul3(uint8_t a) { return a ^ gmul2(a); }

// 2^v in GF(2^8), TODO: replace with table
uint8_t rcon(uint8_t v) {
    uint8_t c = 1;
    if (v == 0) {
        return v;
    }

    for (int i = 0; i < v; i++) {
        c = gmul2(c);
    }

    return c;
}

// rotates the given word n bytes to the left
uint32_t rotate(uint32_t word, uint8_t n) { return (word << 8*n) | (word >> (32-8*n)); }

// substitution box table for Rijndael's
uint8_t sbox[16][16] = {{0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30,
                         0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
                        {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad,
                         0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
                        {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34,
                         0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
                        {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07,
                         0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
                        {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52,
                         0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
                        {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a,
                         0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
                        {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45,
                         0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
                        {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc,
                         0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
                        {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4,
                         0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
                        {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46,
                         0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
                        {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2,
                         0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
                        {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c,
                         0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
                        {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8,
                         0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
                        {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61,
                         0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
                        {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b,
                         0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
                        {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41,
                         0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

// maximum distance scattering matrix
uint8_t mds[4][4] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
};

// creates a 16-byte block with a given offset
uint8_t *makeBlock(char *input, uint32_t offset) {
    uint32_t offset_bytes = offset * BLOCK_SIZE;  // 16 bytes per block

    uint8_t *block = calloc(BLOCK_SIZE, sizeof(uint8_t));

    // TODO: this only works inputs that are a multiple of 16 bytes
    for (int i = 0; i < BLOCK_SIZE; i++) {
        block[4 * (i % 4) + (i / 4)] = input[offset_bytes + i];
    }

    return block;
}

void subBytes(uint8_t *block) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        block[i] = sbox[(block[i] & 0xf0) >> 4][block[i] & 0x0f];
    }
}

// pretty prints the given block
void printBlock(uint8_t *block) {
    for (int i = 1; i <= BLOCK_SIZE; i++) {
        printf("%02x ", block[i-1]);
        if(i%4==0) {
            printf("\n");
        }
    }
    printf("\n");
}

// shifts each row of the given block to the left by the row indice
// this can be improved to do it in place
void shiftRows(uint8_t *block) {
    for(int i = 0; i < 4; i++) {
        uint32_t row;
        memcpy(&row, &block[4*i], 4);
        uint32_t rotated = rotate(row, 4-i);
        memcpy(&block[4*i], &rotated, 4);
    }
}

void mixColumns(uint8_t *block) {
    uint8_t *res = calloc(BLOCK_SIZE, sizeof(uint8_t));

    for(int i = 0; i < 4; i++) {
        uint8_t column[4] = {block[i], block[i+4], block[i+8], block[i+12]};
        for(int j = 0; j < 4; j++) {
            res[i+4*j] = gmul(mds[j][0], column[0]) ^ gmul(mds[j][1], column[1]) ^ gmul(mds[j][2], column[2]) ^ gmul(mds[j][3], column[3]);
        }
    }

    memcpy(block, res, BLOCK_SIZE);
    free(res);
}

void addRoundKey(uint8_t *block, char *key) {
    for(int i = 0; i < BLOCK_SIZE; i++) {
        uint8_t blockIndex = 4 * (i % 4) + (i / 4);
        block[blockIndex] = block[blockIndex] ^ key[i];
    }
}

int main() {
    char *input = "The quick brown fox jumps over the lazy dog.";

    uint8_t *block = makeBlock(input, 0);
    printBlock(block);

    subBytes(block);
    printBlock(block);

    shiftRows(block);
    printBlock(block);

    mixColumns(block);
    printBlock(block);

    addRoundKey(block, "abcdefghijklmnop");
    printBlock(block);

    free(block);
    return (0);
}