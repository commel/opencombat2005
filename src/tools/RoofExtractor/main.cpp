#include <direct.h>
#include <stdio.h>
#include <misc\Array.h>

/**
 * This file parses and extracts roof tile images from close combat
 * .rfm files.
 */
struct Point
{
	int x, y;
};

struct Roof
{
	// A boundary for collision detection
	Array<Point> Boundary;
	// The upper left corner for rendering
	Point UpperLeft;
	// The lower right corner for rendering
	Point LowerRight;
	// The width and height of the graphic
	int Width, Height;
	// The location of the graphic data in the file
	long ExteriorDataStart;
	long InteriorDataStart;
};

static void print_roof(Roof *roof, int roofNum);
static void export_tga(char *fileName, unsigned char *data, int dataLength, int width, int height, int pad);

int
main(int argc, char *argv[])
{
	char fileName[256];

	if(argc != 3)
	{
		printf("Usage: RoofExtractor <rfm file> <map name>\n");
		return -1;
	}

	// Let's make a directory for the roof graphics
	_mkdir("building_graphics");

	// The xml file that we are creating
	sprintf(fileName, "%s.buildings.xml", argv[2]);
	FILE *xml = fopen(fileName, "w");
	fprintf(xml, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	fprintf(xml, "<Buildings>\n");

	// The rfm file that we are reading
	FILE *fp = fopen(argv[1], "rb");
	
	// Let's read the number of maps that are in this file
	fseek(fp, 4L, SEEK_SET);
	int numRoofs = 0;
	fread(&numRoofs, sizeof(int), 1, fp);

	for(int i = 0; i < numRoofs; ++i)
	{
		Roof *roof = new Roof();

		// Seek to the beginning of our header section for this roof
		fseek(fp, 20 + i*132, SEEK_SET);

		// Read in the number of boundary points
		int numBoundaryPoints=0;
		fread(&numBoundaryPoints, sizeof(int), 1, fp);

		// Read in each of these points
		for(int j = 0; j < numBoundaryPoints; ++j)
		{
			Point *p = new Point();
			fread(&(p->x), sizeof(p->x), 1, fp);
			fread(&(p->y), sizeof(p->y), 1, fp);
			roof->Boundary.Add(p);	
		}

		// Now we need to read in our extents
		fseek(fp, 20 + i*132 + 100, SEEK_SET);

		// Read in the width of a scanline. The scanline's seem to be terminated
		// with 00 00 on DWORD boundaries, so let's eliminate that
		int scanlineWidth = 0;
		fread(&scanlineWidth, sizeof(scanlineWidth), 1, fp);

		// Let's read in the upper left and lower right corners
		fread(&(roof->UpperLeft.x), sizeof(roof->UpperLeft.x), 1, fp);
		fread(&(roof->UpperLeft.y), sizeof(roof->UpperLeft.y), 1, fp);
		fread(&(roof->LowerRight.x), sizeof(roof->LowerRight.x), 1, fp);
		fread(&(roof->LowerRight.y), sizeof(roof->LowerRight.y), 1, fp);

		// Let's read in the start and the end of the data segment for this roof
		fread(&(roof->ExteriorDataStart), sizeof(roof->ExteriorDataStart), 1, fp);
		fread(&(roof->InteriorDataStart), sizeof(roof->InteriorDataStart), 1, fp);
	
		// Set the width and the height of our graphic
		roof->Width = scanlineWidth >> 1;
		roof->Height = (roof->InteriorDataStart - roof->ExteriorDataStart) / scanlineWidth;

		// Let's export this roof to our xml file
		fprintf(xml, "\t<Building>\n");
		
		// Do the Boundary first
		fprintf(xml, "\t\t<!-- Used for interior detection, in map coordinates -->\n");
		fprintf(xml, "\t\t<Boundary>\n");
		for(int j = 0; j < roof->Boundary.Count; ++j)
		{
			fprintf(xml, "\t\t\t<Point><X>%d</X><Y>%d</Y></Point>\n", roof->Boundary.Items[j]->x, roof->Boundary.Items[j]->y);
		}
		fprintf(xml, "\t\t</Boundary>\n");

		// Now the upper left corner in map coordinates
		fprintf(xml, "\t\t<!-- The Upper Left Corner, in map coordinates -->\n");
		fprintf(xml, "\t\t<Position><X>%d</X><Y>%d</Y></Position>\n", roof->UpperLeft.x, roof->UpperLeft.y);

		// Now the interior and exterior graphics
		sprintf(fileName, "%s\\roof_graphics\\exterior_%03d.tga", argv[2], i);	
		fprintf(xml, "\t\t<ExteriorGraphic>%s</ExteriorGraphic>\n", fileName);
		sprintf(fileName, "%s\\roof_graphics\\interior_%03d.tga", argv[2], i);	
		fprintf(xml, "\t\t<InteriorGraphic>%s</InteriorGraphic>\n", fileName);

		fprintf(xml, "\t</Building>\n");

		// Let's export the interior and exterior grapics
		int dataLength = roof->InteriorDataStart - roof->ExteriorDataStart;
		unsigned char *graphicData = new unsigned char[dataLength];
		
		// Exterior graphic
		int pad = 4 - ((roof->LowerRight.x-roof->UpperLeft.x)%4);
		pad = (pad == 4) ? 0 : pad;
		fseek(fp, roof->ExteriorDataStart, SEEK_SET);
		fread(graphicData, 1, dataLength, fp);
		sprintf(fileName, "roof_graphics\\exterior_%03d.tga", i);	
		export_tga(fileName, graphicData, dataLength, roof->Width, roof->Height, pad);
		
		// Interior graphic
		fseek(fp, roof->InteriorDataStart, SEEK_SET);
		fread(graphicData, 1, dataLength, fp);
		sprintf(fileName, "roof_graphics\\interior_%03d.tga", i);	
		export_tga(fileName, graphicData, dataLength, roof->Width, roof->Height, pad);

		delete graphicData;

		// Print out what we found
		print_roof(roof, i);
	}
	fprintf(xml, "</Buildings>\n");

	fclose(fp);
	fclose(xml);
	return 0;
}

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
} TGAHeader;

