#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


typedef struct header
{
	char P[3];
	int x;
	int y;
	int max;
}HEADER;

typedef struct pixel{
	int R;
	int G;
	int B;
}PIXEL;

void lerPixel(int* pixR, int* pixG, int* pixB, FILE* img)
{
	fscanf(img, "%d", pixR);
	fscanf(img, "%d", pixG);
	fscanf(img, "%d", pixB);
}

void lerHeader(HEADER *head, FILE* img)
{
	char c;
	fread(head->P, sizeof(char), 2, img);
	head->P[2] = '\0';
	fseek(img, 1, SEEK_CUR);

	fread(&c, sizeof(char), 1, img);
	if(c == '#')
	{
		do{
			fread(&c, sizeof(char), 1, img);
		}while(c != '\n');
	}
	else
	fseek(img, -1, SEEK_CUR);
	fscanf(img, "%d", &head->x);
	fscanf(img, "%d", &head->y);
	fscanf(img, "%d", &head->max);
}

void lerImagem(HEADER* head, FILE* img, int** imgR, int** imgG, int** imgB)
{
	int i, j;

	for(i = 0 ; i < head->x; i++)
	{
		for(j = 0; j < head->y; j++)
		{
			lerPixel(&imgR[i][j], &imgG[i][j], &imgB[i][j], img);
		}
	}
}

void escreverImagem(int** imgR, int** imgG, int** imgB,  HEADER* head, FILE* imgSaida)
{
	int i, j;
	fseek(imgSaida, 0, SEEK_SET);

	fprintf(imgSaida, "%s\n%d %d\n%d\n", head->P, head->x, head->y, head->max);

	for(i = 0; i < head->x; i++)
	{
		for(j = 0; j < head->y; j++)
		{
			fprintf(imgSaida, "%d %d %d\n", imgR[i][j], imgG[i][j], imgB[i][j]);
		}
	}
}
int filtrarPixel(int** imgX, int X, int Y, int nLinhas, int nColunas,int TamFiltro)
{
	int i, j;
	int total = 0;
	int pixX, pixY;

	int a = X-(TamFiltro/2);
	int b = Y-(TamFiltro/2);


	for(i = 0; i<TamFiltro; i++)
	{
		for(j= 0; j<TamFiltro; j++)
		{
			pixX = a +i;
			pixY = b +j;

			if(!(pixX < 0 || pixY < 0 || pixX >= nColunas || pixY >= nLinhas))
			{
				total = total + imgX[pixX][pixY];
			}
		}
	}

	total = total/(TamFiltro*TamFiltro);

	return(total);
}


void passarFiltro(int** imgX, int** imgXF, int nLinhas, int nColunas, int TamFiltro)
{
	int i, j;
	
	for(j = 0; j < nLinhas; j++)
	{
		for(i = 0; i < nColunas; i++)
		{
		
			imgXF[i][j] = filtrarPixel(imgX, i, j, nLinhas, nColunas, TamFiltro);
		}
	}

}

