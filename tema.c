#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float smooth[3][3] = {{1 / 9.0, 1 / 9.0, 1 / 9.0},
				{1 / 9.0, 1 / 9.0, 1 / 9.0},
				{1 / 9.0, 1 / 9.0, 1 / 9.0}};

float blur[3][3] = {{1 / 16.0, 2 / 16.0, 1 / 16.0},
				{2 / 16.0, 4 / 16.0, 2 / 16.0},
				{1 / 16.0, 2 / 16.0, 1 / 16.0}};

float sharpen[3][3] = {{0 / 3.0, -2 / 3.0, 0 / 3.0},
				{-2 / 3.0, 11 / 3.0, -2 / 3.0},
				{0 / 3.0, -2 / 3.0, 0 / 3.0}};

float mean[3][3] = {{-1 / 1.0, -1 / 1.0, -1 / 1.0},
				{-1 / 1.0, 9 / 1.0, -1 / 1.0},
				{-1 / 1.0, -1 / 1.0, -1 / 1.0}};

float emboss[3][3] = {{0, -1, 0},
				{0, 0 , 0 },
				{0 , 1, 0}};

typedef struct img{
    unsigned char** w; 
    unsigned char** r; 
    unsigned char** g;
    unsigned char** b;
    int type; 
    int width;
    int height;
    int maxval;
}img;

void apply(int start, int stop, float filter[3][3], img *image, img *image2){
	//for(int a = 0; a < 150; a++){
	for(int x = start; x < stop; x++)
		for(int l = 1; l < (*image).width-1; l++){
			if (x == 0 || x == (*image).height-1){ 	
				(*image2).r[x][l] = 0;
				(*image2).g[x][l] = 0;
				(*image2).b[x][l] = 0;
			}
			else {
				float s = 0;
				if (s <= 255 && s >= 0)
						s = s + (*image).r[x-1][l-1] * filter[0][0] + 
						(*image).r[x-1][l] * filter[0][1] +
						(*image).r[x-1][l+1] * filter[0][2] +
						(*image).r[x][l-1] * filter[1][0] +
						(*image).r[x][l] * filter[1][1] +
						(*image).r[x][l+1] * filter[1][2] +
						(*image).r[x+1][l-1] * filter[2][0] +
						(*image).r[x+1][l] * filter[2][1] +
						(*image).r[x+1][l+1] * filter[2][2];
				if (s > 255)
					s = 255;
				else if (s < 0)
					s = 0;
				(*image2).r[x][l] = s;

				s = 0;
				if (s <= 255 && s >= 0)
						s = s + (*image).g[x-1][l-1] * filter[0][0] + 
						(*image).g[x-1][l] * filter[0][1] +
						(*image).g[x-1][l+1] * filter[0][2] +
						(*image).g[x][l-1] * filter[1][0] +
						(*image).g[x][l] * filter[1][1] +
						(*image).g[x][l+1] * filter[1][2] +
						(*image).g[x+1][l-1] * filter[2][0] +
						(*image).g[x+1][l] * filter[2][1] +
						(*image).g[x+1][l+1] * filter[2][2];
				if (s > 255)
					s = 255;
				else if (s < 0)
					s = 0;

				(*image2).g[x][l] = s;

				s = 0;
				if (s <= 255 && s >= 0)
						s = s + (*image).b[x-1][l-1] * filter[0][0] + 
						(*image).b[x-1][l] * filter[0][1] +
						(*image).b[x-1][l+1] * filter[0][2] +
						(*image).b[x][l-1] * filter[1][0] +
						(*image).b[x][l] * filter[1][1] +
						(*image).b[x][l+1] * filter[1][2] +
						(*image).b[x+1][l-1] * filter[2][0] +
						(*image).b[x+1][l] * filter[2][1] +
						(*image).b[x+1][l+1] * filter[2][2];
				if (s > 255)
						s = 255;
				else if (s < 0)
					s = 0;
				(*image2).b[x][l] = s;
			}
		}
	//}
}

