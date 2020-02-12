/**************************************************************************
**
**        File:  fast_reader.c
**        Date:  Aug 02
**
** Description:  A collection of routines that read in, "parse", and
**               display data from an HPLogic fast data format file.
**
**
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

static char * programName = 0 ;

/* Used for filtering data */
int filtering = 0; /* Signifies whether we are doing filtering (0-no) */
long filter ;      /* Filter value */
int maxOutputSamples = 0 ;

const unsigned int bitsPerByte = 8; /* How many bits per byte */

static unsigned int walkingOnes32[32] ;

/* Various error messages */
#define ES_NOT_HPLOGIC_FILE     "NOT HPLogic fast binary data format file"
#define ES_TIME_CORR_SECTION    "Read error in TIME_CORR_INFO section"
#define ES_STATE_CORR_SECTION   "Read error in STATE_CORR_INFO section"

#define ID_FORMAT "HPLogic_Fast_Binary_Format_Data_File"
#define ID_STATE_CORR "StateCorrelationInfo"
#define ID_TIME_CORR "TimeCorrelationInfo"
#define ID_AUX_DATA_HEADER "HPLogic_Additional_Data_File"


/* structures used by the bitblock data types */
struct ReorderType {
  int endian16, endian32, endian64, endian128 ;
  int width ;
  int * input2outputMap32 ;
  int * output2inputMap32 ;
} ;

struct ExtractorType {
  unsigned int width ;
  unsigned int widthInBytes ;
  unsigned int inverted ;
  unsigned char * mask ;
  int reorderDirection ;
  struct ReorderType * reorder ;
} ;


/* global variables */
int GlobalSuppressDataOutput = 0 ;   /* used to resove recursive nature of some
                                        data structures */


/* forward declarations */
long readIntegralData( FILE *, int, int ) ;
void readDataSet( FILE *, int, int, int ) ;


void usage() 
{
/*
//  print the usage line(s) and exit
*/
    fprintf( stderr, "Usage:  %s [-l <infoType>] [-n <#samples>] [<file>]\n\n", programName ) ;
    fprintf( stderr, "        infoLevel <= 99 general data information\n" ) ;
    fprintf( stderr, "        infoLevel > 99  includes data\n" ) ;
    fprintf( stderr, "        -n #   limits the data to the first # samples\n" );
    fprintf( stderr, "        -f <value>   filter value\n" );
    exit( 1 ) ;
}


void doError( int l, const char *str )
{
/*
//  print an error message
*/
  if ( l > 0 )
      printf( "Error: %s\n", str ) ;
  exit( 1 ) ;
}


/*
**
**  A section of code that keeps track of shared objects and with
**  them a general pointer to its data.  The most common use is when
**  multiple labels use a common chunk of data.
**
*/

#define MAX_IDS 5000
static int numSharedObjects = -1 ;
static struct {
  long id ;
  char * buffer ;
  int numSamples ;
  int bytesPerSample ;
} sharedObjects[MAX_IDS+1] ;

int inSharedObjects( long id ) 
{
/*
//  return true if this object has been processed before. if not
//  add object id to the list seen and return false.
*/

    int i ;

    /* Iterate through the shared objects list looking for our id, return if
    // found
    */
    for ( i = 0 ; i <= numSharedObjects ; i++ )
        if ( sharedObjects[i].id == id )
            return 1 ;
    
    if ( numSharedObjects > MAX_IDS ) {
        doError( 1, "Exceeded the number of shared id's\n" ) ;
    }

    numSharedObjects++ ;
    sharedObjects[numSharedObjects].id = id ;
    sharedObjects[numSharedObjects].buffer = 0 ;
    sharedObjects[numSharedObjects].numSamples = 0 ;
    sharedObjects[numSharedObjects].bytesPerSample = 0 ;
    
    return 0 ;
}


char * getSharedObjectBuffer( long id, int *n, int *b )
{
/*
**  return the buffer pointer for the given sharedObject id.
*/
    int i ;


    for ( i = 0 ; i <= numSharedObjects ; i++ ) {
      if ( sharedObjects[i].id == id ) {
        *n = sharedObjects[i].numSamples ;
        *b = sharedObjects[i].bytesPerSample ;
        return  sharedObjects[i].buffer  ;
      }
    }

    return 0 ;
}


void setSharedObjectBuffer( long id, char * buf, int n, int b )
{
/*
**  set the buffer pointer for the given sharedObject id.
*/
    int i ;


    for ( i = 0 ; i <= numSharedObjects ; i++ ) {
      if ( sharedObjects[i].id == id ) {
        sharedObjects[i].buffer = buf ;
        sharedObjects[i].numSamples = n ;
        sharedObjects[i].bytesPerSample = b ;
        return ;
      }
    }
}


void cleanupSharedObjects()
{
/*
// this routine goes through the shared objects and "frees" up the
// buffer pointer for each object on the list.
*/
    int i ;

    for ( i = 0 ; i <= numSharedObjects ; i++ ) {
      if ( sharedObjects[i].buffer != 0 ) {
        free( sharedObjects[i].buffer ) ;
        sharedObjects[i].buffer = 0 ;
      }
    }
}

/* end of shared object utilities */


/*
** some general output routines 
*/

/* variables for communcation between outputPackeData and extractValue
   routines */
int ev_inverted, ev_byteOffset, ev_bytesPerLin, ev_bytesPerLine, ev_bytesUsed;
int ev_shiftByte5, ev_shiftByte4, ev_shiftByte3, ev_shiftByte2, ev_shiftByte1;
unsigned int ev_mask ;

unsigned int extractValue(char *buffer, int index )
{
/*
// Extract a value from a datablock
*/

  /* Initialize our value to 0 */
  unsigned int tmpValue = 0;

  /* Set up a pointer to the start of the data value (may extend across
  // multiple bytes.
  */
  unsigned char *bytePtr = (unsigned char *)
                           (&buffer[index*ev_bytesPerLine] + ev_byteOffset);

  switch (ev_bytesUsed)
  {
    /* We want these cases to fall through */
    /* Shift the values over according to their position */
    case 5: tmpValue |= *bytePtr++ << ev_shiftByte5;
    case 4: tmpValue |= *bytePtr++ << ev_shiftByte4;
    case 3: tmpValue |= *bytePtr++ << ev_shiftByte3;
    case 2: tmpValue |= *bytePtr++ << ev_shiftByte2;
    case 1: tmpValue |= *bytePtr   >> ev_shiftByte1;
            break;
  }

  /* If the value is is in negative logic, invert it */
  if (ev_inverted)
    tmpValue = ~tmpValue;

   tmpValue &= ev_mask ;

  return tmpValue;
}


