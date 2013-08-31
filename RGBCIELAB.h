#ifndef RGBCIELAB

#define RGBCIELAB

void Lab2RGB( int L, int a, int b, int *B, int *G, int *R );
void Lab2XYZ( int L, int a, int b, int *X, int *Y, int *Z );
void XYZ2RGB( int X, int Y, int Z, int *B, int *G, int *R );

void RGB2XYZ( int B, int G, int R, int *X, int *Y, int *Z );
void XYZ2Lab( int X, int Y, int Z, int *L, int *a, int *b );
void RGB2Lab( int B, int G, int R, int *L, int *a, int *b );

#endif
