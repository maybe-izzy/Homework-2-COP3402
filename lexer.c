/*=============================================================================
| Assignment: HW 2 - Lexer 
| Authors: Isabelle Montgomery and Christopher Colon Marquez
| Language: C
| Class: COP3402 (0001) - Systems Software - Spring 2023
| Instructor: Gary T. Leavens
| Due Date: 02/14/2023
|
| This program implements a lexical analyzer in C. 
+=============================================================================*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "token.h"
#include "lexer_output.h"
#include "utilities.h"

// Lexer variables 
int done_flag; 

unsigned int current_state; 
unsigned int line; 
unsigned int column; 

FILE *file_ptr; 
const char *file_name; 
char buffer[MAX_IDENT_LENGTH + 1]; 
char legal_symbols[] = {'>', '<', '(', ')', '*', '+', '-', '/', ':', ';', ',', '.', '='}; 

// Returns token type if the input character is a string of some sort
int string_type(){
    if (strcmp(buffer, "var") == 0){
        return varsym; 
    }
    else if (strcmp(buffer, "do") == 0){
        return dosym; 
    }
    else if (strcmp(buffer, "skip") == 0){
        return skipsym; 
    }
    else if (strcmp(buffer, "const") == 0){
        return constsym; 
    }
    else if (strcmp(buffer, "begin") == 0){
        return beginsym; 
    }
    else if (strcmp(buffer, "call") == 0){
        return callsym; 
    }
    else if (strcmp(buffer, "procedure") == 0){
        return procsym; 
    }
    else if (strcmp(buffer, "end") == 0){
        return endsym; 
    }
    else if (strcmp(buffer, "write") == 0){
        return writesym; 
    }
    else if (strcmp(buffer, "read") == 0){
        return readsym; 
    }
    else if (strcmp(buffer, "if") == 0){
        return ifsym; 
    }
    else if (strcmp(buffer, "then") == 0){
        return thensym; 
    }
    else if (strcmp(buffer, "else") == 0){
        return elsesym; 
    }
    else if (strcmp(buffer, "odd") == 0){
        return oddsym; 
    }
    else {
        return identsym; 
    }
}

void is_legal(char c){
    if (isalpha(c) || isdigit(c) || isspace(c) || (c == EOF)){
        return; 
    }
    else {
        for (int i = 0; i < 13; i++){
            if (c == legal_symbols[i]){
                return; 
            }
        }
        char error[50]; 
        sprintf(error, "Illegal character '%c' (%.3o)", c, c); 
        lexical_error(file_name, lexer_line(), lexer_column(), error);
    }
}

void buffer_reset(){
    strcpy(buffer, ""); 
}

void lexer_open(const char *fname){
    FILE *fp = fopen(fname, "r"); 
    
    // Make sure that the program was passed a valid filename 
    if (fp == NULL){
        bail_with_error("Invalid file name");
    }

    // Initialize the lexer
    column = 0; 
    line = 1; 
    file_name = fname; 
    file_ptr = fp; 
    done_flag = 0; 
    buffer_reset(); 
}

void lexer_close(){
    // Close the input file 
    fclose(file_ptr); 
}

bool lexer_done(){
    if (done_flag){
        return true; 
    }
    return false; 
}

const char *lexer_filename(){
    return file_name; 
}

unsigned int lexer_line(){
    return line; 
}

unsigned int lexer_column(){
    int len = strlen(buffer); 
    if (len > 0){
        return (column -  strlen(buffer) + 1);
    }
    else {
        return column; 
    }
}

// Push character to the buffer 
void buffer_cat(char c){
    int len = strlen(buffer); 
    strncat(buffer, &c, 1); 
}

token assemble_token(token_type type){
    token new_token; 

    new_token.typ = type; 
    new_token.column = lexer_column(); 
    new_token.line = lexer_line(); 

    if (type == numbersym){
        new_token.value = atoi(buffer); 
    }
    if (type == eofsym){
        new_token.text = NULL; 
    }
    else {
        new_token.text = (char*) malloc(sizeof(buffer + 1) * sizeof(char)); 
        strcpy(new_token.text, buffer); 
    }
    buffer_reset(); 
    return new_token; 
}

char peek_stream(){
    char c = getc(file_ptr); 
    ungetc(c, file_ptr); 

    return c;
}


char get_character(){
    column++;
    char c = getc(file_ptr);  
    buffer_cat(c);
    return c;
}

void put_back(){
    ungetc(buffer[strlen(buffer) - 1], file_ptr); 
    buffer[strlen(buffer) - 1] = '\0'; 
    column--; 
}


// Eat characters until encounter something meaningful 
void eat_characters(){ 
    int stop_eating = 0; 
    char current_char; 

    while (!stop_eating){
        current_char =  get_character(); 
        //printf("Col: %d", column); 
        
        if (isspace(current_char)){
            if (current_char == '\n'){
                line++; 
                column = 0;  
            }
           // printf("Col: %d", column); 
        }
        else if (current_char == '#'){ // Detect comments 
            while (current_char != '\n'){
                //printf("Col: %d", column); 
                current_char = get_character(); 
                if (current_char == EOF){
                    lexical_error(file_name, lexer_line(), column, "File ended while reading comment!");
                }
            }
            //printf("Col: %d", column); 
            put_back(); 
        }
        else {
            printf("Col: %d", column); 
            stop_eating = 1; 
        }
    }   
    put_back();
    buffer_reset();  
    printf("Col: %d", column); 
}

token lexer_next(){
    eat_characters(); 
    char error[50]; 

    char current_char = get_character(); 
    is_legal(current_char); 
    char next_char; 

    if (current_char == EOF){
        done_flag = 1; 
        return assemble_token(eofsym); 
    }
    else if (isalpha(current_char)){ 
        next_char = get_character(); 

        while (isalpha(next_char) || isdigit(next_char)){
            next_char = get_character(); 
            
            if (strlen(buffer) >= MAX_IDENT_LENGTH){
                lexical_error(file_name, lexer_line(), lexer_column(), "Identifier starting %s is too long!", buffer);
            }
        }
        put_back(); 
        return assemble_token(string_type()); 
    }
    else if (isdigit(current_char)){
        next_char = get_character(); 
    
        while (isdigit(next_char)){
            if ((atoi(buffer) > SHRT_MAX) || (atoi(buffer) < SHRT_MIN)){
                sprintf(error, "The value of %d is too large for a short!", atoi(buffer)); 
                lexical_error(file_name, lexer_line(), lexer_column(), error);
            }
            next_char = get_character(); 
        }
        put_back(); 
        return assemble_token(numbersym); 
    }
    else if (ispunct(current_char)){
        if (current_char == ':'){
            next_char = get_character(); 
            if (next_char == '='){
                return assemble_token(becomessym);  
            }
            else { 
                sprintf(error, "Expecting '=' after a colon, not '%c'", next_char); 
                // Since error is specific to character at current column, use the non-adjusted column value
                lexical_error(file_name, lexer_line(), column, error); 
            }
        }
        else if (current_char == ';'){
            return assemble_token(semisym); 
        }
        else if (current_char == '.'){
            return assemble_token(periodsym); 
        }
        else if (current_char == ','){
            return assemble_token(commasym); 
        }
        else if (current_char == '='){
            return assemble_token(eqsym); 
        }
        else if (current_char == '('){
            return assemble_token(lparensym); 
        }
        else if (current_char == ')'){
            return assemble_token(rparensym); 
        }
        else if (current_char == '<'){
            char next_char = get_character(); 
            if (next_char == '>'){
                return assemble_token(neqsym); 
            }
            else if (next_char == '='){
                return assemble_token(leqsym); 
            }
            else {
                put_back(); 
                return assemble_token(lessym); 
            }
        }
        else if (current_char == '>'){
            char next_char = get_character(); 
            if (next_char == '='){
                return assemble_token(geqsym); 
            }
            else {
                put_back(); 
                return assemble_token(gtrsym); 
            }
        }
        else if (current_char == '+'){
            return assemble_token(plussym); 
        }
        else if (current_char == '-'){
            return assemble_token(minussym); 
        }
        else if (current_char == '/'){
            return assemble_token(divsym); 
        }
        else if (current_char == '*'){
            return assemble_token(multsym); 
        }
    }
    else {
        // ERROR
    }
}

int main(int argc, char *argv[]){
    --argc;

    // Call lexer if received valid command line args 
    if (argc == 1) {  
	    lexer_open(argv[1]);
        lexer_output(); 
        lexer_close(); 
    } 
    // Throw error if received invalid command line args 
    else {
	    bail_with_error("Too many arguments in lexer invocation"); 
    }
    return EXIT_SUCCESS;
}