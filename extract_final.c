// extract.c
// funtions and types used to extract x,y,z values from a
// string containing a url of the form
// "http://almondbread.cse.unsw.edu.au:7191/tile_x3.14_y-0.141_z5.bmp"
// initially by richard buckland
// 13 April 2014
// your name here: Jack Vincent
//Modified date: 04/20/16

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "extract.h"

#define ZERO 48
#define FALSE 0
#define TRUE 1

int main (int argc, char *argv[]) {

    char * message = "http://almondbread.cse.unsw.edu.au:7191/tile_x3.14_y-0.141_z5.bmp";

    triordinate dat = extract (message);

    printf ("dat is (%f, %f, %d)\n", dat.x, dat.y, dat.z);

    assert (dat.x == 3.14);
    assert (dat.y == -0.141);
    assert (dat.z == 5);

    return EXIT_SUCCESS;
}

triordinate extract (char *message) {
   triordinate dat;

   char xLoc[40];
   char yLoc[40];
   char zLoc[40];
   int pos = 0;
   int arrayPos = 0;
  // message = strstr(message, "_x");
   while (message[pos] != 'x') { // removing this while loop made x worse
      pos++;
   }

   pos++;

   while (message[pos] != '_') {
      xLoc[arrayPos] = message[pos];
      arrayPos++;
      pos++;
      //printf ("xLoc is: %s\n", xLoc);
   }
   //arrayPos++;
   xLoc[arrayPos] = '\0';
   pos += 2;
   arrayPos = 0;

   while (message[pos] != '_') {
      yLoc[arrayPos] = message[pos];
      arrayPos++;
      pos++;
   }
   //arrayPos++;
   yLoc[arrayPos] = '\0';
   pos += 2;
   arrayPos = 0;

   while (message[pos] != '.') {
      zLoc[arrayPos] = message[pos];
      arrayPos++;
      pos++;
   }
   //arrayPos++;
   zLoc[arrayPos] = '\0';
   dat.x = myAtoD (xLoc);
   dat.y = myAtoD (yLoc);
   dat.z = myAtoL (zLoc);
   //added printf to test
   //printf ("at the end of extract: x= %f, y= %f, z=%d\n", dat.x, dat.y, dat.z);

   return dat;

}

double myAtoD (char *message) {
   //added printf to test
   //printf ("before myAtoD: %s\n", message);
   int i = 0; //start of string
   int length = strlen(message)-1; //string length
   int num1 = 0; //changed num1 from an int to a double
   double num2 = 0.0;
   int negative = FALSE;
   if (message[i] == 45) {
      i++;
      negative = TRUE;
   }
   while (message[i] != '.') {
      num1 *= 10;
      num1 += (message[i]-ZERO);
      i++;
   }
   while (length > i) {
      num2 += (message[length]-ZERO);
      num2 /= 10.0;
      length--;
   }
   num2 = num1 + num2;
   if (negative == TRUE) {
      num2 *= -1.0;
   }
   //added printf to test
   //printf ("myAtoD gives: %f\n", num2);

   return num2;

}

long myAtoL (char *message) {
   int Pos = 0; //start of string position
   long numLong = 0;
   while (message[Pos] != '\0') {
      if (message[Pos] >= '0' && message[Pos] <= '9') {
         numLong *= 10;
         numLong += (message[Pos] - ZERO);
      }
      Pos++;
  }

  if (message[0] == '-') {
     numLong = numLong * -1;
  }

  return numLong;

}


