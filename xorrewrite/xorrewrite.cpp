


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <set>
#include <vector>


#include <algorithm> // Required for std::remove_if



//If output is inverted, add polynomial to invert output variable - 1
void addpolyout(std::string &polynomials, int &polycount, int &maxvar, int var) {
    std::string polynomial = "poly f" + std::to_string(polycount) + " = x_" + std::to_string(var) + " - (1-x_" + std::to_string(var - 1) + ");\n";
    //poly f1 = z_1 - (1-e_20);
    polynomials += polynomial;
    polycount++;
    maxvar = std::max(maxvar, var);
}
//add to polynomials for each and gate
void addpolyand(std::string &polynomials, int &polycount, int &maxvar, int buffer0, int buffer1, int buffer2) {
    for (int var : {buffer0, buffer1, buffer2}) {
        if (var & 1) {
            addpolyout(polynomials, polycount, maxvar, var);
        }
        maxvar = std::max(maxvar, var);
    }
    std::string polynomial = "poly f" + std::to_string(polycount) + " = x_" + std::to_string(buffer0) + " - (x_" + std::to_string(buffer1) + "*x_" + std::to_string(buffer2) + ");\n";
    //poly f1 = z_1 - (1-e_20);
    polynomials += polynomial;
    polycount++;
    //maxvar = std::max(maxvar, var);
}
struct andgate {
    int num;
    int a;
    int b;
    int out;
    bool an;
    bool bn;
};
struct xorgate {
    int a;
    int b;
    int out;
};
void remove_gate(std::vector<andgate>& gates, int target_num) {
    gates.erase(
        std::remove_if(gates.begin(), gates.end(), 
            [target_num](const andgate& gate) {
                return gate.num == target_num; 
            }),
        gates.end()
    );
}


