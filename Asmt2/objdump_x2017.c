#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int file_size = 0; 
char output[100][100]; //store the Opcode and gonna print in the end
int buffer[512] = {0}; //RAM, store the binary decoded by assembly
int line = 0; //the total line in Opcode 
int cur_bit = 0; // the core, the current bit in binary for decoding the Opcode
int cur_idx = 0; // for 2d array's cur cur_idx in each line
int func_line = 0; //decoded by num of instruction in func it will change by a new func appear
int stk[100] = {0}; //stk and idx works for storing the num of stack for us to compare and link the same
int stk_idx = 0;
//Opcode's binary
int MOV[3] = {0,0,0};
int RETURN[3] = {0,1,0};
int CAL[3] = {0,0,1};
int REF[3] = {0,1,1};
int ADDCODE[3] = {1,0,0};
int PRINT[3] = {1,0,1};
int NO[3] = {1,1,0};
int EQU[3] = {1,1,1};
int VAL[2] = {0,0};
int REGISTER[2] = {0,1};
int STACK[2] = {1,0};
int POINTER[2] = {1,1};
//convert integer to int array
void int_convert_to_arr (int* array, int number, int length) {
    for (int i = length-1; i >= 0; i--) {
        array[i] = number % 10;
        number /= 10;
    }
}
//convert int array to integer
int arr_convert_to_int(int* arr, int length) {
    int ret = 0;
    for (int i = 0; i < length; i++) {
        ret = 10 * ret + arr[i];
    }
    return ret;
}
// get the length of integer
int get_int_len (int value) {
    int res=1;
    while (value > 9) {  
        value /= 10; 
        res++;
    }
    return res;
}

int binary_to_decimal(int binnum) {
    /* USYD CODE CITATION ACKNOWLEDGEMENT
	 * I declare that the following lines of code have been copied from the
     * website titled: "Implement strncpy() function in c" and it is not my own work. 
     * 
     * Original URL
     * https://codescracker.com/c/program/c-program-convert-binary-to-decimal.htm
     * Last access 09, April, 2021
     */
    int decnum = 0, i = 0, rem;
    while (binnum != 0) {
        rem = binnum % 10;
        decnum = decnum + rem * pow(2,i);
        binnum /= 10;
        i++;
    }
    return decnum;
    /* end of copied code */
}

//Convert the byte of hex into binary e.g. 82 -> 10000010
void byte_to_array(int* buffer, unsigned char byte, int cur_hex) {
    int i = cur_hex*8;
    for (int j = 7; j >= 0; j--) { //we know 8 bit in one group of hex number
        int bit = 0;
        int temp = byte & (1<<j);
        if (temp > 0) {
            bit = 1;
        }
        buffer[i] = bit;
        i++;
    }
}
//act as memcmp but more effective
int array_compare(int a[], int b[], int n) {
    int i;
    for(i = 0; i < n; i++) {
        if (a[i] != b[i]) return -1;
    }
    return 0;
}
//work for finding ins already and record the length of func
void instruction() {
    int temp[5];
    for (int i = 0; i < 5; i++) {
        temp[i] = buffer[cur_bit-4+i];
    }
    int bin = arr_convert_to_int(temp, 5);
    func_line = binary_to_decimal(bin);
    cur_bit-=5;
}
// return 
void ret() {
    memcpy(output[line], "RET ", 4); //copy the RET  into output and the following memcpy do so
    line++;
    cur_bit-=3;
}
void print() {
    //the rest one will find cur_idx to store it and cur_idx will increase along its prev length
    memcpy(&output[line][cur_idx], "PRINT ", 6); 
    cur_idx+=6;
    cur_bit-=3;
}
//the cur idx in output go through 4 and cur bit less 3 to find the previous bit
void four_bit() {
    cur_idx+=4;
    cur_bit-=3;
}
void not() {
    memcpy(&output[line][cur_idx], "NOT ", 4);
    four_bit();
}
void call() {
    memcpy(&output[line][cur_idx], "CAL ", 4);
    four_bit();  
}
void add() {
    memcpy(&output[line][cur_idx], "ADD ", 4);
    four_bit();
}
void move() {
    memcpy(&output[line][cur_idx], "MOV ", 4);
    four_bit();
}
void refer() {
    memcpy(&output[line][cur_idx], "REF ", 4);
    four_bit();
}
void equal() {
    memcpy(&output[line][cur_idx], "EQU ", 4);
    four_bit();
}
void stack_or_pointer(int flag) { 
    if (flag == 0) { //0 is stack and 1 is pointer
        memcpy(&output[line][cur_idx], "STK ", 4);
    }
    else {
        memcpy(&output[line][cur_idx], "PTR ", 4);
    }
    cur_idx+=4;
    cur_bit-=2; //search move to prev 2 as 3 bits of stk/ptr has been detected
    //the below is for stk symbol eg A,B,C
    int temp[5];
    memcpy(temp, buffer+cur_bit-4, 5*4); //store the bit of symbol in temp *4 is we assume int store 4 bytes
    int bin = arr_convert_to_int(temp, 5); //convert it into int better to convert
    int res = binary_to_decimal(bin); //convert to decimal
    if (flag == 0) {
        stk[stk_idx] = res; //when stack we store all the num of bit to compare the same
        stk_idx++;
    }
    if (res == 0) {
        memcpy(&output[line][cur_idx], "A ", 2); //00000 will be A default
    }
    else {
        memcpy(&output[line][cur_idx], "B ", 2); 
    }
    cur_idx+=2;
    cur_bit-=5; 
}

