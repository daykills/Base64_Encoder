#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h> // *should* typedef uint8_t
// Check that uint8_t type exists
#ifndef UINT8_MAX
#error "No support for uint8_t"
#endif

#define ENDING_CHAR_SIZE 1
#define INPUT_BATCH_SIZE 3
#define OUTPUT_BATCH_SIZE 4
#define CHAR_SIZE 8

int myceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

void process(uint8_t* input_buffer, unsigned long input_buffer_size, uint8_t* output_buffer) {
    static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/=";
    uint8_t bin[INPUT_BATCH_SIZE * CHAR_SIZE] = { 0 }; //init all members are 0
    int i_bin = 0;
    int count_bits = 0;
    for (unsigned long i = 0; i < input_buffer_size; i++)
    {
        //        printf("encode: %c\n", input_buffer[i]);
        for (int i_bit = CHAR_SIZE - 1; i_bit >= 0; i_bit--) {
            bin[i_bin++] = input_buffer[i] >> i_bit & 0x01;
            count_bits ++;
        }
    }
    const int encoded_bit_size = CHAR_SIZE * INPUT_BATCH_SIZE / OUTPUT_BATCH_SIZE;
    double count_padding_val =  (double)count_bits / (double)encoded_bit_size;
    int count_padding = myceil(count_padding_val);
    
    for (int i = 0; i < OUTPUT_BATCH_SIZE; i++) {
        output_buffer[i] = 0;
        if (i == count_padding)
        {
            output_buffer[i] = '=';
            count_padding ++;
            continue;
        }
        for (int i_bit = 0; i_bit < encoded_bit_size; i_bit++) {
            output_buffer[i]  = (output_buffer[i] << 1) + bin[i_bit + i * encoded_bit_size];
        }
        //        printf("output_buffer: %u\n", output_buffer[i]);
        output_buffer[i] = alphabet[output_buffer[i]];
        //        printf("output_buffer encoded: %c\n", output_buffer[i]);
    }
    //    printf("encode: %\n", input_buffer[0]);
}

int myread(FILE *fp)
{
    int counter = 0;
    while (!feof(fp)) {
        uint8_t input_buffer[INPUT_BATCH_SIZE + ENDING_CHAR_SIZE];
        uint8_t output_buffer[OUTPUT_BATCH_SIZE + ENDING_CHAR_SIZE] = { 0 };
        unsigned long read_size = fread(input_buffer, sizeof * input_buffer, INPUT_BATCH_SIZE, fp);
        if (read_size) {
            // Process here
            process(input_buffer, read_size, output_buffer);
            //        printf("input buffer = %s\n", input_buffer);
            //        printf("output buffer = %s\n", output_buffer);
            counter += OUTPUT_BATCH_SIZE;
            if (counter > 76)
            {   printf("\n");
                counter = OUTPUT_BATCH_SIZE;
            }
            fwrite(output_buffer, sizeof * output_buffer, OUTPUT_BATCH_SIZE, stdout);
        }
        else {
            fprintf(stderr, "error with file/stdin read\n");
            return 1;
        }
    }
    fclose(fp);
    printf("\n");
    return 0;
}


int main(int argc, char * argv[])
{
    FILE *fp;
    
    // check if a file is passed as argument, if a file exists -> fopen, else -> stdin
    // print error if passing two many argument
    if (argc > 2)
    {
        fprintf(stderr, "You pass invalid number of arguments is provided\n");
        return 1;
    }
    if (argc == 1 || strcmp(argv[1], "-") == 0)
    {
        setvbuf(stdin, (char*)NULL, _IONBF, 0);
        fp = stdin;
        return myread(fp);
    }
    else
    {
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open %s\n", argv[1]);
            return 1;
        }
        return myread(fp);
    }
    return 0;
}

