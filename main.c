/* You are not allowed to use <stdio.h> */
#include <stdlib.h>
#include "io.h"

/**
 * @name  main
 * @brief This function is the entry point to your program
 * @return 0 for success, anything else for failure
 *
 *
 * Then it has a place for you to implementation the command
 * interpreter as  specified in the handout.
 */
int main()
{
  /*-----------------------------------------------------------------
   *TODO:  You need to implement the command line driver here as
   *       specified in the assignment handout.
   *
   * The following pseudo code describes what you need to do
   *
   *  Declare the counter and the collection structure variables
   *
   *
   *  In a loop
   *    1) Read a command from standard in using read_char function
   *    2) If the command is not 'a', 'b', 'c': then break the loop
   *    3) Process the command as specified in the handout
   *  End loop
   *
   *  Print your collection of elements as specified in the handout
   *    as a comma delimited series of integers
   *-----------------------------------------------------------------*/
  int counter = 0;                                  // Counter variable for 'a' and 'b' commands
  int capacity = 8;                                 // Initial capacity for array
  int size = 0;                                     // Current size of the collection
  int *collection = malloc(capacity * sizeof(int)); // Memory allocation for collection
  if (!collection)
  { // Error handling for memory allocation failure
    write_string("Memory allocation failed\n");
    return -1; // Return -1 on failure
  }

  while (1)
  {
    int cmd = read_char(); // Read command
    if (cmd == EOF)
      break; // Break on EOF

    if (cmd == 'a')
    {
      // Add counter to collection
      if (size == capacity)
      {                                                                    // Resize array if needed
        capacity *= 2;                                                     // Double the capacity of the array
        int *new_collection = realloc(collection, capacity * sizeof(int)); // Reallocate memory
        if (!new_collection)
        {                   // Error handling for memory allocation failure
          free(collection); // Free old memory
          write_string("Memory allocation failed\n");
          return -1; // Return -1 on failure
        }
        collection = new_collection; // Update collection pointer
      }
      collection[size++] = counter; // Add counter to collection and increment size
      counter++;
    }
    else if (cmd == 'b')
    {
      counter++;
    }
    else if (cmd == 'c')
    {
      if (size > 0)
        size--; // Remove last element if collection is not empty
      counter++;
    }
    else
    {
      break;
    }
  }

  // Print collection as comma-separated list ending with ';' and newline
  for (int i = 0; i < size; ++i)
  {
    write_int(collection[i]); // Write integer to output
    if (i < size - 1)
    {
      write_char(','); // Add comma if not the last element
    }
  }
  write_char(';');  // End with semicolon
  write_char('\n'); // Newline

  free(collection); // Free allocated memory

  return 0; // Return 0 on success
}