#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

#define MAX_BUFFER_CHAR 60000
#define MAX_ARR 50000 //deduct
#define LARGEST_LINE 4096
#define TIME_LENGTH 8
//deduct
char source_station[100]; 
char des_station[100]; // error occur frequently if I set this in local coz we need char** to pass and edit the val frequently, 
//and we are not allowed to use malloc so it is hard not to use global 

void CopyNCharString(char* str_edited, const char* str_copied, int num) {
    /* USYD CODE CITATION ACKNOWLEDGEMENT
	 * I declare that the following lines of code have been copied from the
     * website titled: "Implement strncpy() function in c" and it is not my own work. 
     * 
     * Original URL
     * https://www.techiedelight.com/implement-strncpy-function-c/
     * Last access 19, March, 2021
     */
    while (*str_copied && num--) {
        *str_edited = *str_copied;
        str_edited++;
        str_copied++;
    }
    *str_edited = '\0';
    /* end of copied code */
}

int CompareString(char* str1, char* str2) {
    /* USYD CODE CITATION ACKNOWLEDGEMENT
	 * I declare that the following lines of code have been copied from the
     * website titled: "C Program to Compare Two Strings without using strcmp" and it is not my own work. 
     * 
     * Original URL
     * https://www.tutorialgateway.org/c-program-to-compare-two-strings/
     * Last access 19, March, 2021
     */
	int i = 0;
  	while (str1[i] == str2[i]) {
  		if (str1[i] == '\0' && str2[i] == '\0') {
	  		break;
        }
		i++;
	}
    return str1[i] - str2[i];
    /* end of copied code */
} 

void CopyString(char* str1, char* str2) {
    int i = 0;
    for (i = 0; str2[i] != '\0'; ++i) {
        str1[i] = str2[i];
    }
    str1[i] = '\0';
}
void print_the_next_earliest_time (char time[], char input_time[], int total_line, int cur_des_line) {
    while (cur_des_line < total_line) {       
        char time_temp[TIME_LENGTH+1];
        CopyNCharString(time_temp, time+(cur_des_line*TIME_LENGTH), TIME_LENGTH);
        if (CompareString(time_temp,input_time) > 0) {
            printf("The next train to %s from %s departs at %s\n", des_station, source_station, time_temp);
            break;
        }
        cur_des_line++;
    }
}
int idenfiy_the_pairs_exist(char source[], int cur_des_line) {
    int line = 0;
    int cur = 0;
    int prev = 0;
    for (int i = 0; source[i] != '\0'; i++) {
        if (source[i] == '\n') {
            prev = cur;
            cur = i;
            if (line == 0) {
                char temp[100];
                temp[i] = '\0';
                CopyNCharString(temp, source, i);
                if (CompareString(temp,source_station) == 0) {
                    return 1; //is the same pair with the first row
                }                
            }
            if (line == cur_des_line && line != 0) {
                int single_line_len = cur - prev - 1;
                char temp[100];
                temp[single_line_len] = '\0';
                CopyNCharString(temp, source+prev+1, single_line_len);
                if (CompareString(temp,source_station) == 0) {
                    return 1; //is the same pair
                }                
            }
        line++;
        }   
    }
    return 0; //not the same
}