void outputPackedData( long id, int width, int start, int inverted )
{
/*
//  print the data for a label with the given bit width and start
//  position from the given data buffer.  
//
*/

    int maxsize ;
    int bytesPerSample ;
    int numSamples ;
    char * buffer ;
    int count ;
    int column = 0 ;
    int maxCount ;

    /* Pre-calculation values for bit blocks */
    int lineSize, lsb, bitOffset;
    int bitsPerLine;

    if ( GlobalSuppressDataOutput ) 
      return ;

    /* get some buffer information */
    buffer = getSharedObjectBuffer( id, &numSamples, &bytesPerSample ) ;
    if ( !buffer ) return ;

    /* Set the number of columns to print */
    maxsize = 8;
    if (width > 16)
    {
      maxsize = 4;
    }

    ev_inverted = inverted ;
    ev_bytesPerLine = bytesPerSample ;
    lineSize = bytesPerSample ;
    ev_byteOffset = start/8 ;
    ev_bytesUsed = ((start + width -1) / 8) - (start/8) + 1 ;
    bitsPerLine = lineSize * bitsPerByte ;
    lsb = bitsPerLine - (start + width) ;
    bitOffset  = lsb % 8 ;

    ev_shiftByte5 = 32 - bitOffset ;
    ev_shiftByte4 = 24 - bitOffset ;
    ev_shiftByte3 = 16 - bitOffset ;
    ev_shiftByte2 = 8 - bitOffset ;
    ev_shiftByte1 = 0 + bitOffset ;

    ev_mask = (unsigned int)0xffffffff >> (32 - width) ;

    printf("\n0/%d: ", numSamples) ;

    if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
      maxCount =  maxOutputSamples ;
    } else {
      maxCount = numSamples ;
    }
    /* Iterate through each sample of the data set */ 
    for (count = 0 ; count < maxCount ; count++ )
    {
        unsigned int tmpValue = extractValue(buffer, count) ;
        if (!filtering || tmpValue != filter)
              printf("%#x ", tmpValue) ;
        else
              printf("FILTERED ") ;

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if ( column == maxsize && (count + 1) < maxCount )
        {
          printf("\n%d/%d: ", count + 1, numSamples) ;
          column = 0;
        }

    }
  
    printf("\n\n");
}

unsigned int reorderValue( unsigned int value, struct ReorderType *reorder )
{
/*
** reorder the given value based upon the reorder structure...
*/
  unsigned int retVal = 0 ;
  unsigned char tmp ;

  if ( reorder == 0 ) {
    return value ;
  }

  if ( reorder->endian16 ) {

    union Endian16Bit {
        unsigned short value ;
        unsigned char bytes[2] ;
    } endian16Bit ;

    /* Swap byte 0 and 1 */
    endian16Bit.value = value ;
    tmp = endian16Bit.bytes[0] ;
    endian16Bit.bytes[0] = endian16Bit.bytes[1] ;
    endian16Bit.bytes[1] = tmp ;

    retVal = endian16Bit.value ;

  } else if ( reorder->endian32 ) {

    union Endian32Bit {
        unsigned int value ;
        unsigned char bytes[4] ;
    } endian32Bit ;

    /* Swap bytes (0,3) and (1,2) */
    endian32Bit.value = value ;
    tmp = endian32Bit.bytes[0] ;
    endian32Bit.bytes[0] = endian32Bit.bytes[3] ;
    endian32Bit.bytes[3] = tmp ;
    tmp = endian32Bit.bytes[1] ;
    endian32Bit.bytes[1] = endian32Bit.bytes[2] ;
    endian32Bit.bytes[2] = tmp ;

    retVal = endian32Bit.value;

  } else {
    int i ;
    for ( i = 0 ; i < reorder->width ; i++ ) {
      if ( value & reorder->input2outputMap32[i] ) {
        retVal |= walkingOnes32[i] ;
      }
    }
  }

  return retVal ;
}


unsigned int extractBitBlockValue( unsigned char * bufPtr, 
                                   struct ExtractorType * extractor, 
                                   int index, 
                                   int bytesPerSample )
{
/*
//  Based upon the bit mask for this label, extract the correct data
//  from the data pointed to by bufPtr, using the desired sample number
//  indicated by index.
//
*/
    unsigned char *mP = extractor->mask ;
    unsigned int nC = extractor->width ;
    unsigned char maskVal ;
    unsigned char currentVal ;
    unsigned char mask ;
    unsigned int value ;
    int i, j ;

    value = 0 ;
    bufPtr += ( bytesPerSample * index ) ;
    for ( i = 0 ; i < bytesPerSample ; i++, mP++, bufPtr++ ) {
      if ( *mP ) {     /* are any mask bits for this byte  */
        maskVal = *mP ;
        currentVal = *bufPtr ;
        mask = 0x80 ;
        for ( j = 0 ; j < 8 ; j++, mask>>=1 ) {
          if ( mask & maskVal ) {
            value <<= 1 ;
            if ( mask & currentVal )
              value |= 1 ;
            if ( --nC == 0 ) break ; /* found the last bit, so return */
          }
        }
      }
    }

    if ( extractor->reorder ) {
      value = reorderValue( value, extractor->reorder ) ;
    }

    if ( extractor->inverted ) {
      unsigned int valMask ;
      valMask = (unsigned int) 0xffffffff ;
      valMask = valMask >> ( 32 - extractor->width ) ;
      value = ~value & valMask ;
    }

    return value ;
}


void outputBitBlockData( long id, struct ExtractorType * extractor )
{
    int maxsize ;
    int bytesPerSample ;
    int numSamples ;
    unsigned char * buffer ;
    int index ;
    int column = 0 ;
    unsigned int value ;
    int maxCount ;

    if ( GlobalSuppressDataOutput ) 
      return ;

    /* get some buffer information */
    buffer = (unsigned char*)getSharedObjectBuffer( id, 
                                           &numSamples, &bytesPerSample ) ;
    if ( !buffer ) return ;

    /* Set the number of columns to print */
    maxsize = 8 ;
    if ( extractor->width > 16 ) {
      maxsize = 4 ;
    }

    printf( "\n0/%d: ", numSamples ) ;

    if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
      maxCount =  maxOutputSamples ;
    } else {
      maxCount = numSamples ;
    }

    /* Iterate through each sample of the data set */ 
    for ( index = 0 ; index < maxCount ; index++ ) {
        value = extractBitBlockValue( buffer, extractor, index, bytesPerSample ) ;
        if ( !filtering || ( value != filter ))
              printf( "%#x ", value ) ;
        else
              printf( "FILTERED " ) ;

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if (( column == maxsize ) && ((index + 1) < maxCount )) {
          printf( "\n%d/%d: ", index+1, numSamples ) ;
          column = 0 ;
        }

    }
  
    printf( "\n\n" ) ;
}


