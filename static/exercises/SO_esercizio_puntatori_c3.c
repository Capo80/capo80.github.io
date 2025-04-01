#include<stdio.h>
#include<stdlib.h>

#define FLUSH_STDIN while (getchar() != '\n') 

#define ARRAY_LEN 10

void print_array(char* array, int len) {

	for (int i = 0; i < len; i++) {

		printf("array[%d] = %x\n", i, array[i]);
	}

}

void main() {

	char array[ARRAY_LEN] = {0};
	char array_types[ARRAY_LEN] = {0};

	char over = 0, end = ARRAY_LEN, curr = 0, done = 0; 
	char type, size;
	unsigned long buf;

	while (1) {

		
		printf("Enter type:\n");
		scanf("%c", &type);

		FLUSH_STDIN;

		switch (type) {
			
			case 'c':
				printf("Enter char:\n");
				scanf("%c", (char*) &buf);
				size = 1;
				break;

			case 's':
				printf("Enter short:\n");
				scanf("%hd", (short*) &buf);
				size = 2;
				break;

			case 'i':
				printf("Enter int:\n");
				scanf("%d", (int*) &buf);
				size = 4;
				break;

			case 'l':
				printf("Enter long:\n");
				scanf("%ld", (long*) &buf);
				size = 8;
				break;
			case 'e':
				done = 1;
				break;

		}
		FLUSH_STDIN;

		if (done)
			break;

		if (curr + size > ARRAY_LEN) {
	
			int i, j;
			char size_left = ARRAY_LEN - curr;
			/* printf("size left: %x\n", size_left); */
			for (i = 0; i < size_left; i++) {
				array[curr + i] = *(((char*) &buf) + i);
				array_types[curr + i] = 0;
			}

			for (j = 0; j < (size - size_left); j++) {
				array[j] = *(((char*) &buf) + i + j);
				array_types[j] = 0;
			}		

			array_types[curr] = size;
			over = 1;
			curr = (curr + size) % ARRAY_LEN;

		} else {

			switch (size) {
				case 1:
					array[curr] = *((char*) &buf);
					array_types[curr] = size;
					curr += size;
					break;
				case 2:
					*((short*)(array + curr)) = *((short*) &buf);
					array_types[curr] = size;
					curr += size;
					break;
				case 4:
					*((int*)(array + curr)) = *((int*) &buf);
					array_types[curr] = size;
					curr += size;
					break;

				case 8:
					*((long*)(array + curr)) = *((long*) &buf);
					array_types[curr] = size;
					curr += size;
					break;


			}

		}

		/* printf("Array:\n"); */
		/* print_array(array, ARRAY_LEN); */
		/**/
		/* printf("Array types:\n"); */
		/* print_array(array_types, ARRAY_LEN); */
		/**/
		/* printf("Curr: %d\n", curr); */


	}


	printf("Array:\n");


	if (over) {

		while (array_types[curr] == 0) {
			curr = (curr + 1) % ARRAY_LEN;
		}
	} else { 
		curr = 0;
	}

	int distance = 0;
	while (distance < ARRAY_LEN) {
	
		/* printf("%d %d\n", distance, curr); */
		size = array_types[curr];
		if (curr + size < ARRAY_LEN) {
			switch (size) {

				case 1:
					printf("%c\n", array[curr]);
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 2:
					printf("%hd\n", *((short*)(array + curr)));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 4:
					printf("%d\n", *((int*)(array + curr)));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 8:
					printf("%ld\n", *((long*)(array + curr)));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 0:
					exit(1);
			}
		} else {

			int i, j;
			char size_left = ARRAY_LEN - curr;
			/* printf("size left: %x\n", size_left); */
			for (i = 0; i < size_left; i++) {
				/* printf("i: %d = %x\n", i, array[curr + i]); */
				*(((char*) &buf) + i) = array[curr + i]; 
			}

			for (j = 0; j < (size - size_left); j++) {
				/* printf("j: %d %d = %x\n",i,  j, array[j]); */

				*(((char*) &buf) + i + j) = array[j]; 
			}


			switch (size) {

				case 1:
					printf("%c\n", *((char*)&buf));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 2:
					printf("%hd\n", *((short*)&buf));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 4:
					printf("%d\n", *((int*)&buf));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 8:
					printf("%ld\n", *((long*)&buf));
					curr = (curr + size) % ARRAY_LEN;
					distance += size;
					break;
				case 0:
					exit(1);
			}
		}
	}

}
