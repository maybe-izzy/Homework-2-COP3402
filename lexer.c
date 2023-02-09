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
#include <ctype.h>
#include "token.h"
#include "lexer_output.h"
#include "utilities.h"

// Lexer variables 
int done_flag; 
unsigned int line; 
unsigned int column; 
FILE *file_ptr; 
const char *file_name; 
char *buffer; 

// Returns token type if the input character is a string of some sort
int string_type(){
    if (strcmp(buffer, "var") == 0){
        return varsym; 
    }
    else {
        return identsym; 
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
    column = 1; 
    line = 1; 
    file_name = fname; 
    file_ptr = fp; 
    done_flag = 0; 
    buffer = (char*) malloc(sizeof(char) * (MAX_IDENT_LENGTH + 2)); 
    strcpy(buffer, ""); 
}

void lexer_close(){
    // Close the input file 
    fclose(file_ptr); 
}

bool lexer_done(){
    if (done_flag){
        return true; 
    }
    else {
        return false; 
    }
}

const char *lexer_filename(){
    return file_name; 
}

unsigned int lexer_line(){
    return line; 
}

unsigned int lexer_column(){
    return column;
}

// Push character to the buffer 
void buffer_cat(char c){
    int len = strlen(buffer); 
    
    if (len >= MAX_IDENT_LENGTH){
        bail_with_error("Max identifier length exceeded"); 
    }

    buffer[len] = c; 
    buffer[len + 1] = '\0'; 
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


// Eat characters until encounter something meaningful 
void eat_characters(){ 
    int stop_eating = 0; 
    char current_char; 

    while (!stop_eating){
        current_char = getc(file_ptr); 

        if (isspace(current_char)){
            if (current_char == '\n'){
                line++; 
                column = 0;  
            }
            column++; 
        }
        else if (current_char == '#'){ // Detect comments 
            while (peek_stream() != '\n'){
               current_char = getc(file_ptr); 
               if (current_char == EOF){
                    // ERROR; 
               }
               column++; 
            }
        }
        else {
            ungetc(current_char, file_ptr); 
            stop_eating = 1; 
        }
    }    
}

token lexer_next(){
    // Eat characters until meaningful input is encountered 
    eat_characters(); 

    char current_char = getc(file_ptr);
    
    
    if (current_char == EOF){
        done_flag = 1; 
        return assemble_token(eofsym); 
    }
    else if (isalpha(current_char)){ 
        buffer_cat(current_char); 
        char next_char = peek_stream(); 
        
        while (isalpha(next_char) || isdigit(next_char)){
            buffer_cat(getc(file_ptr));
            column++; 
            next_char = peek_stream(); 
        }
        return assemble_token(string_type()); 
    }
    else if (isdigit(current_char)){
        buffer_cat(current_char); 
        char next_char = peek_stream(); 
    
        while (isdigit(next_char)){
            buffer_cat(getc(file_ptr));
            column++; 
            next_char = peek_stream(); 
        }
        return assemble_token(numbersym); 
    }
    else if (ispunct(current_char)){
        buffer_cat(current_char); 
        if (current_char == ':'){
            char next_char = peek_stream(); 
            if (next_char == '='){
                column++; 
                buffer_cat(getc(file_ptr));
                return assemble_token(becomessym);  
            }
        }
        else if (current_char == ';'){
            return assemble_token(semisym); 
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