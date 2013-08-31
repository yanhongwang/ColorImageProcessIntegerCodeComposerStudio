#include "RGBCIELAB.h"
#include "ColorImageProcess.h"
#include "LookTable_pow_033.h"

extern int RGB2XYZConst1;
extern int RGB2XYZConst2;
extern int RGB2XYZConst3;
extern int RGB2XYZConst4;
extern int RGB2XYZConst5;
extern int RGB2XYZConst6;
extern int RGB2XYZConst7;
extern int RGB2XYZConst8;
extern int RGB2XYZConst9;

extern int RGB2LabLimit;

extern int XYZ2RGBConst1;
extern int XYZ2RGBConst2;
extern int XYZ2RGBConst3;
extern int XYZ2RGBConst4;
extern int XYZ2RGBConst5;
extern int XYZ2RGBConst6;
extern int XYZ2RGBConst7;
extern int XYZ2RGBConst8;
extern int XYZ2RGBConst9;

extern int Lab2RGBLimit;

extern int pow_16_256_1_3;

//	amin =  -86, amax = 98
//	bmin = -108, bmax = 94
//	86 + 98 + 1 = 185
//	108 + 94 + 1 = 203

void XYZ2RGB( int X, int Y, int Z, int *B, int *G, int *R )
{
	
	int RR;
	int GG;
	int BB;
	
	//( *X ) *= ( 0.950456 *  256.0 );
	//( *Y ) *=               256.0;
	//( *Z ) *= ( 1.088754 *  256.0 );
	//RR =  3.240479 * X - 1.537150 * Y - 0.498535 * Z;
	//GG = -0.969256 * X + 1.875992 * Y + 0.041556 * Z;
	//BB =  0.055648 * X - 0.204043 * Y + 1.057311 * Z;
	
	X = ( X * 243 ) >> ScaleBit;
	Y = ( Y * 256 ) >> ScaleBit;
	Z = ( Z * 279 ) >> ScaleBit;
	
	RR = (  XYZ2RGBConst1 * X - XYZ2RGBConst2 * Y - XYZ2RGBConst3 * Z ) >> ( ScaleBit + ScaleHalfBit );
	GG = ( -XYZ2RGBConst4 * X + XYZ2RGBConst5 * Y + XYZ2RGBConst6 * Z ) >> ( ScaleBit + ScaleHalfBit );
	BB = (  XYZ2RGBConst7 * X - XYZ2RGBConst8 * Y + XYZ2RGBConst9 * Z ) >> ( ScaleBit + ScaleHalfBit );
	
	if( ( RR < MaxThreshold ) && ( RR > MinThreshold ) )
		*R = RR;
	else if( RR <= MinThreshold )
		*R = MinThreshold;
	else
		*R = MaxThreshold;
	
	if( ( GG < MaxThreshold ) && ( GG > MinThreshold ) )
		*G = GG;
	else if( GG <= MinThreshold )
		*G = MinThreshold;
	else
		*G = MaxThreshold;
	
	if( ( BB < MaxThreshold ) && ( BB > MinThreshold ) )
		*B = BB;
	else if( BB <= MinThreshold )
		*B = MinThreshold;
	else
		*B = MaxThreshold;
	
}

