#include <stdio.h> 
#include <unistd.h> 
#include <strings.h> //for the bzero function
#include <time.h>   //for the clock_t that is a typedef of long int
#include <errno.h> 
#include <sys/time.h> //for the timeval structure 


int main(int argc, char const *argv[]) 
{ 
	FILE *fp1; 
	fp1 = fopen("1gb.txt", "r");
    if(fp1 == NULL){
    	perror("Error in opening file: the file does not exist");
    	return -1;// exit(1);
    }
    
    char buffer[65536] = {0};
    int iter = 0;
    
    //How to measure time taken to read the content of the file
	// First option is using the clock() function that is present in the time.h header file
	clock_t start, end;
    // Second option is using the struct timeval
	struct timeval tvalBefore, tvalAfter;

	start = clock(); //returns the number of elapsed CPU clock cycles
	gettimeofday (&tvalBefore, NULL); //measure execution time in milliseconds

	while(fgets(buffer, sizeof(buffer), fp1) != NULL) {//while( (b = fread(buffer, 1, sizeof(buffer), fp1))>0 )
		printf("iteration: %d\n", ++iter);
	    bzero(buffer, 65536);
   	}
   	end = clock();
	gettimeofday(&tvalAfter, NULL);
	double timeTaken = ((double)(end - start))/CLOCKS_PER_SEC; // calculate the elapsed time. CLOCK_PER_SEC is a number of clock ticks per second.
	printf("1. It taken %f seconds to read all data\n", timeTaken);
	
	double time_taken = (tvalAfter.tv_sec - tvalBefore.tv_sec) * 1000.0 
						+ ((float)tvalAfter.tv_usec - tvalBefore.tv_usec) / 1000.0;
	fprintf(stdout, "2. It taken: %.2f millisecond\n", time_taken);
	printf("3. Execution time in microseconds: %ld microseconds\n",
        ((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L
       +tvalAfter.tv_usec) - tvalBefore.tv_usec); 
     
	fclose(fp1);
	return 0; 
} 