void outputIntegralData( long id )
{
    int maxsize ;
    int bytesPerSample ;
    int numSamples ;
    char * buffer ;
    int count ;
    int column = 0 ;

    if ( GlobalSuppressDataOutput ) 
      return ;
     
    buffer = getSharedObjectBuffer( id, &numSamples, &bytesPerSample ) ;
    if ( !buffer ) return ;

    /* Set the number of columns to print */
    maxsize = 8 ;
    if ( bytesPerSample > 2 ) {
      maxsize = 4 ;
    }

    printf( "\n0/%d: ", numSamples ) ;

    if ( bytesPerSample == 1 ) {

      unsigned char value ;
      int maxCount ;
       
      if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
        maxCount =  maxOutputSamples ;
      } else {
        maxCount = numSamples ; 
      }
      for ( count = 0 ; count < maxCount ; count++ ) {
        value = (unsigned char)buffer[count] ;

        if ( !filtering || value != filter )
          printf( "%#x ", value ) ;
        else
          printf( "FILTERED " ) ;

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if ( column == maxsize && ( count + 1 ) < maxCount ) {
          printf("\n%d/%d: ", count + 1, numSamples ) ;
          column = 0 ;
        }
      }
    } else if ( bytesPerSample == 2 ) {

      unsigned short value ;
      unsigned short *buf = (unsigned short*)buffer ;
      int maxCount ;

      if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
        maxCount =  maxOutputSamples ;
      } else {
        maxCount = numSamples ; 
      }
      for ( count = 0 ; count < maxCount ; count++ ) {
        value = buf[count] ;

        if ( !filtering || value != filter )
          printf( "%#x ", value ) ;
        else
          printf( "FILTERED " ) ;

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if ( column == maxsize && ( count + 1 ) < maxCount ) {
          printf("\n%d/%d: ", count + 1, numSamples ) ;
          column = 0 ;
        }
      }
    } else if ( bytesPerSample == 4 ) {

      unsigned int value ;
      unsigned int *buf = (unsigned int*)buffer ;
      int maxCount ;

      if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
        maxCount =  maxOutputSamples ;
      } else {
        maxCount = numSamples ; 
      }
      for ( count = 0 ; count < maxCount ; count++ ) {
        value = buf[count] ;

        if ( !filtering || value != filter )
          printf( "%#x ", value ) ;
        else
          printf( "FILTERED " ) ;

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if ( column == maxsize && ( count + 1 ) < maxCount ) {
          printf("\n%d/%d: ", count + 1, numSamples ) ;
          column = 0 ;
        }
      }
    } else if ( bytesPerSample == 8 ) {


      /* We use long double here because we need 64 bit precision and
       * the long double gives us more than this for the mantissa portion.
       * Note that all time tags will be 64 bit values
       */
      long double x = 0.0 ;
      long double y = 4294967296.0 ; /* 2^32 */
      int *buf = (int*)buffer ;
      int subcount ;
      int maxCount ;

      if (( maxOutputSamples ) && ( maxOutputSamples < numSamples )) {
        maxCount =  maxOutputSamples ;
      } else {
        maxCount = numSamples ; 
      }
      for ( count = 0 ; count < maxCount ; count++ ) {
        /* We will pull 64 bit values in 32 bits at a time (2 moves) */
        subcount = count * 2;

        /* Copy upper 32 bits */
        x = buf[subcount++];
        /* Shift these up by 32 */
        x = x * y;

        /* Add the lower 32 bits in as unsigned */
        x += (unsigned int)buf[subcount];

        printf("%Lg ", x);

        /* Increase our column count and see if we need to add a return */
        column++ ;
        if ( column == maxsize && ( count + 1 ) < maxCount ) {
          printf("\n%d/%d: ", count + 1, numSamples ) ;
          column = 0 ;
        }
      }
    } else {
      printf( "Integral Data output of %d bytes/sample not implemented!\n" ) ;
    }

    printf("\n\n");
}


/* end of general output routines */


void readAttributes( FILE *fp, int infoLevel, char *who )
{
/*
//  Read the attributes bit set from a file
//  Formatted: "%d [%d]* \n"
//  See 'LabelEntry->Label Attribute Bitset' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (4.2)
*/
    int i ;
    int num ;
    int bitnum ;

    fscanf( fp, "%d", &num ) ;
    if ( infoLevel > 4 )
        printf( "  %s--Attributes: (%d) ", who, num ) ;

    for ( i = 0 ; i < num ; i++ ) {
        fscanf( fp, "%d", &bitnum ) ;
        if ( infoLevel > 9 )
            printf( "%d ", bitnum ) ;
    }

    if ( infoLevel >4 )
        printf( "\n" ) ;

    fscanf( fp, "\n" ) ;
}


void readStateCorrelationInfo( FILE *fp, int infoLevel )
{
/*
//  Read state correlation information from a file
//  See 'Vertical Header->State Correlation Info' section of online help for 
//  HPLogic Fast Data File Format under the File Out tool (10.0)
*/
    char line[BUFSIZ] ;

    fgets( line, sizeof( line ), fp ) ;
    if ( strncmp( line, ID_STATE_CORR, strlen( ID_STATE_CORR ) ) != 0 )
        doError( infoLevel, ES_STATE_CORR_SECTION ) ;

    /* state correlation offset */
    fgets( line, sizeof( line ), fp ) ;
    if ( infoLevel > 4 )
        printf( "  State Correlation Offset: %s\n", line ) ;
}


void readTimeCorrelationInfo( FILE *fp, int infoLevel )
{
/*
//  Read time correlation information from a file
//  See 'Vertical Header->Time Correlation Info' section of online help for 
//  HPLogic Fast Binary Data File Format under the File Out tool (9.0)
*/
    char line[BUFSIZ] ;
    int type ;
    int source ;

    fscanf( fp, "%s\n", line ) ;
    if ( strcmp( line, ID_TIME_CORR ) != 0 )
        doError( infoLevel, ES_TIME_CORR_SECTION ) ;

    /* time correlation type, source, and offset */
    fscanf( fp, "%d %d %s\n", &type, &source, line ) ;
    if ( infoLevel > 4 ) {
        printf( "  Time Correlation type: %d  source:%d  Offset: %s\n", type, 
                                                             source, line ) ;
    }
}

long readIntegralArray( FILE *fp, int infoLevel, int numbits, int sign, 
                                                                    int ipl )
{
/*
//  Read an Integral Array from a file
//  See 'IntegralData->IntegralArray' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (6.1)
//
//
*/
    long id ;
    int len ;
    
    char tmpstr[BUFSIZ] ;
    char *buffer ;
    int count, maxsize;
    int column = 0;
    int size = numbits / 8 ;

    if ( infoLevel > 4 ) {
        printf( "      IntegralArray with %d bits and sign %d\n", numbits, 
                                                                     sign ) ;
    }

    /* integral data id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      Integral ID: %d", id ) ;

    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 )
            printf( "    already seen this integral object\n" ) ;
    }  else {
      if ( infoLevel > 4 )
        printf( "\n" ) ;

      /* length */
      fscanf( fp, "%d\n", &len ) ;
      if ( infoLevel > 4 )
        printf( "        Length: %d  Size:%d\n", len, size*len ) ;

      /* stuckone and stuckzero */
      fgets( tmpstr, sizeof( tmpstr ), fp ) ;
      /* sscanf( tmpstr, "%s %s\n", x, y ) ; */

      /* raw bytes */
      buffer = (char*)malloc( size * len ) ;

      fread( buffer, size, len, fp ) ;

      len = len / ipl ;    /* adjust len/size to account for integralsPerLine */
      size = size * ipl ;  /* used by BitPackedData objects...                */

      setSharedObjectBuffer( id, buffer, len, size ) ;
    }
    
    if ( infoLevel > 99 ) {
        outputIntegralData( id ) ;
    }
     
    return id ;
}