void Lab2XYZ( int L, int a, int b, int *X, int *Y, int *Z )
{
	
	int fX, fY, fZ;
	
	// proto type formulation
	// fY = ( L + pow_16_256_1_3 ) / 116;
	// fX = a / 500 + fY;
	// fZ = fY - b / 200;
	
	// fY = ( ( L + pow_16_256_1_3 ) / 116 ) << ( `ScaleBit + `ScaleBit );
	// fX = ( ( a / 500 ) << ( `ScaleBit + `ScaleBit ) ) + fY;
	// fZ = fY - ( b / 200 ) << ( `ScaleBit + `ScaleBit );
	
	// avoid usage of the division
	fY = ( L + pow_16_256_1_3 ) * 565;
	fX = a * 131 + fY;
	fZ = fY - b * 328;
	
	// avoid extreme case of fY
	if( fY < Lab2RGBLimit )
		fY = Lab2RGBLimit;
	
	// avoid extreme case of fX
	if( fX < Lab2RGBLimit )
		fX = Lab2RGBLimit;
	
	// avoid extreme case of fZ
	if( fZ < Lab2RGBLimit )
		fZ = Lab2RGBLimit;
	
	// in case of over-range of power 3 operation later
	// fY = fY >> ScaleHalfBit;
	fY = fY >> ( ScaleHalfBit + ScaleBit + ScaleBit );
	*Y = fY * fY * fY;
	
	// in case of over-range of power 3 operation later
	// fX = fX >> ScaleHalfBit;
	fX = fX >> ( ScaleHalfBit + ScaleBit + ScaleBit );
	*X = fX * fX * fX;
	
	// in case of over-range of power 3 operation later
	//fZ = fZ >> ScaleHalfBit;
	fZ = fZ >> ( ScaleHalfBit + ScaleBit + ScaleBit );
	*Z = fZ * fZ * fZ;
	
}

void Lab2RGB( int L, int a, int b, int *B, int *G, int *R )
{
	
	int X, Y, Z;
	
	Lab2XYZ( L, a, b, &X, &Y, &Z );
	
	XYZ2RGB( X, Y, Z, B, G, R );
	
}

void RGB2XYZ( int B, int G, int R, int *X, int *Y, int *Z )
{
	
	// *X = ( 0.412453 * R ) + ( 0.357580 * G ) + ( 0.180423 * B );
	// *Y = ( 0.212671 * R ) + ( 0.715160 * G ) + ( 0.072169 * B );
	// *Z = ( 0.019334 * R ) + ( 0.119193 * G ) + ( 0.950227 * B );
	
	// with the D65 reference white: Xn = 0.9505; Yn = 1.00; Zn = 1.0891;
	//	X /= ( 256 * 0.950456 );
	//	Y /=   256;
	//	Z /= ( 256 * 1.088754 );
	
	// 256 * 0.950456 * 256
	*X = ( RGB2XYZConst1 * R + RGB2XYZConst2 * G + RGB2XYZConst3 * B ) >> ( ScaleBit + ScaleBit );
	*X = ( *X * 269 ) >> ScaleBit;	// 256 / 0.950456 = 269.34439
	
	*Y = ( RGB2XYZConst4 * R + RGB2XYZConst5 * G + RGB2XYZConst6 * B ) >> ( ScaleBit + ScaleBit );
	
	// 256 * 1.088754 * 256
	*Z = ( RGB2XYZConst7 * R + RGB2XYZConst8 * G + RGB2XYZConst9 * B ) >> ( ScaleBit + ScaleBit );
	*Z = ( *Z * 235 ) >> ScaleBit;	// 256 / 1.088754 = 235.13116
	
}

void XYZ2Lab( int X, int Y, int Z, int *L, int *a, int *b )
{
	
	int fX, fY, fZ;
	
	// avoid extreme case of Y
	if( Y < RGB2LabLimit )
		Y = RGB2LabLimit;
	
	// avoid extreme case of X
	if( X < RGB2LabLimit )
		X = RGB2LabLimit;
	
	// avoid extreme case of Z
	if( Z < RGB2LabLimit )
		Z = RGB2LabLimit;
	
	fY = pow_0_256_256_033[ Y ];
	
	*L = ( 116 * fY ) - pow_16_256_1_3;
	
	fX = pow_0_256_256_033[ X ];
	
	fZ = pow_0_256_256_033[ Z ];
	
	*a = 500 * ( fX - fY );
	*b = 200 * ( fY - fZ );
	
}

void RGB2Lab( int B, int G, int R, int *L, int *a, int *b )
{
	
	int X, Y, Z;
	
	RGB2XYZ( B, G, R, &X, &Y, &Z );
	
	XYZ2Lab( X, Y, Z, L, a, b );
	
}
