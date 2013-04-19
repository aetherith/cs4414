#include <stdio.h>

int BUFFER_SIZE = 20;
FILE *source;

char* getFileExtention(FILE *source)
{

int BUFFER_SIZE = 20;
unsigned char buffer[BUFFER_SIZE];
if (source) 
{
  int y=0;
        unsigned char test[2];
	while(fread(test,2,1,source) == 1 && y<12) 
   	{
		buffer[y]=test[0];
		buffer[y+1]=test[1];
		y=y+2;
	}

// Signature intializations
	unsigned char  sigWindowsexecutable[] = {0x4D,0x5A};
	unsigned char  sigJPEG[]={0x49,0x46,0x0};
	unsigned char  sigJPG[]={0xFF,0xD8,0xFF,0xE0,0x00,0x00,0x4a,0x46};
	unsigned char  sigTIF[]={0x49,0x49,0x2A,0x0};
	unsigned char  sigTIFF[]={0x4D,0x4D,0x0,0x2A};
	unsigned char  sigPDF[]={0x25,0x50,0x44,0x46};
	unsigned char  sigPostscript1[]={0x25,0x21,0x50,0x53,0x2d,0x41,0x64,0x6f};
	unsigned char  sigGIF1[]={0x47,0x49,0x46,0x38,0x37,0x61};
	unsigned char  sigGIF2[]={0x47,0x49,0x46,0x38,0x39,0x61};
	unsigned char  sigPostscript2[]={0x50,0x53,0x46,0x2D,0x33,0x20,0x30};
	unsigned char  sigPNG[]={0x89,0x50,0x4E,0x47,0xD,0xA,0x1A,0xA};
	char *ext="";
	int exitId=0;
	int check=0;
	int i;


for(i=0; i<10;i++)
{
printf("%x ",buffer[i]);
}



for(i=0;i<2;i++)
{
		if(sigWindowsexecutable[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".exe";
check=0;

for(i=0;i<3;i++)
{
		if(sigJPEG[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".jpeg";
check=0;

for(i=0;i<4;i++)
{
		if(sigJPG[i]!=buffer[i] && (i!=4 && i!=5) )
		{

			check=1;
			break;
		}
}
if(check==0)
return ".jpg";
check=0;


for(i=0;i<4;i++)
{
		if(sigTIFF[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".tiff";
check=0;


for(i=0;i<4;i++)
{
		if(sigPostscript1[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".ps";
check=0;



for(i=0;i<4;i++)
{
		if(sigPDF[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".pdf";
check=0;




for(i=0;i<4;i++)
{
		if(sigTIF[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".tif";
check=0;

for(i=0;i<6;i++)
{
		if(sigGIF1[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".gif";
check=0;

for(i=0;i<6;i++)
{
		if(sigGIF2[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".gif";
check=0;



for(i=0;i<7;i++)
{
		if(sigPostscript2[i]!=buffer[i])
		{
			check=1;
			break;
		}
}
if(check==0)
return ".ps";
check=0;





for(i=0;i<8;i++)
{			

	
		if(sigPNG[i]!=buffer[i])
		{

			check=1;
			break;
		}
}


if(check==0)
return ".png";
check=0;




return ".txt";


}


else {
        	printf("fail at reading bytes.\n"); }

}



int main() {
unsigned char buffer[BUFFER_SIZE];
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
source = fopen("umss.exe", "rb"); // < ---- Change this file name to anything to test this code.
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
char* sample = getFileExtention(source);
printf("%s\n",sample);

  	  fclose(source);

 }


//How many inodes blocks can fit in a disc