void readAuxDataFileHeader( FILE *fp, int infoLevel ) 
{
/*
//  Read past the file comment section and data file header:
//      "HP16505A File Comment Start     "
//      "          ( zero or more        "
//      "            text lines )        "
//      "HP16505A File Comment End       "
//      "HPLogic_Additional_Data_File    "
//
//  See 'IntegralData->PagedBitBlock' or 'IntegralData->PagedIntegralData'
//  sections of online help for HPLogic Fast Binary Data File Format under 
//  the File Out tool (6.5.2 or 6.6.3)
*/

    char line[BUFSIZ] ;

    while ( fgets( line, sizeof( line ), fp ) != NULL ) {
      if ( strncmp( line, ID_AUX_DATA_HEADER, strlen( ID_AUX_DATA_HEADER ) ) ==
                                                                          0 ) 
        return ;
    }
}


long readPagedIntegralData( FILE *fp, int infoLevel, int numbits, int sign )
{
/*
//  Read Paged Integral Data from a file
//  See 'IntegralData->PagedIntegralData' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (6.6)
*/
    long id ;
    int useFile ;
    char filename[BUFSIZ] ;
    FILE *dataFP ;
    char tmpstr[BUFSIZ] ;
    int tmp ;
    char *buffer ;
    unsigned int numSamples, numBytesPerSample ;

    if ( infoLevel > 4 ) {
        printf( "      PagedIntegralData with %d bits and sign %d\n", numbits, 
                                                                      sign ) ;
    }

    /* integral data id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      Integral ID: %d\n", id ) ;

    /* id == 0 means that there is no data - reason unknown!!! */
    if ( id == 0 ) {
      if ( infoLevel > 4 )
          printf( "  Data missing!\n" ) ;
      return id ; 
    }
    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 ) {
            printf( "    already seen this integral object\n" ) ;
        }
        return id ;
    }

    /* is this data in this file or in separate file? */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    sscanf( tmpstr, "%d `%[^`]`", &useFile, filename ) ;

    if ( useFile ) {
      if ( infoLevel > 4 )
        printf( "      Data is in file `%s`.\n", filename ) ;
        if (( dataFP = fopen( filename, "r" )) == NULL ) {
          /* full name did not work, try just base name */
          char *basename = strrchr( filename, '/' ) ;
          if (( !basename ) || (( dataFP = fopen( basename+1, "r" )) == NULL )) {
              printf( "   Can't open file `%s`.\n" ) ;
              return id ;
          }
        }
        readAuxDataFileHeader( dataFP, infoLevel ) ;
    } else {
      dataFP = fp ;
    }

    /* next two ints are reserved for paging information... */
    fscanf( dataFP,  "%d %d\n", &tmp, &tmp ) ;

    fscanf( dataFP, "%d %d\n", &numSamples, &numBytesPerSample ) ;

    /* raw bytes */
    buffer = (char*)malloc( numSamples * numBytesPerSample ) ;

    fread( buffer, numBytesPerSample, numSamples, dataFP ) ;
    
    setSharedObjectBuffer( id, buffer, numSamples, numBytesPerSample ) ;

    if ( infoLevel > 99 ) {
      outputIntegralData( id ) ;
    }

    if ( useFile ) {
      fclose( dataFP ) ;
    }

    return id ;
}


void readStringBlock( FILE * fp, int infoLevel, int length )
{
/*
//  Read a string block from a file
//  See 'IntegralData->Strings->StringBlock' section of online help for 
//  HPLogic Fast Binary Data File Format under the File Out tool (6.7.2)
*/
    unsigned int numChars ;
    int i;
    char *buffer ;

    for (i = 0; i < length; i++)
    {
      fread( &numChars, sizeof(numChars), 1, fp );

      /* raw bytes */
      buffer = (char*)malloc( sizeof( char ) * numChars ) ;
      fread( buffer, sizeof( char ), numChars, fp ) ;

      free( buffer ) ;
    }
}


long readBitPackedData( FILE *fp, int infoLevel )
{
/*
//  Read bit packed data from a file
//  See 'IntegralData->BitPackedData' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (6.2)
*/
    
    long id, id2 ;
    char tmpstr[BUFSIZ] ;
    int bytesPerLine, integralTypesPerLine ;
    int start, width, inverted ;
    char *buffer ;
    int numSamples, bytesPerSample ;

    if ( infoLevel > 4 )
        printf( "      BitPackedData:\n" ) ;

    /* integral data id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      Integral ID: %d\n", id ) ;

    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 )
            printf( "    already seen this integral object\n" ) ;
        return id ;
    }

    fscanf( fp, "%d %d %d\n", &start, &width, &inverted ) ;

    if ( infoLevel > 4 )
        printf( "      start=%d   width=%d    inverted=%d\n", start, width, inverted ) ;

    /* stuckone and stuckzero */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    /* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */

    /* datablock */
    fscanf( fp, "%d %d\n", &bytesPerLine, &integralTypesPerLine ) ;
    if ( infoLevel > 4 )
        printf( "      bytesPerLine=%d   integralTypesPerLine=%d\n", bytesPerLine, integralTypesPerLine ) ;

    GlobalSuppressDataOutput = 1 ;
    id2 = readIntegralData( fp, infoLevel, integralTypesPerLine ) ;
    GlobalSuppressDataOutput = 0 ;

    fscanf( fp, "\n" ) ;

    if ( infoLevel > 99 ) {
      /* output the data for this label */
      outputPackedData( id2, width, start, inverted ) ;
    }

    return id ;
}


