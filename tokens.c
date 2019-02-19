#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svec.h"
#include "tokens.h"

// Uses svec from the lecture notes by Professor Tuck.

/**
 * @brief Converts a string into a vector of tokens split along whitespace
 * or the following operators: <, >, ;, &, &&, |, ||
 *
 * Strings with quotes are considered tokens and will be stored as a single
 * token without quotes. (, ), and \ are also considered their own tokens.
 *
 * @param line is the string to tokenize.
 *
 * @return the vector containing the tokens in sequential order.
 */
svec* tokenize(char* line) {
  svec* tokens = make_svec(0);
  char buffer[1024];
  int bufferEnd = 0;
  for (long i = 0; i <= strlen(line); i++) {
    char* readPtr = line + i;
    if (isspace(*readPtr) || *readPtr == 0) {
      buffer[bufferEnd] = 0;
      if (bufferEnd > 0) {
        svec_push_back(tokens, buffer);
      }
      bufferEnd = 0;
    } else if (*readPtr == '<' || *readPtr == '>' || *readPtr == ';' ||
               *readPtr == '(' || *readPtr == ')' || *readPtr == '\\') {
      buffer[bufferEnd] = 0;
      if (bufferEnd > 0) {
        svec_push_back(tokens, buffer);
      }
      bufferEnd = 0;
      buffer[bufferEnd] = *readPtr;
      bufferEnd++;
      buffer[bufferEnd] = 0;
      svec_push_back(tokens, buffer);
      bufferEnd = 0;
    } else if (*readPtr == '&' || *readPtr == '|') {
      buffer[bufferEnd] = 0;
      if (bufferEnd > 0) {
        svec_push_back(tokens, buffer);
      }
      bufferEnd = 0;
      buffer[bufferEnd] = *readPtr;
      bufferEnd++;
      if (*(readPtr + 1) == *readPtr) {
        i++;
        buffer[bufferEnd] = *readPtr;
        bufferEnd++;
      }
      buffer[bufferEnd] = 0;
      svec_push_back(tokens, buffer);
      bufferEnd = 0;
    } else if (*readPtr == '"') {
      char* start = readPtr;
      int chars = 0;
      do {
        readPtr++;
        chars++;
      } while (*readPtr != '"');
      // Account for last quote counted
      chars--;
      memcpy(buffer + bufferEnd, start + 1, chars);
      bufferEnd += chars;
      buffer[bufferEnd] = 0;
      svec_push_back(tokens, buffer);
      bufferEnd = 0;
      i += chars + 1;
    } else {
      buffer[bufferEnd] = *readPtr;
      bufferEnd++;
    }
  }

  return tokens;
}