void regist() {
    memcpy(&output[line][cur_idx], "REG ", 4);
    cur_idx+=4;
    cur_bit-=2; //search move to prev 2 in ram
    // reg will have fixed 3 bits for val
    int temp[3];
    memcpy(temp, buffer+cur_bit-2, 3*4);
    int bin = arr_convert_to_int(temp, 3);
    int res = binary_to_decimal(bin); //convert to decimal
    char num[2];
    memcpy(num+1, " ", 1);
    num[0] = res + '0'; //convert to char for better store in output
    memcpy(&output[line][cur_idx], num, 2);
    cur_idx+=2;
    cur_bit-=3;
}

void value() {
    memcpy(&output[line][cur_idx], "VAL ", 4);
    cur_idx+=4;
    cur_bit-=2; //search move to prev 2 in ram
    int temp[8]; //there is 8 bits per one VAL
    memcpy(temp, buffer+cur_bit-7, 8*4);  
    int bin = arr_convert_to_int(temp, 8); 
    int res = binary_to_decimal(bin); //convert to decimal
    int length = get_int_len(res); // get the length of int because we cannot store more than 9's int into one char
    char num[5];
    int arr[5]; // we assume it wouldn't be 5 decimal in total
    int_convert_to_arr(arr, res, length); //convert int to array
    for (int i = 0; i < length; i++) {
        num[i] = arr[i] + '0'; //convert int to char and store in corresponding location in array
    }
    memcpy(num+length, " ", 1);
    memcpy(&output[line][cur_idx], num, length+1); //then we will store the val in output  
    cur_idx+=length+1;
    cur_bit-=8;
}

void function_label() {
    memcpy(&output[line][cur_idx], "FUNC LABEL ", 11);
    cur_idx+=11;
    int temp[3];
    memcpy(temp, buffer+cur_bit-2, 3*4);
    int bin = arr_convert_to_int(temp, 3);
    int res = binary_to_decimal(bin);
    char num[2];
    memcpy(num+1, " ", 1);
    num[0] = res + '0';
    memcpy(&output[line][cur_idx], num, 2);
    cur_idx = 0; // we need to reset the index in output because it has already done in one line
    cur_bit-=3;
}

void identify_stk_ptr_reg_or_val() { //that's ez for us to group and see
    if (array_compare(buffer+cur_bit-1, STACK, 2) == 0) {
        stack_or_pointer(0);  //0 represents stack
    }  
    else if (array_compare(buffer+cur_bit-1, POINTER, 2) == 0) {
        stack_or_pointer(1); // pointer
    }      
    else if (array_compare(buffer+cur_bit-1, REGISTER, 2) == 0) {
        regist(); 
    }
    else if (array_compare(buffer+cur_bit-1, VAL, 2) == 0) {
        value();    
    }
}

