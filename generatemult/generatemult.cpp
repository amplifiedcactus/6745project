#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <set>

//run with ./generatemult [number of bits per input] [output_file_name].blif
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [number of bits per input] [output_file_name].blif\n", argv[0]);
        return 1;
    }

    int num_bits = atoi(argv[1]);

    FILE *outfile = fopen(argv[2], "w");
    if (outfile == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Write BLIF header
    fprintf(outfile, ".model mult_%dx%d\n", num_bits, num_bits);
    // Write inputs line
    fprintf(outfile, ".inputs ");;
    for (int i = 0; i < num_bits; i++) {
        fprintf(outfile, "a%d ", i);
    }
    for (int i = 0; i < num_bits; i++) {
        fprintf(outfile, "b%d ", i);
    }
    // Write outputs line
    fprintf(outfile, "\n.outputs ");;
    for (int i = 0; i < 2 * num_bits; i++) {
        fprintf(outfile, "z%d ", i);
    }
    fprintf(outfile, "\n");
    // write first row of AND gates
    fprintf(outfile, ".names a0 b0 z0\n");
    fprintf(outfile, "11 1\n");
    for (int i = 1; i < num_bits; i++) {
        fprintf(outfile, ".names a%d b0 r0x%d\n", i, i - 1);
        fprintf(outfile, "11 1\n");
    }
    // write remaining rows of fulladder_and subcircuits
    // j is row number from top to bottom, i is column number from right to left
    
    // for last column of first row, a=0
    // for rest of last columns, a=cout from previous rows last column fulladder_and
    
    for (int j = 1; j < num_bits; j++) {
        for (int i = 0; i < num_bits; i++) {
            // for first column of each row, cin=0 s=zj
            if (i == 0)
                fprintf(outfile, ".subckt fulladder_and a=r%dx0 b0=a0 b1=b%d cin=0 s=z%d cout=r%dc0\n", j - 1, j, j, j);
                //last columns
                else if (i == num_bits - 1)
                // for last column of first row, a=0
                if (j == 1)
                    fprintf(outfile, ".subckt fulladder_and a=0 b0=a%d b1=b%d cin=r%dc%d s=r%dx%d cout=r%dc%d\n", i, j, j, i - 1, j, i - 1, j, i);
                //for last row,  last column, outputs are final outputs
                else if (j == num_bits - 1)
                    fprintf(outfile, ".subckt fulladder_and a=r%dc%d b0=a%d b1=b%d cin=r%dc%d s=z%d cout=z%d\n", j - 1, i, i, j, j, i - 1, 2 * num_bits - 2, 2 * num_bits - 1);
                // for rest of last columns, a=cout from previous rows last column fulladder_and
                else
                    fprintf(outfile, ".subckt fulladder_and a=r%dc%d b0=a%d b1=b%d cin=r%dc%d s=r%dx%d cout=r%dc%d\n", j - 1, i, i, j, j, i - 1, j, i - 1, j, i);
            else {
                //for last row, outputs are remaining circuit outputs
                if (j == num_bits - 1)
                    fprintf(outfile, ".subckt fulladder_and a=r%dx%d b0=a%d b1=b%d cin=r%dc%d s=z%d    cout=r%dc%d\n", j - 1, i, i, j, j, i - 1, num_bits + i - 1, j, i);
                else
                    fprintf(outfile, ".subckt fulladder_and a=r%dx%d b0=a%d b1=b%d cin=r%dc%d s=r%dx%d cout=r%dc%d\n", j - 1, i, i, j, j, i - 1, j, i - 1, j, i);

            }

                
        }
        
    }
    
    fprintf(outfile, "\n");
    fprintf(outfile, ".end\n");
    fprintf(outfile, ".model fulladder_and\n.inputs a b0 b1 cin\n.outputs s cout\n.names b0 b1 b\n11 1\n.names a b k\n10 1\n01 1\n.names k cin s\n10 1\n01 1\n.names a b p\n00 0\n.names a b g\n11 1\n.names p cin m\n11 1\n.names m g cout\n00 0\n");
    fprintf(outfile, ".end\n");



    return 0;
}

//     .model mult_3x3
// .inputs a0 a1 a2 b0 b1 b2
// .outputs z0 z1 z2 z3 z4 z5

// .names a0 b0 z0
// 11 1
// .names a1 b0 x0
// 11 1
// .names a2 b0 x1
// 11 1


// .subckt fulladder_and a=x0 b0=a0 b1=b1 cin=0 s=z1 cout=c0


