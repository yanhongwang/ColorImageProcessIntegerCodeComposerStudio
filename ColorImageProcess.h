#ifndef COLORIMAGEPROCESS

#define COLORIMAGEPROCESS

// 2 ^ 9, shift 9 bits is limitation
#define	ScaleBit		( int )8
#define	ScaleHalfBit	( int )( ScaleBit >> 1 )
#define ScaleNumber		( int )( 1 << ScaleBit )
#define MaxThreshold	65535	// 2 ^ ( `BitMapBit + `ScaleBit ) - 1 = 65535
#define	MinThreshold	( int )0

#endif