void new_line() { //reset the cur idx in line and move to a new line in output
    cur_idx = 0;
    line++;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {  //it must be one file
        printf("Argument not enough!");
        return 2;
    }
    FILE* fp = fopen(argv[1], "rb");  
    if (!fp) {
        fprintf(stderr, "error\n"); //aganst null or wrong format
        return 2;
    }  
    fseek(fp, 0, SEEK_END); // seek to end of file
    file_size = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET);
    if (file_size > 256) {
        printf("File size over the RAM load!");
        return 2;
    }
    unsigned char ram[256];
    fread(ram, 1, 256, fp);
    
    
    for (int i = 0; i < file_size; i++) {
        byte_to_array(buffer, ram[i], i); //get the bit of hex set from RAM
    }
    cur_bit = (file_size*8)-1; //cur_bit is init as the last index in ram coz we want read from the end to the prev
    int j = 0; //work for each new function label
    while (cur_bit > 0) {
        if (j > 100) {
            printf("Endless loop may occur! Something wrong!");
            return 2;
        }
        if (cur_bit <= 9) {
            function_label(); //ignore some padding and work safety
            break;
        }
        if (j == 0) {
            instruction();
        }
        if (j == func_line && j != 0) { //for a new function label
            function_label();
            instruction();
            j = 0;
            line++;
        }
        if (array_compare((buffer+cur_bit-2), RETURN, 3) == 0) {
            ret();
        }
        else if (array_compare(buffer+cur_bit-2, PRINT, 3) == 0) {
            print();
            identify_stk_ptr_reg_or_val(); //write before
            new_line();       
        }
        else if (array_compare(buffer+cur_bit-2, MOV, 3) == 0) {
            move(); 
            identify_stk_ptr_reg_or_val(); //choose two of four
            identify_stk_ptr_reg_or_val();
            new_line();
        }
        else if (array_compare(buffer+cur_bit-2, REF, 3) == 0) {
            refer();
            identify_stk_ptr_reg_or_val(); 
            identify_stk_ptr_reg_or_val();    
            new_line();
        }  
        else if (array_compare(buffer+cur_bit-2, ADDCODE, 3) == 0) { 
            add();
            regist(); 
            regist(); 
            new_line();
        }
        else if (array_compare(buffer+cur_bit-2, NO, 3) == 0) {
            not(); 
            regist();
            new_line();
        }
        else if (array_compare(buffer+cur_bit-2, CAL, 3) == 0) {
            call(); 
            value();
            new_line();
        }
        else if (array_compare(buffer+cur_bit-2, EQU, 3) == 0) {
            equal(); 
            regist();
            new_line();
        }
        j++; 
    }
    //check the correct format of opcode if the end is not return and this file is invalid
    if (output[0][0] != 'R' || output[0][1] != 'E' || output[0][2] != 'T') {
        printf("This File is Invalid Assembly Code!");
        return 2;
    }
    //the following is to print out the output
    bool first_stack = true;
    bool first_ptr = true;
    int rec_s = 0;
    bool all_same = false;
    for (int i = stk_idx-1; i > 0; i--) {
        if (stk[i] != stk[i-1]) {
            all_same = false;
            break;
        }
        all_same = true; //if all the stack symbol is the same then it is A
    } 
    for (int i = line; i >= 0; i--) {
        if (output[i][0] != 'F')
            printf("    "); //not func one will have indent
        if (output[i][0] == 'F') {
            first_stack = true; //new func will refresh the first shown stack or ptr symbol
            first_ptr = true;
        }
        for (int j = 0; output[i][j+1] != 0; j++) {
            if (all_same && output[i][j] == 'S') {
                output[i][j+4] = 'A'; //all is the same then print A
            }
            if (first_stack && (output[i][j] == 'S')) {
                output[i][j+4] = 'A'; //if the first time call stack it must be A
                rec_s = i;
                first_stack = false;
            }
            if (output[i][0] != 'P' && first_ptr && output[i][j] == 'P' && output[i][j+1] == 'T') {
                output[i][j+4] = 'A'; //the first time call ptr it must be A
                first_ptr = false;
            }
            if (i == rec_s-1 && output[i][0] == 'P' && output[i][j] == 'S') {
                output[i][j+4] = 'A'; //after use it we print it will not change
            }
            printf("%c", output[i][j]);
        }  
        if (i != 0)
            printf("\n");
    }
    fclose(fp);
    return 0;
}