int main(int argc, char * argv[]){

	int j,k;
	int rank;
	int n;
	img image;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n);

	img image2;

	if (rank == 0){
		
		char *name = (char*)calloc(100, sizeof(char));
		name = argv[1];

		FILE *f = fopen(name, "r");

		unsigned char c;
		c = getc(f);
		int type;
		while ((c = getc(f)) != '\n')
			type = c - '0';
		
		while ((c = getc(f)) != '\n'){

		}

		char *w = (char*)calloc(10, sizeof(char));
		int width;
		int i = 0;
		while ((c = getc(f)) != ' '){
			w[i++] = c;
		}
		width = atoi(w);


		char *h = (char*)calloc(10, sizeof(char));
		int height;
		i = 0;
		while ((c = getc(f)) != '\n'){
			h[i++] = c;
		}
		height = atoi(h);

		char *m = (char*)calloc(10, sizeof(unsigned char));
		int maxval;
		i = 0;
		while ((c = getc(f)) != '\n'){
			m[i++] = c;
		}
		maxval = atoi(m);
		
		image.type = type;
		image.width = width + 2;
		image.height = height + 2;
		image.maxval = maxval;

		if (type == 5){
			image.w= (unsigned char **) calloc(image.height, sizeof(unsigned char *));
        	for (int i = 0; i < image.height; i++) {
            	image.w[i] = (unsigned char *) calloc(image.width, sizeof(unsigned char));
	        }

    	    unsigned char *buffer = (unsigned char *)calloc(image.width, image.height);
    		fread(buffer, sizeof(unsigned char), image.width* image.height, f);

	    	int k = 0;
    		for (int i = 1; i < image.height-1; i++) {
    			for (int j = 1; j < image.width-1; j++) {
    				image.w[i][j] = buffer[k++];
	    		}
    		}
			image.r = image.g = image.b = NULL;

		}
		else if (type == 6){
			image.r = (unsigned char**)calloc(image.height, sizeof(unsigned char *));
			for(i = 0; i < image.height; i++)
				image.r[i] = (unsigned char*)calloc(image.width, sizeof(unsigned char));

			image.g = (unsigned char**)calloc(image.height, sizeof(unsigned char *));
				for(i = 0; i < image.height; i++)
			image.g[i] = (unsigned char*)calloc(image.width, sizeof(unsigned char));

			image.b = (unsigned char**)calloc(image.height, sizeof(unsigned char *));
			for(i = 0; i < image.height; i++)
				image.b[i] = (unsigned char*)calloc(image.width, sizeof(unsigned char));

			unsigned char *colour = (unsigned char*)calloc(3 * image.height * image.width, sizeof(unsigned char));
	
			i = 0;
			while (i < image.height * image.width * 3){
				colour[i] = getc(f);
				i++;
			}

			k = 0;
			for(i = 1; i < image.height - 1; i++)
				for(j = 1; j < image.width - 1; j++){
					image.r[i][j] = colour[k];
					image.g[i][j] = colour[k+1];
					image.b[i][j] = colour[k+2];
					k = k + 3;
				}

			image.w = NULL;

		}
		fclose(f);
	}

	MPI_Bcast(&image.width, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&image.height, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&image.type, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	image2.height = image.height ;
	image2.width = image.width ;
	image2.type = image.type;
	
	int i;

	if (rank != 0){
		if (image.type == 6){
			image.r = (unsigned char **) malloc(sizeof(unsigned char*) * image.height);
			for (i = 0; i < image.height; ++i) {
				image.r[i] = (unsigned char*) malloc(sizeof(unsigned char) * image.width);
			}

			image.g = (unsigned char **) malloc(sizeof(unsigned char*) * image.height);
			for (i = 0; i < image.height; ++i) {
				image.g[i] = (unsigned char*) malloc(sizeof(unsigned char) * image.width);
			}

			image.b = (unsigned char **) malloc(sizeof(unsigned char*) * image.height);
			for (i = 0; i < image.height; ++i) {
				image.b[i] = (unsigned char*) malloc(sizeof(unsigned char) * image.width);
			}
		}

		else if (image.type == 5){
			image.w = (unsigned char **) malloc(sizeof(unsigned char*) * image.height);

			for (i = 0; i < image.height; ++i) {
				image.w[i] = (unsigned char*) malloc(sizeof(unsigned char) * image.width);
			}

		}
	}

	if (image2.type == 5){
		image2.w= (unsigned char **) malloc(image2.height* sizeof(unsigned char *));
        	for (int i = 0; i < image2.height; i++) {
            	image2.w[i] = (unsigned char *) malloc(image2.width * sizeof(unsigned char));
		    }

		image2.r = image2.g = image2.b = NULL;
	}
	else if (image2.type == 6){
		image2.r = (unsigned char**)calloc(image2.height, sizeof(unsigned char *));
			for(i = 0; i < image2.height; i++)
			image2.r[i] = (unsigned char*)calloc(image2.width, sizeof(unsigned char));

		image2.g = (unsigned char**)calloc(image2.height, sizeof(unsigned char *));
			for(i = 0; i < image2.height; i++)
				image2.g[i] = (unsigned char*)calloc(image2.width, sizeof(unsigned char));

		image2.b = (unsigned char**)calloc(image2.height, sizeof(unsigned char *));
		for(i = 0; i < image2.height; i++)
			image2.b[i] = (unsigned char*)calloc(image2.width, sizeof(unsigned char));

		image2.w = NULL;
		}

	int nrargumente = argc;

	for(int i = 3; i < nrargumente; i++){

		if (image.type == 6){
			for (int i = 0; i < image.height; i++) {
				MPI_Bcast(&image.r[i][0], image.width, MPI_CHAR, 0, MPI_COMM_WORLD);
				MPI_Bcast(&image.g[i][0], image.width, MPI_CHAR, 0, MPI_COMM_WORLD);
				MPI_Bcast(&image.b[i][0], image.width, MPI_CHAR, 0, MPI_COMM_WORLD);
			}
		}
		else if (image.type == 5){
			for (int i = 0; i < image.height; i++) {
				MPI_Bcast(&image.w[i][0], image.width, MPI_CHAR, 0, MPI_COMM_WORLD);
			}
		}

		int row = image.height;

		int start = (row / n) * rank;
		int stop = (row / n) * (rank + 1);

		if (rank == n-1)
			stop = row;

		if (image.type == 5){
			if (strcmp(argv[i], "emboss") == 0){
				//for(int a = 0; a < 100; a++){
				for(int x = start; x < stop; x++)
					for(int l = 1; l < image.width-1; l++){
						if (x == 0 || x == image.height-1){
							image2.w[x][l] = 0;
						}
						else {
							float s = 0;
							if (s <= 255 && s >= 0)
									s = s + image.w[x-1][l-1] * emboss[0][0] + 
									image.w[x-1][l] * emboss[0][1] +
									image.w[x-1][l+1] * emboss[0][2] +
									image.w[x][l-1] * emboss[1][0] +
									image.w[x][l] * emboss[1][1] +
									image.w[x][l+1] * emboss[1][2] +
									image.w[x+1][l-1] * emboss[2][0] +
									image.w[x+1][l] * emboss[2][1] +
									image.w[x+1][l+1] * emboss[2][2];
							if (s > 255)
								s = 255;
							else if (s < 0)
								s = 0;
							image2.w[x][l] = s;
						}
					}
				//}
			}

			if (strcmp(argv[i], "smooth") == 0){
				//for(int a = 0; a < 100; a++){
				for(int x = start; x < stop; x++)
					for(int l = 1; l < image.width-1; l++){
						if (x == 0 || x == image.height-1){
							image2.w[x][l] = 0;
						}
						else {
							float s = 0;
							if (s <= 255 && s >= 0)
									s = s + image.w[x-1][l-1] * smooth[0][0] + 
									image.w[x-1][l] * smooth[0][1] +
									image.w[x-1][l+1] * smooth[0][2] +
									image.w[x][l-1] * smooth[1][0] +
									image.w[x][l] * smooth[1][1] +
									image.w[x][l+1] * smooth[1][2] +
									image.w[x+1][l-1] * smooth[2][0] +
									image.w[x+1][l] * smooth[2][1] +
									image.w[x+1][l+1] * smooth[2][2];
							if (s > 255)
								s = 255;
							else if (s < 0)
								s = 0;
							image2.w[x][l] = s;
						}
					}
				//}
			}

				if (strcmp(argv[i], "blur") == 0){
				//for(int a = 0; a < 100; a++){
				for(int x = start; x < stop; x++)
					for(int l = 1; l < image.width-1; l++){
						if (x == 0 || x == image.height-1){
							image2.w[x][l] = 0;
						}
						else {
							float s = 0;
							if (s <= 255 && s >= 0)
									s = s + image.w[x-1][l-1] * blur[0][0] + 
									image.w[x-1][l] * blur[0][1] +
									image.w[x-1][l+1] * blur[0][2] +
									image.w[x][l-1] * blur[1][0] +
									image.w[x][l] * blur[1][1] +
									image.w[x][l+1] * blur[1][2] +
									image.w[x+1][l-1] * blur[2][0] +
									image.w[x+1][l] * blur[2][1] +
									image.w[x+1][l+1] * blur[2][2];
							if (s > 255)
								s = 255;
							else if (s < 0)
								s = 0;
							image2.w[x][l] = s;
						}
					}
				//}
			}
				if (strcmp(argv[i], "mean") == 0){
				//	for(int a = 0; a < 100; a++){
				for(int x = start; x < stop; x++)
					for(int l =1; l < image.width -1; l++){
						if (x == 0 || x == image.height-1){
							image2.w[x][l] = 0;
						}
						else {
							float s = 0;
							if (s <= 255 && s >= 0)
									s = s + image.w[x-1][l-1] * mean[0][0] + 
									image.w[x-1][l] * mean[0][1] +
									image.w[x-1][l+1] * mean[0][2] +
									image.w[x][l-1] * mean[1][0] +
									image.w[x][l] * mean[1][1] +
									image.w[x][l+1] * mean[1][2] +
									image.w[x+1][l-1] * mean[2][0] +
									image.w[x+1][l] * mean[2][1] +
									image.w[x+1][l+1] * mean[2][2];
							if (s > 255)
								s = 255;
							else if (s < 0)
								s = 0;
							image2.w[x][l] = s;
						}
					}
				//}
			}
				if (strcmp(argv[i], "sharpen") == 0){
				//	for(int a = 0; a < 100; a++){
				for(int x = start; x < stop; x++)
					for(int l = 1; l < image.width-1; l++){
						if (x == 0 || x == image.height-1 ){
							image2.w[x][l] = 0;
						}
						else {
							float s = 0;
							if (s <= 255 && s >= 0)
									s = s + image.w[x-1][l-1] * sharpen[0][0] + 
									image.w[x-1][l] * sharpen[0][1] +
									image.w[x-1][l+1] * sharpen[0][2] +
									image.w[x][l-1] * sharpen[1][0] +
									image.w[x][l] * sharpen[1][1] +
									image.w[x][l+1] * sharpen[1][2] +
									image.w[x+1][l-1] * sharpen[2][0] +
									image.w[x+1][l] * sharpen[2][1] +
									image.w[x+1][l+1] * sharpen[2][2];
							if (s > 255)
								s = 255;
							else if (s < 0)
								s = 0;
							image2.w[x][l] = s;
						}
					}
				//}
			}

			for(int p = start; p < stop; p++)
				MPI_Send(&image2.w[p][0], image.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		}
		else if (image.type == 6){
			if (strcmp(argv[i], "emboss") == 0){
				apply(start, stop, emboss, &image, &image2);
			}
			if (strcmp(argv[i], "smooth") == 0){
				apply(start, stop, smooth, &image, &image2);
			}
			if (strcmp(argv[i], "blur") == 0){
				apply(start, stop, blur, &image, &image2);
			}
			if (strcmp(argv[i], "sharpen") == 0){
				apply(start, stop, sharpen, &image, &image2);
			}
			if (strcmp(argv[i], "mean") == 0){
				apply(start, stop, mean, &image, &image2);
			}
			for(int p = start; p < stop; p++){
					MPI_Send(&image2.r[p][0], image.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
					MPI_Send(&image2.g[p][0], image.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
					MPI_Send(&image2.b[p][0], image.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
				}
		}

		if (rank == 0){
			for (int ri = 0; ri < n; ri++) {

				int nr;
				nr = image.height / n;

				int start = nr * ri;
				int stop  = nr * (ri + 1);

				if (ri == n-1) {
					stop = image.height;
				}

			if (image.type == 5)
				for (int p = start; p < stop; p++) {
					MPI_Recv(&image.w[p][0], image.width, MPI_CHAR, ri, 0, MPI_COMM_WORLD, &status);
				}
			else if (image.type == 6){
				for (int p = start; p < stop; p++) {
					MPI_Recv(&image.r[p][0], image.width, MPI_CHAR, ri, 0, MPI_COMM_WORLD, &status);
					MPI_Recv(&image.g[p][0], image.width, MPI_CHAR, ri, 0, MPI_COMM_WORLD, &status);
					MPI_Recv(&image.b[p][0], image.width, MPI_CHAR, ri, 0, MPI_COMM_WORLD, &status);
				}
			}
			}
		}

	}

	if (rank == 0){
		int i;
		char *name2 = (char*)calloc(100, sizeof(char));
		name2 = argv[2];

		FILE *g = fopen(name2, "w");

		if (image.type == 5){
			fprintf(g, "P5\n");
		}
		else if (image.type == 6){
			fprintf(g, "P6\n");
		}

		fprintf(g, "%d ", image.width-2);
		fprintf(g, "%d\n", image.height-2);
		fprintf(g, "%d\n", image.maxval);

		if (image.type == 5){
			for(i = 1; i < image.height -1; i++)
				for(j = 1; j < image.width -1; j++){
				fprintf(g, "%c", image.w[i][j]);
			}
		}
		else if (image.type == 6){
		
			for(i = 1; i < image.height-1; i++){
				for(j = 1; j <  image.width - 1; j++){
					fprintf(g, "%c", image.r[i][j]);
					fprintf(g, "%c", image.g[i][j]);
					fprintf(g, "%c", image.b[i][j]);
				}
			}
		}
		fclose(g);
	}

	MPI_Finalize();

	return 0;
}