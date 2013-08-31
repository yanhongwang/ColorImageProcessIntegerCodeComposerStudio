#include <stdio.h>
#include <time.h>
#include <string.h>

#include "BMP.h"
#include "AutoWhiteBalance.h"
#include "RGBCIELAB.h"
#include "LookTable_pow_045.h"
#include "ColorImageProcess.h"
#include <stdlib.h>
// RGB2XYZ
int RGB2XYZConst1 = ( int )( 0.412453 * ScaleNumber + 0.5 );
int RGB2XYZConst2 = ( int )( 0.357580 * ScaleNumber + 0.5 );
int RGB2XYZConst3 = ( int )( 0.180423 * ScaleNumber + 0.5 );
int RGB2XYZConst4 = ( int )( 0.212671 * ScaleNumber + 0.5 );
int RGB2XYZConst5 = ( int )( 0.715160 * ScaleNumber + 0.5 );
int RGB2XYZConst6 = ( int )( 0.072169 * ScaleNumber + 0.5 );
int RGB2XYZConst7 = ( int )( 0.019334 * ScaleNumber + 0.5 );
int RGB2XYZConst8 = ( int )( 0.119193 * ScaleNumber + 0.5 );
int RGB2XYZConst9 = ( int )( 0.950227 * ScaleNumber + 0.5 );

int RGB2LabLimit = ( int )( 0.008856 * ScaleNumber + 0.5 );

// XYZ2RGB
int XYZ2RGBConst1 = ( int )( 3.240479 * ScaleNumber + 0.5 );
int XYZ2RGBConst2 = ( int )( 1.537150 * ScaleNumber + 0.5 );
int XYZ2RGBConst3 = ( int )( 0.498535 * ScaleNumber + 0.5 );
int XYZ2RGBConst4 = ( int )( 0.969256 * ScaleNumber + 0.5 );
int XYZ2RGBConst5 = ( int )( 1.875992 * ScaleNumber + 0.5 );
int XYZ2RGBConst6 = ( int )( 0.041556 * ScaleNumber + 0.5 );
int XYZ2RGBConst7 = ( int )( 0.055648 * ScaleNumber + 0.5 );
int XYZ2RGBConst8 = ( int )( 0.204043 * ScaleNumber + 0.5 );
int XYZ2RGBConst9 = ( int )( 1.057311 * ScaleNumber + 0.5 );

int Lab2RGBLimit = ( int )( 0.206893 * ScaleNumber * 6.3496042078727978990068225569719 * ScaleNumber * ScaleNumber + 0.5 );

// for color space transformation
// pow( 256, 0.3 ) = 6.3496042078727978990068225569719
int pow_16_256_1_3 = ( int )( 16 * 6.3496042078727978990068225569719 * ScaleNumber + 0.5 );

#define SDRAM_BASE 0x80000000

#define MY_CLOCKS_PER_SEC ( double )1000000000 // 1000 MHz

#define	Count	1

#define	EnableAL		// Auto Level
#define	EnableWB		// White Balance
#define	EnableCC		// Color Correction
//#define	EnableSpace		// Color Space Transformation
#define	EnableSE		// Saturation Enhancement
//#define	EnableTR		// Tone Reproduction
#define	EnableGC		// Gamma Correction

// space transformation must be done before Saturation Enhancement 
#ifdef	EnableSE		// Saturation Enhancement
#define	EnableSpace
#endif

