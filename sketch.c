// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState(void) {
  state *s = malloc(sizeof(state));
  s->x = 0;
  s->y = 0;
  s->tx = 0;
  s->ty = 0;
  s->tool = LINE;
  s->data = 0;
  s->start = 0;
  s->end = false;
  return s;
}

// Release all memory associated with the drawing state
void freeState(state *s) { 
  free (s);
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
  return (b & 0xC0) >> 6; 
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
  int x = (b & 0x3F);
  if ((x & 0x20) == 0x20){
    return (x - 64);
  } 
  else return x;
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  int x = getOpcode (op);
  int y = getOperand (op);
  if (x == DX) {
    s -> tx += y;
  }
  else if (x == DY) {
    s -> ty += y;
    if (s -> tool == LINE){
      line (d, s->x, s->y, s->tx, s->ty);
    }
    s->x = s->tx;
    s->y= s->ty;
  }
  else if (x == TOOL){
    if (y == NONE) {
      s->tool = NONE;
    } 
    else if (y == LINE){
      s->tool = LINE;
    }
  }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, const char pressedKey, void *data) {
  if (data == NULL) {
    return (pressedKey == 27);
  }
  state *s = (state*) data;
  char *filename = getName(d);
  FILE *file = fopen(filename, "rb");
  byte op; 
  while (! feof(file)){
     op = fgetc(file);
     obey(d, s, op);
  }
  fclose (file);
  show(d);
  s->x = 0;
  s->y = 0;
  s->tx = 0;
  s->ty = 0;
  s->tool = LINE;
  s->data = 0;
  s->end = false;
  return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
