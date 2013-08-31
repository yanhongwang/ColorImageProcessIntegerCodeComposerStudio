#include "AutoWhiteBalance.h"
#include "ColorImageProcess.h"
#include "CTCR.h"
#include "CTCB.h"

//#include <stdio.h>
//extern FILE *test;

void ColorTemperatureCurve
(
	
	int *ImageDataBlock_int,
	int *RCoefficient,
	int *BCoefficient,
	int TotalPixelCount
	
)
{
	
	int Index = 0;
	
	int BIndex;
	int GIndex;
	int RIndex;
	
	int GRIndex;
	int GBIndex;
	
	// in case of overflow
	long int RTotal = 0;
	long int GTotal = 0;
	long int BTotal = 0;
	
	while( Index < TotalPixelCount )
	{
		
		BIndex = Index ++;
		GIndex = Index ++;
		RIndex = Index ++;
		
		BTotal += ImageDataBlock_int[ BIndex ];
		GTotal += ImageDataBlock_int[ GIndex ];
		RTotal += ImageDataBlock_int[ RIndex ];
		
	}
//	fprintf( test, "RTotal = %ld, GTotal = %ld, BTotal = %ld\n", RTotal, GTotal, BTotal );
	
	GRIndex = ( int )( ( double )GTotal / ( RTotal >> ScaleHalfBit ) + 0.5 );
	GBIndex = ( int )( ( double )GTotal / ( BTotal >> ScaleHalfBit ) + 0.5 );
	
//	fprintf( test, "old GRIndex = %d, old GBIndex = %d\n", GRIndex, GBIndex );
	
	if( ( GRIndex >= 16 ) && ( GRIndex <= 40 ) )
		GRIndex -= 16;
	else if( GRIndex < 16 )
		GRIndex = 0;
	else
		GRIndex = 23;
	
	if( ( GBIndex >= 16 ) && ( GBIndex <= 40 ) )
		GBIndex -= 16;
	else if( GBIndex < 16 )
		GBIndex = 0;
	else
		GBIndex = 23;
//	fprintf( test, "new GRIndex = %d, new GBIndex = %d, Index = %d\n", GRIndex, GBIndex, GRIndex * 24 + GBIndex );
	
	*RCoefficient = CTCR[ GRIndex * 24 + GBIndex ];
	*BCoefficient = CTCB[ GRIndex * 24 + GBIndex ];
	
//	fprintf( test, "old RCoefficient = %d, old BCoefficient = %d\n", *RCoefficient, *BCoefficient );
	
}