// space transformation must be done before Tone Reproduction 
#ifdef	EnableTR		// Tone Reproduction
#define	EnableSpace
#endif
FILE * test;
void main()
{
	
	int i;
	
	int Index;
	
	int RIndex;
	int GIndex;
	int BIndex;
	
	int imageWidth = 0;
	int imageHeight = 0;
	int TotalPixelCount = 0;
	
	struct BitMapFileHeader header1;
	struct BitMapInfoHeader header2;
	
	unsigned char *ImageDataBlock;
	int *ImageDataBlock_int;
	
	int Rtemp;
	int Gtemp;
	int Btemp;
	
	// Auto Level
	// if no parenthesis, parser will be misread
	int HighThreshold = ( int )( 250 << ScaleBit );
	int LowThreshold = ( int )( 5  << ScaleBit );
	int AutoLevelMultiple = ( int )( ( 256 << ScaleBit ) / ( HighThreshold - LowThreshold ) + 0.5 );
	
	// AutoWhiteBalance
	int RCoefficient;
	int BCoefficient;
	int tempValue;
	
	int WBRCorrection = ( int )( ScaleNumber * 1.15 + 0.5 );
	int WBBCorrection = ( int )( ScaleNumber / 1.15 + 0.5 );
	
	// Color Correction
	int tempRValue;
	int tempGValue;
	int tempBValue;
	
	// color correction registering constants
	int CC1 = ( int )( ScaleNumber / 512.0 * 860.0 + 0.5 );	// for R element
	int CC2 = ( int )( ScaleNumber / 512.0 * 253.0 + 0.5 );	// for R element
	int CC3 = ( int )( ScaleNumber / 512.0 *  95.0 + 0.5 );	// for R element
	int CC4 = ( int )( ScaleNumber / 512.0 * 109.0 + 0.5 );	// for G element
	int CC5 = ( int )( ScaleNumber / 512.0 * 928.0 + 0.5 );	// for G element
	int CC6 = ( int )( ScaleNumber / 512.0 * 307.0 + 0.5 );	// for G element
	int CC7 = ( int )( ScaleNumber / 512.0 *  20.0 + 0.5 );	// for B element
	int CC8 = ( int )( ScaleNumber / 512.0 * 290.0 + 0.5 );	// for B element
	int CC9 = ( int )( ScaleNumber / 512.0 * 782.0 + 0.5 );	// for B element
	
	// sRGB and CIELAB
	int L;
	int a;
	int b;
	
	// Color Saturation Enhancement
	int aFactor = ( int )( 1.1 * ScaleNumber + 0.5 );
	int bFactor	= ( int )( 1.1 * ScaleNumber + 0.5 );
	
	FILE *fptr;
	
	// algorithm efficiency measurement
	time_t	start;
	time_t	now;
	
	clock_t start_cpu;
	clock_t finish_cpu;
	clock_t overhead;
	
	char FileName[ 30 ];
	
	char OriFileName[ 15 ];
	scanf( "%s", FileName );
	
	sprintf( OriFileName, "%s%s", FileName, ".BMP" );
	
	fptr = fopen( OriFileName, "rb" );
	test = fopen( "test.txt", "w" );
	fread( &header1, 14, 1, fptr );
	
	fread( &header2, 40, 1, fptr );
	
	imageWidth = header2.biWidth;
	imageHeight = header2.biHeight;
	TotalPixelCount = imageHeight * imageWidth * 3;
	
	ImageDataBlock = ( unsigned char * )SDRAM_BASE;
	ImageDataBlock_int = ( int * )( SDRAM_BASE + TotalPixelCount );
	
	fread( ImageDataBlock, TotalPixelCount, sizeof( unsigned char ), fptr );
	
	fclose( fptr );
	
	// algorithm begin
	printf( "algorithm start\n\n" );
	
	printf( "Do loop %d counts with %f Hz for testbench.\n\n", Count, MY_CLOCKS_PER_SEC );
	
	// system count start
	start = time( NULL );
	
	// clock count start
	start_cpu = clock();
	finish_cpu = clock();
	
	// Calculate the overhead from calling clock()
	overhead = finish_cpu - start_cpu;
	
	start_cpu = clock();
	
	for( i = 0; i < Count; i ++ )
	{
		
		Index = 0;
		
		while( Index < TotalPixelCount )
		{
			
			BIndex = Index ++;
			GIndex = Index ++;
			RIndex = Index ++;
			
			ImageDataBlock_int[ BIndex ] = ImageDataBlock[ BIndex ] << ScaleBit;
			ImageDataBlock_int[ GIndex ] = ImageDataBlock[ GIndex ] << ScaleBit;
			ImageDataBlock_int[ RIndex ] = ImageDataBlock[ RIndex ] << ScaleBit;
			//	fprintf( test, "R = %d\tG = %d\tB = %d\n" , ImageDataBlock_int[ RIndex ], ImageDataBlock_int[ GIndex ], ImageDataBlock_int[ BIndex ] );
		}
		
		Index = 0;
		
#ifdef EnableAL
		
		// Auto Level
		while( Index < TotalPixelCount )
		{
			
			BIndex = Index ++;
			GIndex = Index ++;
			RIndex = Index ++;
			
			// B channel
			if( ( ImageDataBlock_int[ BIndex ] > LowThreshold ) && ( ImageDataBlock_int[ BIndex ] < HighThreshold ) )
				ImageDataBlock_int[ BIndex ] = ( ImageDataBlock_int[ BIndex ] - LowThreshold ) * AutoLevelMultiple;
			else if( ImageDataBlock_int[ BIndex ] <= LowThreshold )
				ImageDataBlock_int[ BIndex ] = MinThreshold;
			else	// ImageDataBlock_int[ BIndex ] >= HighThreshold
				ImageDataBlock_int[ BIndex ] = MaxThreshold;
			
			// G channel
			if( ( ImageDataBlock_int[ GIndex ] > LowThreshold ) && ( ImageDataBlock_int[ GIndex ] < HighThreshold ) )
				ImageDataBlock_int[ GIndex ] = ( ImageDataBlock_int[ GIndex ] - LowThreshold ) * AutoLevelMultiple;
			else if( ImageDataBlock_int[ GIndex ] <= LowThreshold )
				ImageDataBlock_int[ GIndex ] = MinThreshold;
			else	// ImageDataBlock_int[ GIndex ] >= HighThreshold
				ImageDataBlock_int[ GIndex ] = MaxThreshold;
			
			// R channel
			if( ( ImageDataBlock_int[ RIndex ] > LowThreshold ) && ( ImageDataBlock_int[ RIndex ] < HighThreshold ) )
				ImageDataBlock_int[ RIndex ] = ( ImageDataBlock_int[ RIndex ] - LowThreshold ) * AutoLevelMultiple;
			else if( ImageDataBlock_int[ RIndex ] <= LowThreshold )
				ImageDataBlock_int[ RIndex ] = MinThreshold;
			else	// ImageDataBlock_int[ RIndex ] >= HighThreshold
				ImageDataBlock_int[ RIndex ] = MaxThreshold;
		//	fprintf( test, "R = %d\tG = %d\tB = %d\n" , ImageDataBlock_int[ RIndex ], ImageDataBlock_int[ GIndex ], ImageDataBlock_int[ BIndex ] );
		}
		
#endif	// EnableAL
		
#ifdef	EnableWB
		
		// Auto White Balance
		ColorTemperatureCurve
		(
			
			ImageDataBlock_int,
			&RCoefficient,
			&BCoefficient,
			TotalPixelCount
			
		);
		
		// inaccurate experiment cause inaccurate ctc coefficient
		// so that we need to manually correct
	//	RCoefficient = ( RCoefficient * WBRCorrection ) >> ScaleBit;
	//	BCoefficient = ( BCoefficient * WBBCorrection ) >> ScaleBit;
	//	fprintf( test, "new RCoefficient = %d\tnew BCoefficient = %d\n", RCoefficient, BCoefficient );
#endif	// EnableWB
		
		Index = 0;
		
		while( Index < TotalPixelCount )
		{
			
			BIndex = Index ++;
			GIndex = Index ++;
			RIndex = Index ++;
			
#ifdef	EnableWB
			
			// Auto White Balance
			// operation may cause over range
			tempValue = ( ImageDataBlock_int[ BIndex ] * BCoefficient ) >> ScaleBit;
			if( tempValue <= MaxThreshold )
				Btemp = tempValue;
			else
				Btemp = MaxThreshold;
			
			Gtemp = ImageDataBlock_int[ GIndex ];
			
			tempValue = ( ImageDataBlock_int[ RIndex ] * RCoefficient ) >> ScaleBit;
			if( tempValue <= MaxThreshold )
				Rtemp = tempValue;
			else
				Rtemp = MaxThreshold;
		//	fprintf( test, "R = %d\tG = %d\tB = %d\n" , ImageDataBlock_int[ RIndex ], ImageDataBlock_int[ GIndex ], ImageDataBlock_int[ BIndex ] );
#else
			
			// no Auto White Balance
			Btemp = ImageDataBlock_int[ BIndex ];
			Gtemp = ImageDataBlock_int[ GIndex ];
			Rtemp = ImageDataBlock_int[ RIndex ];
			
#endif	// EnableWB
			
#ifdef	EnableCC
			
			// Color Correction
			tempRValue = ( int )(  Rtemp * CC1 - Gtemp * CC2 - Btemp * CC3 ) >> ScaleBit;	// R element
			tempGValue = ( int )( -Rtemp * CC4 + Gtemp * CC5 - Btemp * CC6 ) >> ScaleBit;	// G element
			tempBValue = ( int )(  Rtemp * CC7 - Gtemp * CC8 + Btemp * CC9 ) >> ScaleBit;	// B element
			
			if( ( tempBValue <= MaxThreshold ) && ( tempBValue >= MinThreshold ) )
				Btemp = tempBValue;
			else if( tempBValue < MinThreshold )
				Btemp = MinThreshold;
			else
				Btemp = MaxThreshold;
			
			if( ( tempGValue <= MaxThreshold ) && ( tempGValue >= MinThreshold ) )
				Gtemp = tempGValue;
			else if( tempGValue < MinThreshold )
				Gtemp = MinThreshold;
			else
				Gtemp = MaxThreshold;
			
			if( ( tempRValue <= MaxThreshold ) && ( tempRValue >= MinThreshold ) )
				Rtemp = tempRValue;
			else if( tempRValue < MinThreshold )
				Rtemp = MinThreshold;
			else
				Rtemp = MaxThreshold;
			
#endif	// EnableCC

#ifdef	EnableSpace
			
			// sRGB to CIELAB
			RGB2Lab( Btemp, Gtemp, Rtemp, &L, &a, &b );
			
#endif	// EnableSpace
			
#ifdef	EnableSE
			
			// Saturation Enhancement
			a = ( a * aFactor ) >> ScaleBit;
			b = ( b * bFactor ) >> ScaleBit;
			
#endif	// EnableSE

#ifdef	EnableSpace
			
			// CIELAB to sRGB
			Lab2RGB( L, a, b, &Btemp, &Gtemp, &Rtemp );
			
#endif	// EnableSpace
			
#ifdef	EnableGC
			
			// Gamma Correction
			ImageDataBlock_int[ BIndex ] = pow_0_255_256_045[ Btemp >> ScaleBit ];
			ImageDataBlock_int[ GIndex ] = pow_0_255_256_045[ Gtemp >> ScaleBit ];
			ImageDataBlock_int[ RIndex ] = pow_0_255_256_045[ Rtemp >> ScaleBit ];
			
#else
			
			// no Gamma Correction
			ImageDataBlock_int[ BIndex ] = Btemp;
			ImageDataBlock_int[ GIndex ] = Gtemp;
			ImageDataBlock_int[ RIndex ] = Rtemp;
			
#endif	// EnableGC
			
		}
		
	}
	
	// algorithm end
	finish_cpu = clock();
	now = time( NULL );
	printf( "System elapsed time = %d seconds\n", now - start );
	
	printf( "CPU Time = %u cycle\n", finish_cpu - start_cpu - overhead );
	printf( "CPU Time = %f seconds\n\n", ( finish_cpu - start_cpu - overhead ) / MY_CLOCKS_PER_SEC );
	
#ifdef	EnableAL
	strcat( FileName, "_AL" );
#endif
#ifdef	EnableWB
	strcat( FileName, "_WB" );
#endif
#ifdef	EnableCC
	strcat( FileName, "_CC" );
#endif
#ifdef	EnableSE
	strcat( FileName, "_SE" );
#endif
#ifdef	EnableGC
	strcat( FileName, "_GC" );
#endif
	
	strcat( FileName, ".BMP" );
	printf("%s\n", FileName );
	
	fptr = fopen( FileName, "wb" );
	
	fwrite( &header1, 14, 1, fptr );
	
	fwrite( &header2, 40, 1, fptr );
	
	Index = 0;
	
	while( Index < TotalPixelCount )	
		ImageDataBlock[ Index ++ ] = ImageDataBlock_int[ Index ] >> ScaleBit;
	
	fwrite( ImageDataBlock, TotalPixelCount, sizeof( unsigned char ), fptr );
	
	fclose( fptr );
	fclose( test );
	printf( "\ndone\n" );
//	system( "dir" );
}