void readReorderObject( FILE *fp, int infoLevel, struct ReorderType *r )
{
/*
//  Read label bit reorder information from the file
//  See 'IntegralData->Label Extractor' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (6.8.4)
*/
    int endian16, endian32, endian64, endian128, width ;
    char buffer[256] ;
    /* longest input2outputMapL = malloc( width * sizeof( longest )) ; */
    /* longest output2inputMapL = malloc( width * sizeof( longest )) ; */
    /* longest is a type that deals with numbers greater than 32 bits  */
    int i ;

    fscanf( fp, "Endian16: %d Endian32: %d Endian64: %d Endian128: %d Width: %d\n",
                       &endian16, &endian32, &endian64, &endian128, &width ) ;

    if ( infoLevel > 4 ) {
      printf( "      Bit reordering: Endian16:%d  32:%d  64:%d  128:%d\n",
                         endian16, endian32, endian64, endian128 ) ;
    }

    r->endian16 = endian16 ;
    r->endian32 = endian32 ;
    r->endian64 = endian64 ;
    r->endian128 = endian128 ;
    r->width = width ;
    r->input2outputMap32 = 0;
    r->output2inputMap32 = 0;

    if ( endian16 || endian32 || endian64 || endian128 ) {
      return ;
    }

    /* not one the "standard" endian orderings, so a custom one... */

    if ( width <= 32 ) {

      r->input2outputMap32 = (int*)malloc( width * sizeof( int ) ) ;
      r->output2inputMap32 =  (int*)malloc( width * sizeof( int ) ) ;

      if ( infoLevel > 9 ) {
        printf( "        Custom Reordering:\n" ) ;
      }

      for ( i = 0 ; i < width ; i++ ) {
        fscanf( fp, "%d %d\n", &(r->input2outputMap32[i]),
                                                &(r->output2inputMap32[i]) ) ;
        if ( infoLevel > 9 ) {
          printf( "                           %-#10.*x  %-#10.*x\n", 
                                        width/8, r->input2outputMap32[i],
                                        width/8, r->output2inputMap32[i]  ) ;
        }
      }

      return ;
    }

    
    /* custom, but greater than 32 bits wide ... */

    for ( i = 0 ; i < width ; i++ ) {
        fscanf( fp, "%s", buffer ) ;
        /* input2outputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
        fscanf( fp, "%s\n", buffer ) ;
        /* output2inputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
     }

}


void readExtractorInfo( FILE *fp, int infoLevel, struct ExtractorType *e )
{
/*
//  Read an Extractor object from the given file.
//  See 'IntegralData->Label Extractor' section fo the online help
//  for Fast Binary Data File Format under the File Out tool (6.8)
*/
  
    int tmp ;
    int haveReorderObject ;
    int i ;
    int mask ;


    /* get the width in bit and bytes of the mask and whether it needs to
       be inverted or not */
    fscanf( fp, "%u %u %d ", &e->widthInBytes, &e->width, &e->inverted ) ;

    if ( infoLevel > 4 )
        printf( "      width=%d  bytes=%d  inverted=%d\n",  e->width,
                                              e->widthInBytes, e->inverted ) ;

    /* get the label mask */
    e->mask = (unsigned char *)malloc( e->widthInBytes ) ;
    if ( infoLevel > 4 ) {
        printf( "      mask:" ) ;
    }
    for ( i = 0 ; i < e->widthInBytes ; i++ ) {
      fscanf( fp, "%x", &mask ) ;
      e->mask[i] = mask ; 
      if ( infoLevel > 4 ) {
        printf( " 0x%x", mask ) ;
      }
    }

    if ( infoLevel > 4 ) {
      printf( "\n" ) ;
    }

    /* a reserved field for future use */
    fscanf( fp, "%d\n", &tmp ) ;

    /* get the reorder direction - may or may not actually be used */
    fscanf( fp, "%d\n", &e->reorderDirection ) ;

    /* get the reorder structure flag */
    fscanf( fp, "%d\n", &haveReorderObject ) ;

    if ( haveReorderObject ) {
      e->reorder = (struct ReorderType *)malloc( sizeof( struct ReorderType )) ;
      readReorderObject( fp, infoLevel, e->reorder ) ;
    } else {
      e->reorder = 0 ;   /* make sure pointer is cleared */
      fscanf( fp, "\n" ) ;
    }
}


long readBitBlock( FILE *fp, int infoLevel )
{
/*
//  Read a BitBlock object from the given file.
//  See 'IntegralData->BitBlock' or 'IntegralData->PagedBitBlock' sections 
//  of the online help for Fast Binary Data File Format under the File Out 
//  tool (6.4 or 6.5)
*/
    char tmpstr[BUFSIZ] ;
    int paged = 0 ;
    int tmp ;
    long id ;
    int useFile ;
    char filename[BUFSIZ] ;
    int numSamples, numBytesPerSample ;
    char * buffer ;
    FILE *dataFP ;

    fscanf( fp, "%s\n", tmpstr ) ;

    if ( strcmp( tmpstr, "BitBlock" ) == 0 ) {
        paged = 0 ;
    } else if  ( strcmp( tmpstr, "PagedBitBlock" ) == 0 ) {
        paged = 1 ;
    } else {
        char errStr[BUFSIZ] ;
        sprintf( errStr, "Unknown BitBlock object (%s)", tmpstr ) ;
        doError( infoLevel, errStr ) ;
    }

    /* first two ints reserved */
    fscanf( fp, "%d\n%d\n", &tmp, &tmp ) ;

    /* BitBlock id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      BitBlock ID: %d", id ) ;

    /* id == 0 means that there is no data - reason unknown!!! */
    if ( id == 0 ) {
      if ( infoLevel > 4 )
          printf( "  Data missing!\n" ) ;
      return id ; 
    }

    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 )
            printf( "  already seen this integral object\n" ) ;
        return id ;
    }
    if ( infoLevel > 4 )
        printf( "\n" ) ;


    /* is this data in this file or in separate file? */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    sscanf( tmpstr, "%d `%[^`]`", &useFile, filename ) ;

    if ( useFile ) {
      if ( infoLevel > 4 )
        printf( "      Data is in file `%s`.\n", filename ) ;
        if (( dataFP = fopen( filename, "r" )) == NULL ) {
          /* full name did not work, try just base name */
          char *basename = strrchr( filename, '/' ) ;
          if (( !basename ) || (( dataFP = fopen( basename+1, "r" )) == NULL )) {
              printf( "   Can't open file `%s`.\n" ) ;
              return id ;
          }
        }
        readAuxDataFileHeader( dataFP, infoLevel ) ;
    } else {
      dataFP = fp ;
    }

    if ( paged ) {
      /* next two ints are reserved for paging information... */
      fscanf( dataFP,  "%d %d\n", &tmp, &tmp ) ;
    }

    fscanf( dataFP, "%d %d\n", &numSamples, &numBytesPerSample ) ;
    if ( infoLevel > 4 )
        printf( "        Number samples: %d  Bytes Per Sample: %d\n", numSamples, numBytesPerSample ) ;

    /* raw bytes */
    buffer = (char*)malloc( numSamples * numBytesPerSample ) ;

    fread( buffer, numBytesPerSample, numSamples, dataFP ) ;

    fscanf( fp, "\n" ) ;
    
    setSharedObjectBuffer( id, buffer, numSamples, numBytesPerSample ) ;

    if ( useFile ) {
      fclose( dataFP ) ;
    }

    return id ;
}


void readBitBlockData( FILE *fp, int infoLevel )
{
/*
//  Read bitblock data object from this file
//  See 'IntegralData->BitBlockData' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (6.3)
*/
    
    long id ;
    char tmpstr[BUFSIZ] ;
    struct ExtractorType extractor ;

    if ( infoLevel > 4 )
        printf( "      BitBlockData:\n" ) ;

    /* integral data id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      Integral ID: %d  ", id ) ;

    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 )
            printf( "    already seen this integral object\n" ) ;
        return ;
    } else {
        if ( infoLevel > 4 )
            printf( "\n" ) ;
    }

    /* extractor information for this label */
    readExtractorInfo( fp, infoLevel, &extractor ) ;

    /* stuckone and stuckzero */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    /* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */

    /* bitblock */
    id = readBitBlock( fp, infoLevel ) ;

    if ( infoLevel > 99 ) {
      outputBitBlockData( id, &extractor ) ;
    }

    /* some memory cleanup... */
    free( extractor.mask ) ;
    if ( extractor.reorder )
        free( extractor.reorder ) ;
}


void readStringData( FILE *fp, int infoLevel )
{
/*
//  Read string data from a file
//  See 'IntegralData->Strings' section of online help for HPLogic Fast 
//  Binary Data File Format under the File Out tool (6.7)
*/
    
    long id ;
    char tmpstr[BUFSIZ] ;
    int length ;

    if ( infoLevel > 4 )
        printf( "      StringData:\n" ) ;

    /* integral data id */
    fscanf( fp, "%d\n", &id ) ;

    if ( infoLevel > 4 )
        printf( "      Integral ID: %d\n", id ) ;

    if ( inSharedObjects( id ) ) {
        if ( infoLevel > 4 )
            printf( "    already seen this integral object\n" ) ;
        return ;
    }

    fscanf( fp, "%d\n", &length) ;

    if ( infoLevel > 4 )
        printf( "      length=%d\n", length ) ;

    /* stringblock */
    readStringBlock( fp, infoLevel, length ) ;
}