void addpolyand(std::string &polynomials, int &polycount, int &maxvar, std::vector<andgate>& gates) {
    if (gates.an)
    
    
    for (int var : {buffer0, buffer1, buffer2}) {
        if (var & 1) {
            addpolyout(polynomials, polycount, maxvar, var);
        }
        maxvar = std::max(maxvar, var);
    }
    std::string polynomial = "poly f" + std::to_string(polycount) + " = x_" + std::to_string(buffer0) + " - (x_" + std::to_string(buffer1) + "*x_" + std::to_string(buffer2) + ");\n";
    //poly f1 = z_1 - (1-e_20);
    polynomials += polynomial;
    polycount++;
    //maxvar = std::max(maxvar, var);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file.aag> <output_file.sing>\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "r");
    if (infile == NULL) {
        perror("Error opening file");
        return 1;
    }

    FILE *outfile = fopen(argv[2], "w");
    if (outfile == NULL) {
        perror("Error opening output file");
        // Close the input file before exiting to avoid resource leaks
        fclose(infile); 
        return 1;
    }


    int buffer;
    char newline_char;
    static unsigned M, I, L, O, A;

    if (fscanf (infile, "aag %u %u %u %u %u\n", &M, &I, &L, &O, &A) != 5)
        perror("invalid header");
    
    fprintf (stderr, "aag %u %u %u %u %u\n", M, I, L, O, A);

    //I is number of inputs
    //O is number of outputs

    //Make variables for inputs, outputs, polynomials, ideals, ring term order
    std::string inputs = "";
    std::string outputs = "";
    std::string polynomials = "";
    std::string fspec = "";
    std::string J = "";
    std::string J0 = "";
    std::string order = "";
    int polycount = 0;
    int maxvar = 0;

    //Create inputs string
    for (unsigned i = 0; i < I-1; i++) {
        if (fscanf(infile, "%d", &buffer) == 1)
            inputs += "x_" + std::to_string(buffer) + ", ";
        fgetc(infile); //consume newline
    }
    if (fscanf(infile, "%d", &buffer) == 1)
        inputs += "x_" + std::to_string(buffer) + "";
    fgetc(infile); //consume newline

    printf("inputs: %s\n", inputs.c_str());
    
    //Create outputs string and polynomials for inverted outputs
    for (unsigned i = 0; i < O-1; i++) {
        if (fscanf(infile, "%d", &buffer) == 1)
            if (buffer & 1)
                addpolyout(polynomials, polycount, maxvar, buffer);
            outputs += "x_" + std::to_string(buffer) + ", ";
        fgetc(infile); //consume newline
    }
    if (fscanf(infile, "%d", &buffer) == 1)
        if (buffer & 1)
            addpolyout(polynomials, polycount, maxvar, buffer);
        outputs += "x_" + std::to_string(buffer) + "";
    fgetc(infile); //consume newline

    printf("outputs: %s\n", outputs.c_str());
    printf("polynomials: \n%s\n", polynomials.c_str());

    std::vector<andgate> andgates;
    std::vector<xorgate> xorgates;
    int num = 0;
    //Create polynomials for and gates
    while (1) {
        // Step 1: Try to read text up to the newline
        // %1023[^\n] -> Read up to 1023 chars that are NOT a newline
        int buffer2[3];
        if (fscanf(infile, "%d %d %d", &buffer2[0], &buffer2[1], &buffer2[2]) == 3) {
            andgates.push_back({num, buffer2[1], buffer2[2], buffer2[0], static_cast<bool>(buffer2[1] & 1), static_cast<bool>(buffer2[2] & 1)});
            num++;
            //addpolyand(polynomials, polycount, maxvar, buffer2[0], buffer2[1], buffer2[2]);
            //printf(" %d %d %d \n", buffer2[0], buffer2[1], buffer2[2]);
        }

        // Step 2: Read the actual newline character (or whatever stopped the previous scan)
        // We use %c to pick up the '\n' that Step 1 left behind.
        if (fscanf(infile, "%c", &newline_char) != 1) {
            break; // If we can't read a character, we are at EOF
        }
        
        // Print the newline to maintain exact formatting
        printf("%c", newline_char);
    }
    //print andgates
    for (auto& gate : andgates) {
        //printf("and gate: out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate.out, gate.a, gate.an, gate.b, gate.bn);
        if (gate.an)
            gate.a -= 1;
        if (gate.bn)
            gate.b -= 1;
        //printf("\n");
        //printf("and gate: out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate.out, gate.a, gate.an, gate.b, gate.bn);
    }
    //search andgates and check for xor gates. If found, remove and gate and add xor gate
    //if there are two and gates with same a and b inputs, one with an false bn true, other with an true bn false, then another and gate with negated outputs of those and gates as inputs, xor
    //for each variable, loop through andgates
    for (auto& gate : andgates) {
        for (auto& gate2 : andgates) {
            // printf("and gate: num=%d out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate.num, gate.out, gate.a, gate.an, gate.b, gate.bn);
            // printf("and gate2: num=%d out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate2.num, gate2.out, gate2.a, gate2.an, gate2.b, gate2.bn);
            // printf("\n");
            if (gate.num == gate2.num) {
                continue;
            }
            else {
                if ((gate.a == gate2.a && gate.b == gate2.b && gate.an != gate2.an && gate.bn != gate2.bn) || (gate.a == gate2.b && gate.b == gate2.a && gate.an == gate2.an && gate.bn == gate2.bn)) {
                    //search for and gate with negated outputs of gate and gate2 as inputs
                    printf("xor canidates:\n");
                    printf("and gate: num=%d out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate.num, gate.out, gate.a, gate.an, gate.b, gate.bn);
                    printf("and gate2: num=%d out=%d a=%d (an=%d) b=%d (bn=%d)\n", gate2.num, gate2.out, gate2.a, gate2.an, gate2.b, gate2.bn);
                    printf("\n");
            
                    for (auto& gate3 : andgates) {
                        if (gate3.a == gate.out && gate3.b == gate2.out && gate3.an && gate3.bn) {
                            //found xor gate
                            printf("found xor gate: out=%d a=%d b=%d\n", gate3.out, gate.a, gate.b);
                            //add xor gate
                            //xor gate output is gate3.out, inputs are gate.a and gate.b
                            xorgates.push_back({gate.a, gate.b, gate3.out});

                            //remove gate, gate2, gate3 from andgates
                            remove_gate(andgates, gate.num);
                            remove_gate(andgates, gate2.num);
                            remove_gate(andgates, gate3.num);
                            break;
                            
                            
                        }
                    }
                }
            }
        }
    }
    for (auto& xorgate : xorgates) {
        printf("xor gate: out=%d a=%d b=%d\n", xorgate.out, xorgate.a, xorgate.b);
    }

    //create polynomials for and gates:
    for (auto& gate : andgates) {
        addpolyand(polynomials, polycount, maxvar, gate.out, gate.a, gate.b);
    }


    return 0;
}
    