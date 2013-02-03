#include <stdio.h>

int main( int argc, char *argv[] )
{
  char c = getchar();
  while( c != EOF )
    {
      printf("%c",c);
      c = getchar();
    }
  return 0;
};