long readIntegralData( FILE *fp, int infoLevel, int ipl ) 
{
/*
//  Read integral data from a file
//  See 'IntegralData' section of online help for HPLogic Fast Binary Data 
//  File Format under the File Out tool (6.0)
*/
    char tmpstr[BUFSIZ] ;
    int numbits ;
    long id = 0 ;

    fscanf( fp, "%s\n", tmpstr ) ;

    if ( strcmp( tmpstr, "BitPackedData" ) == 0 ) {
        id = readBitPackedData( fp, infoLevel ) ;
    } else if  ( strcmp( tmpstr, "StringData" ) == 0 ) {
        readStringData( fp, infoLevel );
    } else if  ( strcmp( tmpstr, "BitBlockData" ) == 0 ) {
        readBitBlockData( fp, infoLevel ) ;
    } else if  ( strncmp( tmpstr, "IntegralArray<", 14 ) == 0 ) {
        if ( sscanf( &tmpstr[14], "signed%d", &numbits ) == 1 ) {
            id = readIntegralArray( fp, infoLevel, numbits, 1,ipl ) ;
        } else  if ( sscanf( &tmpstr[14], "unsigned%d", &numbits ) == 1 ) {
            id = readIntegralArray( fp, infoLevel, numbits, -1, ipl ) ;
        } else {
            doError( infoLevel, "Invalid IntegralArray object" ) ;
        }
    } else if  ( strncmp( tmpstr, "PagedIntegralData<", 18 ) == 0 ) {
        if ( sscanf( &tmpstr[18], "signed%d", &numbits ) == 1 ) {
            id = readPagedIntegralData( fp, infoLevel, numbits, 1 ) ;
        } else  if ( sscanf( &tmpstr[18], "unsigned%d", &numbits ) == 1 ) {
            id = readPagedIntegralData( fp, infoLevel, numbits, -1 ) ;
        } else {
            doError( infoLevel, "Invalid PagedIntegralData object" ) ;
        }
    } else {
        char errStr[BUFSIZ] ;
        sprintf( errStr, "Unknown IntegralData object (%s)", tmpstr ) ;
        doError( infoLevel, errStr ) ;
    }

    return id ;
}

void readScopeHeader( FILE *fp, int infoLevel )
{
/*
//  Read a scope header from a file
//  See 'Vertical Header->ScopeHeader' section of online help for HPLogic 
//  Fast Binary Data File Format under the File Out tool (7.2)
*/
      float YIncrement, YOrigin;
      int yReference, numbits;
 
      fread( &YIncrement, sizeof( float ), 1, fp ) ;
      fread( &YOrigin, sizeof( float ), 1, fp ) ;

      if ( infoLevel > 4 )
      {
        printf( "      YIncrement=%f\n", YIncrement ) ;
        printf( "      YOrigin=%f\n", YOrigin ) ;
      }

      fscanf( fp, "%d %d\n", &yReference, &numbits) ;

      if ( infoLevel > 4 )
      {
        printf( "      yReference=%d\n", yReference ) ;
        printf( "      numbits=%d\n", numbits ) ;
      }
}

void readDefaultOrdinateHeader( FILE *fp, int infoLevel )
{
/*
//  Read a default ordinate header from a file (currently empty)
//  See 'Vertical Header->DefaultOrdinateHeader' section of online help for 
//  HPLogic Fast Binary Data File Format under the File Out tool (7.1)
*/
  return;
}

void readVerticalHeader( FILE *fp, int infoLevel )
{
/*
//  Read a veritcal header from a file
//  See 'Vertical Header' section of online help for HPLogic Fast Binary 
//  Data File Format under the File Out tool (7.0)
*/
    char tmpstr[BUFSIZ] ;

    fscanf( fp, "%s\n", tmpstr ) ;
    if ( strcmp( tmpstr, "DefaultOrdinateHeader" ) == 0 ) {
            readDefaultOrdinateHeader( fp, infoLevel ) ;
    } else if ( strcmp( tmpstr, "ScopeHeader" ) == 0 ) {
            readScopeHeader( fp, infoLevel ) ;
    } else
    {
        doError( infoLevel, "Unknown VerticalHeader object" ) ;
    }
}


void readLabelData( FILE *fp, int infoLevel )
{
/*
//  Read label data from a file
//  See 'LabelData' section of online help for HPLogic Fast Binary Data 
//  File Format under the File Out tool (5.0)
*/
    char tmpstr[BUFSIZ] ;

    fscanf( fp, "%s\n", tmpstr ) ;

    if ( strcmp( tmpstr, "NoData" ) == 0 ) {
        readAttributes( fp, infoLevel, "  NoData" ) ;
    } else if  ( strcmp( tmpstr, "States" ) == 0 ) {
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readAttributes( fp, infoLevel, "  States" ) ;
    } else if  ( strcmp( tmpstr, "StateCount" ) == 0 ) {
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readAttributes( fp, infoLevel, "  StateCount" ) ;
    } else if  ( strcmp( tmpstr, "Glitch" ) == 0 ) {
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readAttributes( fp, infoLevel, "  Glitch" ) ;
    } else if  ( strcmp( tmpstr, "Analog" ) == 0 ) {
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readVerticalHeader( fp, infoLevel ) ;
        fscanf( fp, "%s\n", tmpstr ) ;
        readAttributes( fp, infoLevel, "  Analog" ) ;
    } else if  ( strcmp( tmpstr, "TextLines" ) == 0) {
        readIntegralData( fp, infoLevel, 1 ) ;
        fscanf( fp, "\n" ) ;
        readAttributes( fp, infoLevel, "  StateCount" ) ;
    } else {
        char errstr[BUFSIZ] ;
        sprintf( errstr, "Unknown LabelData object (%s).", tmpstr ) ;
        doError( infoLevel, errstr ) ;
    }

}


void readAbscissaData( FILE * fp, int infoLevel )
{
/*
//  Read abscissa data from a file
//  See 'Vertical Header->Abscissa Data Type->AbscissaData' section of 
//  online help for HPLogic Fast Binary Data File Format under the File Out
//  tool (8.1)
*/
    int samples, trig ;

    if ( infoLevel > 1 )
        printf( "  Abscissa Data\n" ) ;

    /* number of samples and trigger position */
    fscanf( fp, "%d %d\n", &samples, &trig ) ;

    if ( infoLevel > 4 )
        printf( "    samples = %d   trigger = %d\n", samples, trig ) ;

}