void
export_tga(char *fileName, unsigned char *data, int dataLength, int width, int height, int pad)
{
	TGAHeader header;

	memset(&header, 0, sizeof(TGAHeader));
	header.datatypecode = 2; // RGB
	header.width = width-pad;
	header.height = height;
	header.bitsperpixel = 16;
	//header.imagedescriptor = 1;

	FILE *fp = fopen(fileName, "wb");
	
	// Write the header
	fwrite(&header.idlength, 1, 1, fp);
	fwrite(&header.colourmaptype, 1, 1, fp);
	fwrite(&header.datatypecode, 1, 1, fp);
	fwrite(&header.colourmaporigin, 2, 1, fp);
	fwrite(&header.colourmaplength, 2, 1, fp);
	fwrite(&header.colourmapdepth, 1, 1, fp);
	fwrite(&header.x_origin, 2, 1, fp);
	fwrite(&header.y_origin, 2, 1, fp);
	fwrite(&header.width, 2, 1, fp);
	fwrite(&header.height, 2, 1, fp);
	fwrite(&header.bitsperpixel, 1, 1, fp);
	fwrite(&header.imagedescriptor, 1, 1, fp);


	// Now write the data. Remember, our stupid image has a 00 00
	// at the end of each scanline that we do not want to include
	unsigned short *sdata = (unsigned short *)data;
	unsigned short pixel = 0;
	for(int j = height-1; j >= 0; --j)
	{
		for(int i = 0; i < width-pad; ++i)
		{
			pixel = sdata[j*width+i];
			
			fwrite(&pixel, sizeof(short), 1, fp);
		}
	}
	fclose(fp);
}

void
print_roof(Roof *roof, int roofNum)
{
	printf("Roof %d:\n", roofNum);
	printf("\tWidth          = %d\n", roof->Width);
	printf("\tHeight         = %d\n", roof->Height);
	printf("\tExterior Start = %d\n", roof->ExteriorDataStart);
	printf("\tInterior End   = %d\n", roof->InteriorDataStart);
	printf("\tUpper Left     = {%d, %d}\n", roof->UpperLeft.x, roof->UpperLeft.y);
	printf("\tLower Right    = {%d, %d}\n", roof->LowerRight.x, roof->LowerRight.y);
	printf("\tBoundary:\n");
	for(int i = 0; i < roof->Boundary.Count; ++i)
	{
		printf("\t\t{%d, %d}\n", roof->Boundary.Items[i]->x, roof->Boundary.Items[i]->y);
	}
}
