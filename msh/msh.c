// The MIT License (MIT)
// 
// Copyright (c) 2024 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 32     

int main(int argc, char **argv)
{
char error_message[30] = "An error has occurred\n";
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  FILE* fileptr = stdin;

  
  int batch = 0, n_commands=0;
  if (argc == 2) {
    batch=1;
      fileptr = fopen(argv[1], "r"); 
      if(fileptr == NULL){
     write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
      }
  } else if(argc > 2) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

 
  while( 1 )
  {
    // Print out the msh prompt
    if(!batch)
      printf ("msh> ");

    // Read the command from the commandi line.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something.
    
    while( !fgets (command_string, MAX_COMMAND_SIZE, fileptr) ) {
      if(batch) {
        if(feof(fileptr)) {
          if(n_commands == 0) exit(1);
          exit(0);
        }
      }
    }

    n_commands++;
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_pointer;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    
    char *head_ptr = working_string;
    
    // Tokenize the input with whitespace used as the delimiter
    while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      } else {
        token_count++;
      }
    }
   
 
    

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality
  if (token[0] == NULL){
    continue;
  }
  if(strcmp(token[0],"exit") == 0){
    if(token[1] != NULL) {
    write(STDERR_FILENO, error_message, strlen(error_message));
  } else {
    exit(0);
  }
} else if (strcmp(token[0], "cd") == 0){
  if(token[1] == NULL) {
    write(STDERR_FILENO, error_message, strlen(error_message));
  } else {
   int r = chdir(token[1]);
   if(r < 0) {
    write(STDERR_FILENO, error_message, strlen(error_message));
   }
  }
}else{
   pid_t pid = fork();

  if( pid == -1 )
  {
    // When fork() returns -1, an error happened.
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit( EXIT_FAILURE );
  }
  else if ( pid == 0 )
  {
    // redirect?
    int i;
     for(i=1; i<token_count; i++ )
      {
        
        if(token[i] == NULL) break;
         if( strcmp( token[i], ">" ) == 0 )
         {
          if(token[i+1] != 0 && token[i + 2] != 0){
             write(STDERR_FILENO, error_message, strlen(error_message));
              exit(0);

          }
            int fd = open( token[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
            if( fd < 0 )
            {
              write(STDERR_FILENO, error_message, strlen(error_message));
              exit(0);
            }
            dup2( fd, 1 );
            close( fd );
            
            // Trim off the > output part of the command
            token[i] = NULL;
            break;
         }
      }
    char path[4][40] = { "/bin/", "/usr/bin/", "/usr/local/bin/", "./"};

    for(int i=0; i<4; i++) {
      char character[126];
      sprintf(character, "%s%s", path[i], token[0]);
      if(access(character, X_OK) == 0){
        execv(character, token);
        break;
      }
    }
      write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(NULL);
    exit( EXIT_SUCCESS );
  }
  else 
  {
    // When fork() returns a positive number, we are in the parent
    // process and the return value is the PID of the newly created
    // child process.
    int status;

    // Force the parent process to wait until the child process 
    // exits
    waitpid(pid, &status, 0 );
    fflush( NULL );
  }
}

    free( head_ptr );

  }
  return 0;
  // e2520ca2-76f3-90d6-0242ac1210022
}