void readPeriodic( FILE * fp, int infoLevel )
{
/*
//  Read periodic information from a file
//  See 'Vertical Header->Abscissa Data Type->Perodic' section of online 
//  help for HPLogic Fast Binary Data File Format under the File Out tool (8.2)
*/
    int samples, trig ;
    char tmpstr[BUFSIZ] ;

    if ( infoLevel > 1 )
        printf( "  Periodic Data\n" ) ;

    /* number of samples and trigger position */
    fscanf( fp, "%d %d\n", &samples, &trig ) ;

    if ( infoLevel > 4 )
        printf( "    samples = %d   trigger = %d\n", samples, trig ) ;

    /* Time of first sample sample */
    fscanf( fp, "%s ", tmpstr ) ;
    if ( infoLevel > 4 ) 
        printf( "    origin = %s ps", tmpstr ) ;

    /* Time between samples */
    fscanf( fp, "%s\n", tmpstr ) ;
    if ( infoLevel > 4 ) 
        printf( "   increment = %s ps\n", tmpstr ) ;

    readAttributes( fp, infoLevel, "Periodic" ) ;

}


void readPeriodic2( FILE * fp, int infoLevel )
{
/*
//  Read periodic2 information from a file
//  See 'Vertical Header->Abscissa Data Type->Perodic2' section of online 
//  help for HPLogic Fast Binary Data File Format under the File Out tool
*/
    int numerator, denominator ;
    char tmpstr[BUFSIZ] ;

    if ( infoLevel > 1 )
        printf( "  Periodic2 Data\n" ) ;

    readPeriodic( fp, infoLevel );

    /* numerator and denominator */
    fscanf( fp, "%d %d\n", &numerator, &denominator ) ;

    if ( infoLevel > 4 )
        printf( "    numerator = %d   denominator = %d\n", numerator, denominator ) ;

    /* end of line */
    fscanf( fp, "\n");
}


void readTimeTags( FILE * fp, int infoLevel, int pagedFlag )
{
/*
//  Read time tag information from a file
//  See 'Vertical Header->Abscissa Data Type->Time Tags' section of online 
//  help for HPLogic Fast Binary Data File Format under the File Out tool (8.3)
*/
    int samples, trig ;
    fpos_t pos ;
    int haveData = 1 ;

    fgetpos( fp, &pos ) ;
    if ( infoLevel > 1 )
        printf( "  Time Tagged Data    (pos=%d)\n", pos ) ;

    /* number of samples and trigger position */
    fscanf( fp, "%d %d\n", &samples, &trig ) ;

    if ( infoLevel > 4 )
        printf( "    samples = %d   trigger = %d\n", samples, trig ) ;

    if ( pagedFlag ) {
      fscanf( fp, "%d\n", &haveData ) ;
    }

    if ( haveData ) {
      readIntegralData( fp, infoLevel, 1 ) ;
      fscanf( fp, "\n" ) ;
    }

    readAttributes( fp, infoLevel, "TimeTags" ) ;
}


void readXaxis( FILE *fp, int infoLevel )
{
/*
//  Read X axis information from a file
//  See section 'Vertical Header->Abscissa Data Type' of online help for 
//  HPLogic Fast Binary Data File Format under the File Out tool (8.0)
*/
    char tmpstr[BUFSIZ] ;

    fscanf( fp, "%s\n", tmpstr ) ;

    if ( strcmp( tmpstr, "AbscissaData" ) == 0 ) {
        readAbscissaData( fp, infoLevel ) ;
    } else if  ( strcmp( tmpstr, "Periodic" ) == 0 ) {
        readPeriodic( fp, infoLevel ) ;
    } else if  ( strcmp( tmpstr, "Periodic2" ) == 0 ) {
        readPeriodic2( fp, infoLevel ) ;
    } else if  ( strcmp( tmpstr, "TimeTags" ) == 0 ) {
        readTimeTags( fp, infoLevel, 0 ) ;
    } else if  ( strcmp( tmpstr, "PagedTimeTags" ) == 0 ) {
        readTimeTags( fp, infoLevel, 1 ) ;
    } else {
        char errStr[BUFSIZ] ;
        sprintf( errStr, "Unknown Abscissa Data Type (%s)", tmpstr ) ;
        doError( infoLevel, errStr ) ;
    }
}


void readLabelEntry( FILE *fp, int infoLevel, int Fmajor, int Fminor )
{
/*
//  Read a label entry from a file
//  See 'Label Entry' section of online help for HPLogic Fast Binary Data 
//  File Format under the File Out tool (4.0)
*/
    long labelId ;
    char labelName[BUFSIZ] ;
    char tmpstr[BUFSIZ] ;
    fpos_t pos ;

    fgetpos( fp, &pos ) ;
    fscanf( fp, "%d\n", &labelId ) ;

    if ( infoLevel > 4 )
        printf( "  Label ID: %d    (pos=%d)\n", labelId, pos ) ;

    if ( inSharedObjects( labelId ) ) {
        if ( infoLevel > 4 )
            printf( "    already seen this LabelEntry object\n" ) ;
        return ;
    }

    /* label name */
    fscanf( fp, "`%[^`]`\n", labelName ) ;
    if ( infoLevel > 4 )
        printf( "    Label Name: '%s'\n", labelName ) ;

    /* label data */
    readLabelData( fp, infoLevel ) ;

    fscanf( fp, "\n" ) ;

    /* label attributes */
    readAttributes( fp, infoLevel, "  Label" ) ;

}


void readDataSet( FILE *fp, int infoLevel, int Fmajor, int Fminor )
{
/*
//  read the dataset section from the given file.
//  See 'DataSet' section of online help for HPLogic Fast Binary Data 
//  File Format under the File Out tool (3.0)
*/
    int i ;
    int numLabelEntries ;
    char tmpstr[BUFSIZ] ;
    int id1, id3 ;
    int beginTime, endTime ;
    int startSample, lastSample ;
    fpos_t pos ;

    /* number of label entries */
        fgetpos( fp, &pos ) ;
    fscanf( fp, "%d\n", &numLabelEntries ) ;
    if ( infoLevel > 1 )
        printf( "  Number of LabelEntries: %d     (pos=%d)\n", 
                                                    numLabelEntries, pos ) ;
    for( i = 0 ; i < numLabelEntries ; i++ ) {
        printf( "  Label %3d  -- ", i+1 ) ;
        readLabelEntry( fp, infoLevel, Fmajor, Fminor ) ;
    }
    
    /* x-axis data */
    readXaxis( fp, infoLevel ) ;

    /* time correlation info */
    readTimeCorrelationInfo( fp, infoLevel ) ;

    /* state correlation info */
    readStateCorrelationInfo( fp, infoLevel ) ;

    /* origin path */
    fscanf( fp, "`%[^`]`\n", tmpstr ) ;
    if ( infoLevel > 4 )
        printf( "  Origin: '%s'\n", tmpstr ) ;

    /* dataset id, etc... */
    fscanf( fp, "%d %d\n", &id1, &id3 ) ;
    if ( infoLevel > 9 ) {
        printf( "  DataSet ID: %d\n", id1 ) ;
        printf( "  Run ID    : %d\n", id3 ) ;
    }

    /* Begin and End time */
    fscanf( fp, "%d\n", &beginTime); ; ;
    fscanf( fp, "%d\n", &endTime) ; ;
    if ( infoLevel > 4 ) {
        printf( "  Begin Time: %d\n", beginTime ) ;
        printf( "  End   Time: %d\n", endTime ) ;
    }

    /* start sample */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    startSample = atoi( tmpstr ) ;
    /* last sample */
    fgets( tmpstr, sizeof( tmpstr ), fp ) ;
    lastSample = atoi( tmpstr ) ;

}


