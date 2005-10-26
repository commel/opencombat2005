#include ".\tga.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TGA pixel data is stored in {G,B,R,A} format
 */
typedef struct {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} HEADER;

static long m_tgaIndex = 0;

TGA::TGA(void)
{
	_data = NULL;
	_originX = 0;
	_originY = 0;
	_idx = m_tgaIndex++;
}

TGA::~TGA(void)
{
	if(!_data) {
		free(_data);
	}
}

TGA *
TGA::Create(char *fileName)
{
	HEADER header;
	FILE *fptr;
	TGA *tga;
    int n=0,i,j;
    unsigned int bytes2read;
	int skipover = 0;
	unsigned char p[5];
	unsigned char *ptr;
	int w=0, h=0;

	// Open the file
    if ((fptr = fopen(fileName,"rb")) == NULL) {
       return NULL;
    }

	// Create the return value
	tga = new TGA();

    // Read in the header
    header.idlength = (char)fgetc(fptr);
    header.colourmaptype = (char)fgetc(fptr);
    header.datatypecode = (char)fgetc(fptr);
    fread(&header.colourmaporigin,2,1,fptr);
    fread(&header.colourmaplength,2,1,fptr);
    header.colourmapdepth = (char)fgetc(fptr);
    fread(&header.x_origin,2,1,fptr);
    fread(&header.y_origin,2,1,fptr);
    fread(&header.width,2,1,fptr);
    fread(&header.height,2,1,fptr);
    header.bitsperpixel = (char)fgetc(fptr);
    header.imagedescriptor = (char)fgetc(fptr);
	tga->_width = header.width;
	tga->_height = header.height;
	tga->_depth = 4;

	// Stored as 32 bit ARGB
    if((tga->_data = (unsigned char*)malloc(header.width*header.height*sizeof(char)*4)) == NULL) {
	   return NULL;
    }
    ptr = tga->_data;
	
	skipover += header.idlength;
    skipover += header.colourmaptype * header.colourmaplength;
    fseek(fptr,skipover,SEEK_CUR);

	bytes2read = header.bitsperpixel / 8;
    while(n < header.width * header.height) {
		if(header.datatypecode == 2) {
			if (fread(p,1,bytes2read,fptr) != bytes2read) {
				return NULL;
			}
			if(w >= header.width) {
				w -= header.width;
				++h;
			}
			//memcpy(ptr, p, bytes2read);
			//ptr += bytes2read;
			if(header.bitsperpixel == 32) {
				memcpy(&(ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth]), p, bytes2read);
			}
			else if(header.bitsperpixel == 24) {
				memcpy(&(ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth]), p, bytes2read);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+3] = 0xFF;				
			}
			else if(header.bitsperpixel == 16)
			{
				unsigned short pixel = ((unsigned short *)p)[0];
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+0] = (unsigned char)((pixel << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+1] = (unsigned char)(((pixel >> 5) << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+2] = (unsigned char)(((pixel >> 10) << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+3] = 0xFF;
			}
			n++;
			++w;
		} else if (header.datatypecode == 10) {
			// Compressed
			assert(0); // Not implemented yet, needs to flip the bits
			if (fread(p,1,bytes2read+1,fptr) != bytes2read+1) {
				return NULL;
			}
	        j = p[0] & 0x7f;
			memcpy(ptr, &(p[1]), bytes2read);
			ptr += bytes2read;
			n++;
			
			if(p[0] & 0x80) {         /* RLE chunk */
				for (i=0;i<j;i++) {
					memcpy(ptr, &(p[1]), bytes2read);
					ptr += bytes2read;
					n++;
				}
			} else {                   /* Normal chunk */
				for (i=0;i<j;i++) {
					if (fread(p,1,bytes2read,fptr) != bytes2read) {
						return NULL;
					}
					memcpy(ptr, p, bytes2read);
					ptr += bytes2read;
	                n++;
				}
			}
        }
    }
    fclose(fptr);

	// Let's find our origin, if it is embedded in the filename.
	// An origin exists if there are 3 or more '.' in the filename
	int count = 0;
	for(int i = 0; fileName[i] != '\0'; ++i) {
		if(fileName[i] == '.') {
			++count;
		}
	}

	if(count >= 3) {
		char fName[256];
		strcpy(fName, fileName);
		char *last = strrchr(fName, '.');
		*last = '\0';
		char *second = strrchr(fName, '.');
		*second = '\0';
		char *third = strrchr(fName, '.');
		int x = atoi(third+1);
		int y = atoi(second+1);
		tga->SetOrigin(x,y);
	}

	return tga;
}