int** alocarMatriz(int x, int y)
{
	int i;
	int **mat;

	mat = (int**) malloc (sizeof(int*)*x);
	if(mat == NULL)
	{
		printf("Erro ao alocar matriz\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i<x;i++)
	{

		mat[i] = (int*) malloc(sizeof(int)*y);
		if(mat[i] == NULL)
		{
			printf("Erro ao alocar matriz\n");
			exit(EXIT_FAILURE);
		}
	}
	return(mat);
}

void lerImagemGray(HEADER* head, FILE* img, int** imgGray)
{
	int i, j;

	for(i = 0 ; i < head->x; i++)
	{
		for(j = 0; j < head->y; j++)
		{
			fscanf(img, "%d", &imgGray[i][j]);
		}
	}
}

void escreverImagemGray(int** imgG, HEADER* head, FILE* imgSaida)
{
	int i, j;
	fseek(imgSaida, 0, SEEK_SET);

	fprintf(imgSaida, "%s\n%d %d\n%d\n", head->P, head->x, head->y, head->max);

	for(i = 0; i < head->x; i++)
	{
		for(j = 0; j < head->y; j++)
		{
			fprintf(imgSaida, "%d \n", imgG[i][j]);
		}
	}
}

void desalocarMatriz(int x, int **matriz)
{
	int i;
	for(i = 0; i<x; i++)
	{
    	free(matriz[i]);
	}
	free(matriz);
}


int main()
{
	HEADER head;

	FILE* img;
	FILE* imgSaida;

	char *nomeImagem = NULL;
	char *extensao = NULL;
	char c;
	int i, j;
	int ext = 0;

	int ** imgR;
	int ** imgG;
	int ** imgB;

	int ** imgRF;
	int ** imgGF;
	int ** imgBF;

	int **imgGray;
	int **imgGrayFinal;

	struct timespec clockStart, clockEnd;

	i = 0;
	j = 0;

	printf("Insira o caminho da imagem .ppm ou .pgm: ");

	while(c != '\n')
	{
		c = getchar();
		i++;
		nomeImagem = (char*) realloc(nomeImagem, sizeof(char)*i);
		nomeImagem[i-1] = c;

		if(c == '.')
			ext = 1;

		if(ext == 1)
		{
			j++;
			extensao = (char*) realloc(extensao, sizeof(char)*j);
			extensao[j-1] = c;
		}
	}
	nomeImagem[i-1] = '\0';
	extensao[j-1] = '\0';
		
	if(!strcmp(extensao, ".ppm"))
	{
		printf("Executando .PPM\n");

		img = fopen(nomeImagem, "r");
		if(img == NULL)
		{
			printf("Erro ao abrir o arquivo %s\n", nomeImagem);
			exit(EXIT_FAILURE);
		}

		lerHeader(&head, img);

		imgR = alocarMatriz(head.x, head.y);
		imgG = alocarMatriz(head.x, head.y);
		imgB = alocarMatriz(head.x, head.y);
	
		imgRF = alocarMatriz(head.x, head.y);
		imgGF = alocarMatriz(head.x, head.y);
		imgBF = alocarMatriz(head.x, head.y);
	
		lerImagem(&head, img, imgR, imgG, imgB);
	
		imgSaida = fopen("out.ppm", "w");
		if(imgSaida == NULL)
		{
			printf("Erro ao criar arquivo out.ppm\n");
			exit(EXIT_FAILURE);
		}
	
		clock_gettime(CLOCK_MONOTONIC, &clockStart);

		passarFiltro(imgR, imgRF, head.y, head.x, 5);
		passarFiltro(imgG, imgGF, head.y, head.x, 5);
		passarFiltro(imgB, imgBF, head.y, head.x, 5);
		
		clock_gettime(CLOCK_MONOTONIC, &clockEnd);

		printf("Tempo=> %fs\n", ((double)(clockEnd.tv_nsec - clockStart.tv_nsec)/1000000000) + (clockEnd.tv_sec - clockStart.tv_sec));

	
		escreverImagem(imgRF, imgGF, imgBF, &head, imgSaida);

		desalocarMatriz(head.x, imgR);
		desalocarMatriz(head.x, imgRF);

		desalocarMatriz(head.x, imgG);
		desalocarMatriz(head.x, imgGF);

		desalocarMatriz(head.x, imgB);
		desalocarMatriz(head.x, imgBF);

		fclose(imgSaida);
		fclose(img);

	}
	else if(!strcmp(extensao, ".pgm"))
	{
		printf("Executando .PGM\n");

		img = fopen(nomeImagem, "r");
		if(img == NULL)
		{
			printf("Erro ao abrir o arquivo\n");
			exit(EXIT_FAILURE);
		}
	
		lerHeader(&head, img);
	
		imgGray = alocarMatriz(head.x, head.y);
		imgGrayFinal = alocarMatriz(head.x, head.y);
	
		lerImagemGray(&head, img, imgGray);
	
		imgSaida = fopen("out.pgm", "w");
		if(imgSaida == NULL)
		{
			printf("Erro ao criar arquivo out.ppm\n");
			exit(EXIT_FAILURE);
		}
	
		passarFiltro(imgGray, imgGrayFinal, head.y, head.x, 5);

	
		escreverImagemGray(imgGrayFinal, &head, imgSaida);

		desalocarMatriz(head.x, imgGray);
		desalocarMatriz(head.x, imgGrayFinal);

		fclose(imgSaida);
		fclose(img);

	}
	else
	{
		printf("Extensao nao suportada\n");
	}
	return(0);
}
