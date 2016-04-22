/*
 *  bmpServer.c
 *  1917 serve that 3x3 bmp from lab3 Image activity
 *
 *  Created by Tim Lambert on 02/04/12.
 *  Containing code created by Richard Buckland on 28/01/11.
 *  Copyright 2012 Licensed under Creative Commons SA-BY-NC 3.0.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "mandelbrot.h"
#include "pixelColor.h"

int waitForConnection (int serverSocket);
int makeServerSocket (int portno);
void serveBMP (int socket, double x, double y, double zoom, int valuesScanned);
void writeHeader (int socket);
static int calcPoint (double *seed, double x, double y, int iteration);

#define SIZE 512
#define BMP_FILE "mandelbrot.bmp"
#define BYTES_PER_PIXEL 3
#define BITS_PER_PIXEL (BYTES_PER_PIXEL*8)
#define NUMBER_PLANES 1
#define PIX_PER_METRE 2835
#define MAGIC_NUMBER 0x4d42
#define NO_COMPRESSION 0
#define OFFSET 54
#define DIB_HEADER_SIZE 40
#define NUM_COLORS 0
#define BLACK 0
#define WHITE 255
#define ZERO 48
#define FALSE 0
#define TRUE 1

#define MAX_ITERATIONS 255
#define X 0
#define Y 1

#define SIMPLE_SERVER_VERSION 1.0
#define REQUEST_BUFFER_SIZE 1000
#define DEFAULT_PORT 1917
#define NUMBER_OF_PAGES_TO_SERVE -1
// after serving this many pages the server will halt

typedef unsigned char  bits8;
typedef unsigned short bits16;
typedef unsigned int   bits32;

int main (int argc, char *argv[]) {

   printf ("************************************\n");
   printf ("Starting simple server %f\n", SIMPLE_SERVER_VERSION);
   printf ("Serving bmps since 2012\n");

   int serverSocket = makeServerSocket (DEFAULT_PORT);
   printf ("Access this server at http://localhost:%d/\n", DEFAULT_PORT);
   printf ("************************************\n");

   char request[REQUEST_BUFFER_SIZE];

   int numberServed = 0;
   while (numberServed > NUMBER_OF_PAGES_TO_SERVE) {

      printf ("*** So far served %d pages ***\n", numberServed);

      int connectionSocket = waitForConnection (serverSocket);
      // wait for a request to be sent from a web browser, open a new
      // connection for this conversation

      // read the first line of the request sent by the browser
      int bytesRead;
      bytesRead = read (connectionSocket, request, (sizeof request)-1);
      assert (bytesRead >= 0);
      double x = 0.0;
      double y = 0.0;
      double zoom = 1.0;
      int valuesScanned = sscanf(request, "GET /tile_x%lf_y%lf_z%lf.bmp", &x, &y, &zoom);
      // were we able to read any data from the connection?

      // print entire request to the console
      printf (" *** Received http request ***\n %s\n", request);
      printf ("%f %f %f %d\n", x, y, zoom, valuesScanned);
      //send the browser a simple html page using http
      printf (" *** Sending http response ***\n");
      serveBMP(connectionSocket, x, y, zoom, valuesScanned);

      // close the connection after sending the page- keep aust beautiful
      close(connectionSocket);

      numberServed++;
   }

   // close the server connection after we are done- keep aust beautiful
   printf ("** shutting down the server **\n");
   close (serverSocket);

   return EXIT_SUCCESS;
}

void serveBMP (int socket, double x, double y, double zoom, int valuesScanned) {
   char* message;
   // first send the http response header

   // (if you write stings one after another like this on separate
   // lines the c compiler kindly joins them togther for you into
   // one long string)
   if (valuesScanned != 3) {
      message =
         "HTTP/1.0 200 Found\n"
         "Content-Type: text/html\n"
         "\n";
      printf ("about to send=> %s\n", message);
      write (socket, message, strlen (message));

      message =
         "<!DOCTYPE html>\n"
         "<script src=\"https://almondbread.cse.unsw.edu.au/tiles.js\"></script>"
         "\n";
      write (socket, message, strlen (message));
   } else {
      message = "HTTP/1.0 200 OK\r\n"
                  "Content-Type: image/bmp\r\n"
                  "\r\n";
      printf ("about to send=> %s\n", message);
      write (socket, message, strlen (message));

   // now send the BMP
   writeHeader (socket);
   double zoom_x = x - 1.5/zoom;
   double zoom_y = y - 2.0/zoom;
//   double seed[2] = {zoom_x,zoom_y};
   int colPos = 0;
   int colRow = 0;
   while (colPos < SIZE) {
      while (colRow < SIZE) {
         int steps = escapeSteps(zoom_x, zoom_y);
         unsigned char colour[3] =
         {0, steps, 0};
         write (socket, colour, sizeof(colour));
         zoom_x = zoom_x + 2.0/zoom/256.0;
         colRow ++;
      }
      zoom_y = zoom_y + 2.0/zoom/256.0;
      zoom_x = x - 1.5/zoom;
      colPos ++;
      colRow = 0;
   }
   }
}

int escapeSteps (double x, double y) {
   //int iteration = 1;
   double seed[2] = {x, y};
   int iteration = calcPoint(seed, x, y, 1);
   return iteration;
}

static int calcPoint (double *seed, double x, double y, int iteration) {
   //printf ("%f %f %d\n", x, y, iteration);
   if ((iteration == 256) | ((x*x + y*y) >= 4.0) ) {
      return (iteration);
   } else {
      double x_new = seed[0] + (x*x - y*y);
      double y_new = seed[1] + (2.0*x*y);
      return (calcPoint(seed, x_new, y_new, iteration+1));
   }
}


// start the server listening on the specified port number
int makeServerSocket (int portNumber) {

   // create socket
   int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
   assert (serverSocket >= 0);
   // error opening socket

   // bind socket to listening port
   struct sockaddr_in serverAddress;
   memset ((char *) &serverAddress, 0,sizeof (serverAddress));

   serverAddress.sin_family      = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port        = htons (portNumber);

   // let the server start immediately after a previous shutdown
   int optionValue = 1;
   setsockopt (
      serverSocket,
      SOL_SOCKET,
      SO_REUSEADDR,
      &optionValue,
      sizeof(int)
   );

   int bindSuccess =
      bind (
         serverSocket,
         (struct sockaddr *) &serverAddress,
         sizeof (serverAddress)
      );

   assert (bindSuccess >= 0);
   // if this assert fails wait a short while to let the operating
   // system clear the port before trying again

   return serverSocket;
}


// wait for a browser to request a connection,
// returns the socket on which the conversation will take place
int waitForConnection (int serverSocket) {
   // listen for a connection
   const int serverMaxBacklog = 10;
   listen (serverSocket, serverMaxBacklog);

   // accept the connection
   struct sockaddr_in clientAddress;
   socklen_t clientLen = sizeof (clientAddress);
   int connectionSocket =
      accept (
         serverSocket,
         (struct sockaddr *) &clientAddress,
         &clientLen
      );

   assert (connectionSocket >= 0);
   // error on accept

   return (connectionSocket);
}

void writeHeader (int socket) {
   assert(sizeof (bits8) == 1);
   assert(sizeof (bits16) == 2);
   assert(sizeof (bits32) == 4);

   bits16 magicNumber = MAGIC_NUMBER;
   write (socket, &magicNumber, sizeof magicNumber);

   bits32 fileSize = OFFSET + (SIZE * SIZE * BYTES_PER_PIXEL);
   write (socket, &fileSize, sizeof fileSize);

   bits32 reserved = 0;
   write (socket, &reserved, sizeof reserved);

   bits32 offset = OFFSET;
   write (socket, &offset, sizeof offset);

   bits32 dibHeaderSize = DIB_HEADER_SIZE;
   write (socket, &dibHeaderSize, sizeof dibHeaderSize);

   bits32 width = SIZE;
   write (socket, &width, sizeof width);

   bits32 height = SIZE;
   write (socket, &height, sizeof height);

   bits16 planes = NUMBER_PLANES;
   write (socket, &planes, sizeof planes);

   bits16 bitsPerPixel = BITS_PER_PIXEL;
   write (socket, &bitsPerPixel, sizeof bitsPerPixel);

   bits32 compression = NO_COMPRESSION;
   write (socket, &compression, sizeof compression);

   bits32 imageSize = (SIZE * SIZE * BYTES_PER_PIXEL);
   write (socket, &imageSize, sizeof imageSize);

   bits32 hResolution = PIX_PER_METRE;
   write (socket, &hResolution, sizeof hResolution);

   bits32 vResolution = PIX_PER_METRE;
   write (socket, &vResolution, sizeof vResolution);

   bits32 numColors = NUM_COLORS;
   write (socket, &numColors, sizeof numColors);

   bits32 importantColors = NUM_COLORS;
   write (socket, &importantColors, sizeof importantColors);
}
