#include <stdio.h>
#include <stdint.h>

#define LITTLE_ENDIAN

typedef struct HuffmanTable_s {
    uint8_t symbols[128];
    uint16_t codes[128];
    uint8_t len;
} HuffmanTable_t;

HuffmanTable_t huffman_tables[4];

static uint16_t get_len(uint8_t** ptr) {
    uint16_t len;
#ifdef LITTLE_ENDIAN
    len = *(*ptr)++ << 8;
    len += *(*ptr)++;
#else
    len = *(*ptr)++;
    len += *(*ptr)++ << 8; /* not tested */
#endif
    return len;
}

void decode_huffman_table(uint8_t **ptr, uint16_t len) {
    /* Get class and destination */
    HuffmanTable_t *huffman_table;
    switch (*(*ptr)++) {
        case 0x00: /* Class 0 (DC), destination 0 (luminance) */
            huffman_table = &huffman_tables[0];
            break;
        case 0x01: /* Class 0 (DC), destination 0 (luminance) */
            huffman_table = &huffman_tables[1];
            break;
        case 0x10: /* Class 1 (AC), destination 0 (chrominance) */
            huffman_table = &huffman_tables[2];
            break;
        case 0x11: /* Class 1 (AC), destination 1 (chrominance) */
            huffman_table = &huffman_tables[3];
            break;
    }

    /* Generate codes */
    uint8_t i, j, code = 0, base = 0;
    for (i = 0; i < 16; i++) {
        for (j = base; j < base + *(*ptr); j++) {
            huffman_table->codes[j] = code;
            code += 1;
        }
        base += *(*ptr)++;
        code <<= 1;
    }

    /* Collect symbols */
    for (i = 0; i < j; i++) {
        huffman_table->symbols[i] = *(*ptr)++;
    }

    /* Save number of symbol-code pairs */
    huffman_table->len = j;
}

void print_huffman_tables(void) {
    for (int n = 0; n < 4; n++) {
        printf("Huffman table: %d\n", n);
        for (int i = 0; i < huffman_tables[n].len; i++) {
            printf("%02x | %02x\n",  huffman_tables[n].codes[i],  huffman_tables[n].symbols[i]);
        }
    }
}

int main() {
    int i = 0, j = 0, n = 0;
    size_t sz = 0;
    unsigned char image[2000];
    FILE *fp;
    uint16_t len = 0;

    fp = fopen("../images/test.jpeg","rb");
    if (fp == NULL) {
        printf("Could not open the file\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);

    /* Read file to buffer 'image' */
    fread(image,sz,1,fp);

    unsigned char *ptr = image;

    /* Check if image is JPEG */
    if (*ptr == 0xFF && *(++ptr) == 0xD8) {
        printf("Image is JPEG\n");
    } else {
        printf("Image is not JPEG\n");
        return 0;
    }
    ptr++;

    /* Iterate over image */
    int eoi = 0;
    int iter = 0;
    while (eoi == 0) {
        if (*ptr++ == 0xFF) {
            switch (*ptr++) {
                case 0xe0: /* APP0 */
                case 0xe2: /* APP2 */
                case 0xfe: /* COM */
                    printf("APP0/APP2/COM\n");
                    len = get_len(&ptr);
                    ptr += len-2;
                    break;
                case 0xdb: /* DQT */
                    printf("DQT\n");
                    get_len(&ptr);
                    break;
                case 0xc0: /* SOF0 */
                    printf("SOF0\n");
                    get_len(&ptr);
                    break;
                case 0xc4: /* DHT */
                    len = get_len(&ptr);
                    printf("DHT - len = %d\n", len);
                    decode_huffman_table(&ptr, len);
                    break;
                case 0xda: /* SOS */
                    printf("SOS\n");
                    get_len(&ptr);
                    break;
                case 0xd9: /* EOI */
                    printf("EOI\n");
                    eoi = 1;
                    break;
            }
        }
        iter++;
    }

    print_huffman_tables();

    printf("Iterations: %d\n", iter);

//    /* Print file bytes */
//    for (i = 0; i < sz; i++) {
//        printf("%02x ", image[i]);
//    }
//    printf("\n\n");

    return 0;
}
