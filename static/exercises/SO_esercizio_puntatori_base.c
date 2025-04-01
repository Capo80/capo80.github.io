#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN	10

#define FLUSH_STDIN while (getchar() != '\n') 

void main() {

	char array[ARRAY_LEN];
	char type;
	char size, curr;
	unsigned long buf;

	while (1) {

		printf("Tipo dato:\n");
		scanf("%c", &type);
		
		FLUSH_STDIN;
		switch (type) {

			case 's':
				scanf("%hd", (short *)&buf);
				size = sizeof(short);
				break;

			case 'l':
				scanf("%ld", &buf);
				size = sizeof(long);
				break;

			default:
				printf("Invalid type\n");
				break;

		
		}

		FLUSH_STDIN;
		//size = dimensione letta
		
		if (curr + size < ARRAY_LEN) {
			memcpy(&array[curr], &buf, size);
			curr += size;
		} else {	
			break;
		}

		printf("Dato inserito\n");

	}

	for (int i = 0; i < ARRAY_LEN; i++) {

		printf("%x ", array[i]);
	}

	printf("\n");




}
