#include <stdio.h>
#include <stdlib.h>

int main()
{
	char ch[] = "oi " ;
	char ch2[] = "Roberto" ;

	char buffer [10] ;

	snprintf(buffer, 11, "%s%s", ch, ch2) ;

	printf("%s\n", buffer) ;
	return 0 ;
}