int find_the_next_des(char des[], char input_des[], int cur_des_index, int* cur_des_line) {
    bool first = true;
    int num = 0;
    int prev = cur_des_index; //store the last index of des array 
    int cur = cur_des_index; //the current index of des array
    int next_des_line = *cur_des_line+1;
    for (int j = cur_des_index+1; des[j] != '\0'; j++) {       
        if (des[j] == '\n') {
            prev = cur; 
            cur = j;
            int diff = cur - prev - 1;
            char temp[100];
            temp[diff] = '\0';
            CopyNCharString(temp, des+prev+1, diff);
            if (CompareString(temp,input_des) == 0) {
                if (first) {
                    num = prev;
                    first = false;
                }
                if (prev-num > diff) {
                    *cur_des_line = next_des_line;  //find the next des line not in a series
                    return 1;
                }
                num = cur;  
            }
            next_des_line++;
        }
            
    }
    return -1;
}
int find_source_or_des(char source_or_des[], char input_buffer[], int kind, int* cur_des_index, int* cur_des_line) {
    bool first = true; //first line
    int prev = 0; //store the previous index of array 
    int cur; //the current index of array
   
    for (int j = 0; source_or_des[j] != '\0'; j++) {       
        if (source_or_des[j] == '\n') { 
            if (first) { //we need to distinguish the first line, we need ignore the '\n' in the first row, but the rest need
                first = false;
                cur = j;
                char temp[100];
                temp[j] = '\0';
                CopyNCharString(temp, source_or_des, j);
                if (CompareString(temp,input_buffer) == 0) {
                    if (kind == 0) {
                        CopyString(source_station, temp); //kind 0 refer source, 1 refer to destination
                        return 0;
                    }
                    *cur_des_index = j;
                    CopyString(des_station, temp); 
                    return 1;
                }
            }
            else { 
                prev = cur;
                cur = j;
                int single_line_len = cur - prev - 1;
                char temp[100];
                temp[single_line_len] = '\0';
                CopyNCharString(temp, source_or_des+prev+1, single_line_len);
                if (CompareString(temp,input_buffer) == 0) {
                    if (kind == 0) {
                        CopyString(source_station, temp);
                        return 0;
                    }
                    *cur_des_index = j;
                    CopyString(des_station, temp);   
                    return 1;
                }
            }
            if (kind == 1) {
                *cur_des_line = *cur_des_line + 1;
            }
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    char buffer[MAX_BUFFER_CHAR];
    char source[MAX_ARR];
    char des[MAX_ARR];
    char time[MAX_ARR];

    int flag = 0; // flag 0 indicates source, 1 for destination, 2 for time
    int total_line = 0;
    int source_idx = 0; // those three work for separate the array
    int des_idx = 0;
    int time_idx = 0;
    
    int cur_des_line = 0; //retrieve the current line with the same source and des
    int cur_des_index = 0; 
 
    if (argc != 4) {
        printf("Please provide <source> <destination> <time> as command line arguments\n");
        return 0;
    }   
    char* input_source = argv[1];
    char* input_des = argv[2];
    char* input_time = argv[3];

    while (fgets(buffer, MAX_BUFFER_CHAR, stdin) != NULL) {       
        if (total_line > LARGEST_LINE) {
            printf("Lines are longer than the maximum limit\n");
            return 2;
        }
        int time_len = 0;
        int colon_count = 0;
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == ':') {
                colon_count++;
                if (buffer[i+1] == ':') {
                    continue;
                }
                if (buffer[i-1] == ':') {     
                    flag++;
                    continue;
                }   
            }   
            if (flag == 0) {
                source[source_idx] = buffer[i];
                source_idx++;
            }
            else if (flag == 1) {
                des[des_idx] = buffer[i];
                des_idx++;
                
            }
            else if (flag == 2 && buffer[i] != '\n') {
                time[time_idx] = buffer[i];
                time_idx++; 
                time_len++;               
            }
            else if (buffer[i] == '\n' && (buffer[i-1] == '0' || buffer[i-1] == '1' || buffer[i-1] == '2' || buffer[i-1] == '3' || buffer[i-1] == '4' || buffer[i-1] == '5' || buffer[i-1] == '6' || buffer[i-1] == '7' || buffer[i-1] == '8' || buffer[i-1] == '9' )) {        
                flag = 0;
                source[source_idx] = '\n';
                des[des_idx] = '\n';
                source_idx++;
                des_idx++;
                if (time_len != TIME_LENGTH) {
                    printf("Invalid Time Format In File\n");
                    return 2;
                }
                if (colon_count != 6) {
                    printf("Invalid Colon Format In File\n");
                    return 2;
                }
                continue;
            }
                          
        }
        total_line++; 
    }
    source[source_idx] = '\0';
    des[des_idx] = '\0'; 
    time[time_idx] = '\0';
    
    if (find_source_or_des(source, input_source, 0, 0, 0) == 0) { 
        if (find_source_or_des(des, input_des, 1, &cur_des_index, &cur_des_line) == 1) {
            if (idenfiy_the_pairs_exist(source, cur_des_line) == 1) {
                print_the_next_earliest_time(time, input_time, total_line, cur_des_line);
                return 0;
            }
            else if (idenfiy_the_pairs_exist(source, cur_des_line) == 0) { //we need one more round
                if (find_the_next_des(des, input_des, cur_des_index, &cur_des_line) == 1) { //coz, we want to find some des not pair to source behind the existed pair
                    if (idenfiy_the_pairs_exist(source, cur_des_line) == 1) {
                        print_the_next_earliest_time(time, input_time, total_line, cur_des_line);
                        return 0;
                    }
                }   
            }           
        }         
    }
    printf("No suitable trains can be found\n");       
    return 0;
}