void readDataGroup( FILE *fp, int infoLevel )
{
/*
//  read the dategroup section from the given file.
//  See 'DataGroup' section of online help for HPLogic Fast Binary Data 
//  File Format under the File Out tool (2.0)
*/
    char line[BUFSIZ] ;
    int Fmajor, Fminor ;
    int num, bit ;
    int numDataSets ;
    int timeCrossCorrID, stateCrossCorrID ;
    int i ;
        fpos_t pos ;

        fgetpos( fp, &pos ) ;

    /* get first line */
    fgets( line, sizeof( line ), fp ) ;

    if ( strncmp( line, ID_FORMAT, strlen( ID_FORMAT) ) != 0 )
        doError( infoLevel, ES_NOT_HPLOGIC_FILE ) ;
    fgets( line, sizeof( line ), fp ) ;
    sscanf( line, "%d %d", &Fmajor, &Fminor ) ;
    if ( infoLevel > 0 )
        printf( "Version: %d.%d\n", Fmajor, Fminor ) ;

    /* the correlation bits */
    if ( infoLevel > 9 )
        printf( "Correlation Bits: " ) ;
    fscanf( fp, "%d", &num ) ;
    for ( i = 0 ; i < num ; i++ ) {
      fscanf( fp, "%d", &bit ) ;
      if ( infoLevel > 9 )
        switch ( bit ) {
          case 0:
            printf( "Time " ) ;
            break ;
          case 1:
            printf( "State " ) ;
            break ;
          default:
            printf( "unknown(%d) \n", bit ) ;
            break ;
        }
    }
    if ( infoLevel > 9 )
        printf( "\n" ) ;
    fscanf( fp, "\n" ) ;  /* pass by the newline */

    /* the cross correlation id's */
    fscanf( fp, "%d %d\n", &timeCrossCorrID, &stateCrossCorrID ) ;
    if ( infoLevel > 9 ) {
        printf( "    Time cross correlation id = %d\n", timeCrossCorrID ) ;
        printf( "    State cross correlation id = %d\n", stateCrossCorrID ) ;
    }

    fscanf( fp, "%d\n", &numDataSets ) ;
    if ( infoLevel > 0 )
        printf( "DataSets: %d\n", numDataSets ) ;

    for ( i = 0 ; i < numDataSets ; i++ ) {
        if ( infoLevel > 1 )
            printf( "DataSet #%d\n", i+1 ) ;
        readDataSet( fp, infoLevel, Fmajor, Fminor ) ;
    }
}


#ifdef _WIN32
int intArg( int argc, char **argv, int *curArg )
{
/*
//  Get an integer arg from the command line
*/
    int i = *curArg ;
    int retVal ;

    if ( argv[i][2] == 0 ) {
        i++ ;
        if ( i >= argc ) {
            return 0 ;
        }
        retVal = atoi( argv[i] ) ;
        *curArg = i ;
    } else {
      retVal = atoi( &argv[i][2] ) ;
    }
    return retVal ;
}

char* strArg( int argc, char **argv, int *curArg )
{
/*
//  Get a string arg from the command line
*/
    int i = *curArg ;
    char* retVal ;

    if ( argv[i][2] == 0 ) {
        i++ ;
        if ( i >= argc ) {
            return NULL ;
        }
        retVal = argv[i] ;
        *curArg = i ;
    } else {
        retVal = &argv[i][2] ;
    }
    return retVal ;
}

#endif


main( int argc, char **argv )
{
/*
// main function for this sample HPLogic fast binary file reader
*/

    FILE *fp = 0 ;
    int result = 0 ;
    int infoLevel = 99 ;
    char *p ;
    int i ;
    int c,base ;     
    char * filterStr ;

#ifndef _WIN32
    extern int optind ;      /* for getopt */
    extern int opterr ;      /* for getopt */
    extern char *optarg ;    /* for getopt */
#endif
  
    programName = argv[0] ;                   /* name or program (sum/cksum) */
    p = (char*)strrchr( programName, (int)'/' ) ;  /* find the last / */
    if ( p != NULL )                          /* if there is indeed one: */
        programName = p + 1 ;                 /* just use last component */

#ifndef _WIN32
    opterr = 0;                    /* suppress getopt(3c) error messages */
    while (( c = getopt(argc, argv, ":df:l:n:")) != -1) {
      switch ( c ) {
        case 'n':
          maxOutputSamples = atoi( optarg ) ;
          break ;

        case 'l':
          infoLevel = atoi( optarg ) ;
          break ;

        case 'f':
          filtering = 1 ;
          if (strstr(optarg, "0x"))
            base=16 ;
          else
            base=10 ;

          printf( "optarg: %s, base: %d\n", optarg, base ) ;
          filter = strtol( optarg, (char **)NULL, base ) ;
          printf( "filter: %d (%x)\n", filter, filter ) ;
          break ;

        default:
          usage() ;
          break ;
      }
    }

    /* if no file specified then setup to use stdin */
    if ( optind >= argc ) {
        optind = 1 ;
        argv[optind] = "-" ;
    }

    if (( argv[optind][0] == '-' ) && ( argv[optind][1] == '\0')) {
        /* use stdin */
        fp = stdin ;
    } else {
        /* use specified file */
        if (( fp = fopen( argv[optind], "r" )) == NULL ) {
            fprintf( stderr, "%s: can't open file '%s'.\n", programName, 
                                                            argv[optind] ) ;
            usage() ;
        }
    }
#else
    for ( i = 1 ; i < argc ; i++ ) {
      if ( argv[i][0] == '-' ) {
        switch( argv[i][1] ) {
          case 'n':
             maxOutputSamples = intArg( argc, argv, &i ) ;
             break ;

          case 'l':
            infoLevel = intArg( argc, argv, &i ) ;
            break ;

          case 'f':
            filtering = 1 ;
            filterStr = strArg( argc, argv, &i ) ;
            if ( strstr( filterStr, "0x" )) {
              base=16 ;
            } else {
              base=10 ;
            }

            printf( "optarg: %s, base: %d\n", filterStr, base ) ;
            filter = strtol( filterStr, (char **)NULL, base ) ;
            printf( "filter: %d (%x)\n", filter, filter ) ;
            break ;

          default:
            usage() ;
            break ;
        }
      } else {
        if (i == (argc-1)) {
          if (( fp = fopen( argv[i], "rb" )) == NULL ) {
              fprintf( stderr, "%s: can't open file '%s'.\n", programName,
                                                            argv[i] ) ;
              usage() ;
          }
        } else {
          usage();
        }
      }
    }
    if ( fp == 0 ) {
      /* use standard in */
      fp = stdin ;
    }

#endif

    /* initialize needed static data */
    for ( i = 0 ; i < 32 ; i++ ) {
      walkingOnes32[i] = 1 << i ;
    }

    /* Begin parse of the fast binary file */
    readDataGroup( fp, infoLevel ) ;


    /* cleanup */
    cleanupSharedObjects() ;
}
  
