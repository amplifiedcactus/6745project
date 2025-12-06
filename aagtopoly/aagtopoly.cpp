#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <set>

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


    //Create polynomials for and gates
    while (1) {
        // Step 1: Try to read text up to the newline
        // %1023[^\n] -> Read up to 1023 chars that are NOT a newline
        int buffer2[3];
        if (fscanf(infile, "%d %d %d", &buffer2[0], &buffer2[1], &buffer2[2]) == 3) {
            addpolyand(polynomials, polycount, maxvar, buffer2[0], buffer2[1], buffer2[2]);
            printf(" %d %d %d \n", buffer2[0], buffer2[1], buffer2[2]);
        }

        // Step 2: Read the actual newline character (or whatever stopped the previous scan)
        // We use %c to pick up the '\n' that Step 1 left behind.
        if (fscanf(infile, "%c", &newline_char) != 1) {
            break; // If we can't read a character, we are at EOF
        }
        
        // Print the newline to maintain exact formatting
        printf("%c", newline_char);
    }
    printf("polynomials: \n%s\n", polynomials.c_str());
    printf("maxvar: %d\n", maxvar);
    


    //Create string for ring term order
    int i = maxvar;
    // The string containing items to remove
    std::string s = inputs + ", " + outputs;
    
    // 1. Parse 's' into a set for fast lookup
    // Using a set makes checking "does this exist?" very fast
    std::set<std::string> to_remove;
    std::stringstream ss(s);
    std::string segment;
    
    while(std::getline(ss, segment, ',')) {
        // Trim whitespace (so " x2" becomes "x2")
        size_t first = segment.find_first_not_of(' ');
        size_t last = segment.find_last_not_of(' ');
        
        if (first != std::string::npos && last != std::string::npos) {
            to_remove.insert(segment.substr(first, (last - first + 1)));
        }
    }

    for (int k = maxvar; k >= 0; k--) {
        // Construct the current token (e.g., "x1")
        std::string current_token = "x_" + std::to_string(k);

        // 2. Only add if it is NOT in the removal set
        if (to_remove.find(current_token) == to_remove.end()) {
            
            // Add comma if this isn't the first item added to the order
            if (!order.empty()) {
                order += ", ";
            }
            
            order += current_token;
        }
    }
    order = "ring R = 0, (" + outputs + ", " + order + ", " + inputs;
    order += "), lp;\n";
    printf("order: %s\n", order.c_str());


    //Create fspec string
    //poly f_spec = (x10 + 2*x21 + 4*x31 + 8*x32) - (x2 + 2*x4)*(x6 + 2*x8); //spec
    fspec += "poly f_spec = (";
    std::stringstream ss_outputs(outputs);
    std::string output_var;
    int coeff = 1;
    bool first_output = true;
    while (std::getline(ss_outputs, output_var, ',')) {
        // Trim whitespace
        size_t first = output_var.find_first_not_of(' ');
        size_t last = output_var.find_last_not_of(' ');
        if (first != std::string::npos && last != std::string::npos) {
            std::string trimmed_var = output_var.substr(first, (last - first + 1));
            if (!first_output) {
                fspec += " + ";
            }
            fspec += std::to_string(coeff) + "*" + trimmed_var;;
            coeff *= 2;
            first_output = false;
        }
    }
    fspec += ") - (";
    coeff = 1;
    std::stringstream ss_inputs2(inputs);
    std::string input_var2;
    bool first_input2 = true;
    for (int k = 0; k < I/2; k++) {
        std::getline(ss_inputs2, input_var2, ',');
        // Trim whitespace
        size_t first = input_var2.find_first_not_of(' ');
        size_t last = input_var2.find_last_not_of(' ');
        if (first != std::string::npos && last != std::string::npos) {
            std::string trimmed_var = input_var2.substr(first, (last - first + 1));
            if (!first_input2) {
                fspec += " + ";
            }
            fspec += std::to_string(coeff) + "*" + trimmed_var;;
            coeff *= 2;
            first_input2 = false;
        }
    }
    fspec += ")*(";
    first_input2 = true;
    coeff = 1;
    for (int k = I/2; k < I; k++) {
        std::getline(ss_inputs2, input_var2, ',');
        printf("input_var2: %s\n", input_var2.c_str());
        
        // Trim whitespace
        size_t first = input_var2.find_first_not_of(' ');
        size_t last = input_var2.find_last_not_of(' ');
        if (first != std::string::npos && last != std::string::npos) {
            std::string trimmed_var = input_var2.substr(first, (last - first + 1));
            if (!first_input2) {
                fspec += " + ";
            }
            fspec += std::to_string(coeff) + "*" + trimmed_var;;
            coeff *= 2;
            first_input2 = false;
        }
    }

    fspec += ");\n";


    //create J string
    J += "ideal J = ";
    for (int k = 0; k < polycount; k++) {
        J += "f" + std::to_string(k);
        if (k != polycount - 1) {
            J += ", ";
        }
    }
    J += ";\n";

    //create J0 string
    //loop through inputs, create vanishing polynomials [input_var^2 - input_var]
    J0 += "ideal J0 = ";
    std::stringstream ss_inputs(inputs);
    std::string input_var;
    bool first_input = true;
    while (std::getline(ss_inputs, input_var, ',')) {
        // Trim whitespace
        size_t first = input_var.find_first_not_of(' ');
        size_t last = input_var.find_last_not_of(' ');
        if (first != std::string::npos && last != std::string::npos) {
            std::string trimmed_var = input_var.substr(first, (last - first + 1));
            if (!first_input) {
                J0 += ", ";
            }
            J0 += trimmed_var + "^2 - " + trimmed_var;
            first_input = false;
        }
    }
    J0 += ";\n";
    printf("fspec: %s\n", fspec.c_str());
    printf("J: %s\n", J.c_str());
    printf("J0: %s\n", J0.c_str());
    fclose(infile);

    std::string lastlines = "printf(\"Verification: f_spec mod (J+J0) should be 0\"); \n reduce(f_spec, J+J0);\n";

    //create mult.sing file
    // FILE *outFile = fopen("multout3.sing", "w");
    // if (outFile == NULL) {
    //     perror("Error creating output file");
    //     return 1;
    // }
    fprintf(outfile, "%s\n%s\n%s\n%s\n%s\n%s", order.c_str(), fspec.c_str(), polynomials.c_str(), J.c_str(), J0.c_str(), lastlines.c_str());

    fclose(outfile);

    return 0;
}