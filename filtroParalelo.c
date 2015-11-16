#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <mpi.h>

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

void paralelizaFiltro(int** imgX, int** imgXF, int nLinhas, int nColunas, int TamFiltro, int comecaLinha, int terminaLinha)
{
	int i, j;
	///PARALELIZA, divide em 4 processadores
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	for(j = comecaLinha; j < terminaLinha; j++)
	{
		for(i = 0; i < nColunas; i++)
		{
			imgXF[i][j] = filtrarPixel(imgX, i, j, nLinhas, nColunas, TamFiltro);
		}
	}

}


void passarFiltro(int** imgX, int** imgXF, int nLinhas , int nColunas, int TamFiltro, int JobInit, int JobEnd)//Inverti nColunas, nLinhas!!!
{
	int i;
	int comecaLinha[4], terminaLinha[4];
	int divLinhas, restoLinhas;

	divLinhas = nLinhas/4;//Cada Processador ficara responsavel por "nLinhas/4" linhas
	restoLinhas = nLinhas%4;//Se a quantidade de linhas nao eh divisivel por 4 então a ultima thread processa "divLinhas+restoLinhas" Linhas

	comecaLinha[0] = 0;
	terminaLinha[0] = comecaLinha[0] + divLinhas;

	comecaLinha[1] = terminaLinha[0] ;
	terminaLinha[1] = comecaLinha[1] + divLinhas;

	comecaLinha[2] = terminaLinha[1];
	terminaLinha[2] = comecaLinha[2] + divLinhas;

	comecaLinha[3] = terminaLinha[2];
	terminaLinha[3] = comecaLinha[3] + divLinhas + restoLinhas;

	#pragma omp parallel for
	for(i = 0; i<4; i++)
	{
		paralelizaFiltro(imgX, imgXF, nLinhas, nColunas, TamFiltro, comecaLinha[i], terminaLinha[i]);
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

void passarFiltroMPI()
{

	//int rank;
	int i, j;
	int **mat; 
	int **matFinal;
	int JobInit;
	int JobEnd;
	int linhas, colunas;



	MPI_Recv(&linhas, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&colunas, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&JobInit, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&JobEnd, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	mat = alocarMatriz(linhas, colunas);
	matFinal = alocarMatriz(linhas, colunas);


	for(i = 0; i<linhas; i++)//Recebe as linhas
		MPI_Recv(mat[i], colunas, MPI_INT, 0, 4+i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	passarFiltro(mat, matFinal, colunas, linhas, 5, JobInit, JobEnd);

	j = 0;
	for(i = JobInit; i< JobEnd; i++)
	{
		MPI_Send(&matFinal[i][0], colunas, MPI_INT, 0, j, MPI_COMM_WORLD);
		j++;
	}

	desalocarMatriz(linhas, mat);
	desalocarMatriz(linhas, matFinal);

}

int main(int argc, char **argv)
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


	int numtask, rank, rc;
	int k;
	int divi;
	int resto;
	int qtdLinhas;
	int JobInit;
	int JobEnd;
	int linhaAPassar;

	i = 0;
	j = 0;

	rc = MPI_Init(&argc, &argv);
	if(rc == MPI_SUCCESS)
	{
		MPI_Comm_size(MPI_COMM_WORLD, &numtask);//Salva o numero de threads criadas em  numtask
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//Verifica o rank desta thread(Cada thread criada pelo MPI tem um rank que vai de [0..numtask-1], ele que indentifica  qual thread é qual!!!)

		if(rank == 0)
		{
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
					MPI_Finalize();
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
		
				int RankTag;
				int divProcess;
				int restoProcess;

				divProcess = (numtask-1)/3;//Divide quantos Nos cuidarao de cada imagem
				restoProcess = (numtask-1)%3;

				divi = head.x/((numtask-1)/3);//Define quantas linhas da matriz cada thread vai ter
				resto = head.x%((numtask-1)/3);//Pega o resto

				//Envia imgR
				linhaAPassar = 0;
				for(i = 0; i< divProcess; i++)
				{
					RankTag = i+1;
	
					//Calcula quantas linhas vao ser comunicadas!
					if(i == 0)
					{
						qtdLinhas = divi +2;//Se for o primeiro bloco de linhas envia soh 2 linhas bonus!
						JobInit = 0; //Indica as linhas que serao filtradas
						JobEnd = divi;
						
					}
					else if(i == divProcess-1)
					{
						qtdLinhas = divi+resto+2;//Se for a ultima, envia 2 linhas bonus + resto da divisao!
						JobInit = 2;
						JobEnd = qtdLinhas;
	
						linhaAPassar = linhaAPassar-4;
						
					}
					else
					{
						qtdLinhas = divi+4;
						JobInit = 2;
						JobEnd = qtdLinhas-2;
	
						linhaAPassar = linhaAPassar-4;
					}
	
					//Envia quantide de linhas
					MPI_Send(&qtdLinhas, 1, MPI_INT, RankTag, 0, MPI_COMM_WORLD);
					//Envia quantas colunas por linha serao enviadas
					MPI_Send(&head.y, 1, MPI_INT, RankTag, 1, MPI_COMM_WORLD);
					//Envia onde comeca o processamento
					MPI_Send(&JobInit, 1, MPI_INT, RankTag, 2, MPI_COMM_WORLD);
					//Envia onde termina o processamento
					MPI_Send(&JobEnd, 1, MPI_INT, RankTag, 3, MPI_COMM_WORLD);
	
					for(k = 0; k<qtdLinhas; k++)
					{
						MPI_Send(&imgR[linhaAPassar][0], head.y, MPI_INT, RankTag, 4+k, MPI_COMM_WORLD);//Para enviar uma matriz, vc tem que enviar uma linha por vez!(Tem outras formas de fazer isso!!!)
						linhaAPassar++;
					}
	
				}

				//Envia imgG
				linhaAPassar = 0;
				for(i = 0; i< divProcess; i++)
				{
					RankTag = i+1 + divProcess;
					
					//Calcula quantas linhas vao ser comunicadas!
					if(i == 0)
					{
						qtdLinhas = divi +2;//Se for o primeiro bloco de linhas envia soh 2 linhas bonus!
						JobInit = 0; //Indica as linhas que serao filtradas
						JobEnd = divi;
						
					}
					else if(i == divProcess-1)
					{
						qtdLinhas = divi+resto+2;//Se for a ultima, envia 2 linhas bonus + resto da divisao!
						JobInit = 2;
						JobEnd = qtdLinhas;
	
						linhaAPassar = linhaAPassar-4;
						
					}
					else
					{
						qtdLinhas = divi+4;
						JobInit = 2;
						JobEnd = qtdLinhas-2;
	
						linhaAPassar = linhaAPassar-4;
					}
	
					//Envia quantide de linhas
					MPI_Send(&qtdLinhas, 1, MPI_INT, RankTag, 0, MPI_COMM_WORLD);
					//Envia quantas colunas por linha serao enviadas
					MPI_Send(&head.y, 1, MPI_INT, RankTag, 1, MPI_COMM_WORLD);
					//Envia onde comeca o processamento
					MPI_Send(&JobInit, 1, MPI_INT, RankTag, 2, MPI_COMM_WORLD);
					//Envia onde termina o processamento
					MPI_Send(&JobEnd, 1, MPI_INT, RankTag, 3, MPI_COMM_WORLD);
	
					for(k = 0; k<qtdLinhas; k++)
					{
						MPI_Send(&imgG[linhaAPassar][0], head.y, MPI_INT, RankTag, 4+k, MPI_COMM_WORLD);//Para enviar uma matriz, vc tem que enviar uma linha por vez!(Tem outras formas de fazer isso!!!)
						linhaAPassar++;
					}
				}

			head.x%(div	divi = head.x/(divProcess+restoProcess);//Refaz os calculos da divisao usando o resto dos processos!!!!
				resto = Process+restoProcess);//Calcula o resto

				//Envia imgB
				linhaAPassar = 0;
				for(i = 0; i< divProcess+restoProcess; i++)
				{
					RankTag = i+1 +(2*divProcess);
	
					//Calcula quantas linhas vao ser comunicadas!
					if(i == 0)
					{
						qtdLinhas = divi +2;//Se for o primeiro bloco de linhas envia soh 2 linhas bonus!
						JobInit = 0; //Indica as linhas que serao filtradas
						JobEnd = divi;
						
					}
					else if(i == divProcess+restoProcess-1)
					{
						qtdLinhas = divi+resto+2;//Se for a ultima, envia 2 linhas bonus + resto da divisao!
						JobInit = 2;
						JobEnd = qtdLinhas;
	
						linhaAPassar = linhaAPassar-4;
						
					}
					else
					{
						qtdLinhas = divi+4;
						JobInit = 2;
						JobEnd = qtdLinhas-2;
	
						linhaAPassar = linhaAPassar-4;
					}
	
					//Envia quantide de linhas
					MPI_Send(&qtdLinhas, 1, MPI_INT, RankTag, 0, MPI_COMM_WORLD);
					//Envia quantas colunas por linha serao enviadas
					MPI_Send(&head.y, 1, MPI_INT, RankTag, 1, MPI_COMM_WORLD);
					//Envia onde comeca o processamento
					MPI_Send(&JobInit, 1, MPI_INT, RankTag, 2, MPI_COMM_WORLD);
					//Envia onde termina o processamento
					MPI_Send(&JobEnd, 1, MPI_INT, RankTag, 3, MPI_COMM_WORLD);
	
					for(k = 0; k<qtdLinhas; k++)
					{
						//printf("Linha = %d passada para Rank = %d\n",linhaAPassar, i);
						MPI_Send(&imgB[linhaAPassar][0], head.y, MPI_INT, RankTag, 4+k, MPI_COMM_WORLD);//Para enviar uma matriz, vc tem que enviar uma linha por vez!(Tem outras formas de fazer isso!!!)
						linhaAPassar++;
					}

				}

				//Recebendo imgR);
				for(i = 0;i < divProcess; i++)
				{
					RankTag = i+1;	
					if(RankTag != divProcess-1)
						qtdLinhas = divi;
					else qtdLinhas = divi + resto;

					for(k = 0; k< qtdLinhas; k++)
					{
						MPI_Recv(imgRF[((i)*divi)+k], head.y, MPI_INT, RankTag, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}

				//Recebendo imgG);
				for(i = 0;i < divProcess; i++)
				{
					RankTag = i+1+divProcess;	
					if(RankTag != divProcess-1)
						qtdLinhas = divi;
					else qtdLinhas = divi + resto;

					for(k = 0; k< qtdLinhas; k++)
					{
						MPI_Recv(imgGF[((i)*divi)+k], head.y, MPI_INT, RankTag, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}

				//Recebendo imgB);
				for(i = 0;i < divProcess+restoProcess; i++)
				{
					RankTag = i+1+(2*divProcess);	
					if(RankTag != divProcess+restoProcess-1)
						qtdLinhas = divi;
					else qtdLinhas = divi + resto;

					for(k = 0; k< qtdLinhas; k++)
					{
						MPI_Recv(imgBF[((i)*divi)+k], head.y, MPI_INT, RankTag, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}
				///////////////
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
					MPI_Finalize();
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
		
				clock_gettime(CLOCK_MONOTONIC, &clockStart);

				divi = head.x/(numtask-1);//Define quantas linhas da matriz cada thread vai ter
				resto = head.x%(numtask-1);//Pega o resto

				linhaAPassar = 0;

				for(i = 1; i< numtask; i++)
				{
					
					//Calcula quantas linhas vao ser comunicadas!
					if(i == 1)
					{
						qtdLinhas = divi +2;//Se for o primeiro bloco de linhas envia soh 2 linhas bonus!
						JobInit = 0; //Indica as linhas que serao filtradas
						JobEnd = divi;
						
					}
					else if(i == numtask-1)
					{
						qtdLinhas = divi+resto+2;//Se for a ultima, envia 2 linhas bonus + resto da divisao!
						JobInit = 2;
						JobEnd = qtdLinhas;
	
						linhaAPassar = linhaAPassar-4;
						
					}
					else
					{
						qtdLinhas = divi+4;
						JobInit = 2;
						JobEnd = qtdLinhas-2;
	
						linhaAPassar = linhaAPassar-4;
					}
	
					//printf("Rank = %d qtdLinhas = %d JobInit = %d JobEnd = %d linhaAPassar = %d \n",i , qtdLinhas, JobInit, JobEnd, linhaAPassar);
	
	
					//Envia quantide de linhas
					MPI_Send(&qtdLinhas, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
					//Envia quantas colunas por linha serao enviadas
					MPI_Send(&head.y, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
					//Envia onde comeca o processamento
					MPI_Send(&JobInit, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
					//Envia onde termina o processamento
					MPI_Send(&JobEnd, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
	
					for(k = 0; k<qtdLinhas; k++)
					{
						//printf("Linha = %d passada para Rank = %d\n",linhaAPassar, i);
						MPI_Send(&imgGray[linhaAPassar][0], head.y, MPI_INT, i, 4+k, MPI_COMM_WORLD);//Para enviar uma matriz, vc tem que enviar uma linha por vez!(Tem outras formas de fazer isso!!!)
						linhaAPassar++;
					}
	
				}

				for(i = 1;i < numtask; i++)
				{
					if(i != numtask-1)
						qtdLinhas = divi;
					else qtdLinhas = divi + resto;

					for(k = 0; k< qtdLinhas; k++)
					{
						MPI_Recv(imgGrayFinal[((i-1)*divi)+k], head.y, MPI_INT, i, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}

	
				clock_gettime(CLOCK_MONOTONIC, &clockEnd);
	
				printf("Tempo=> %fs\n", ((double)(clockEnd.tv_nsec - clockStart.tv_nsec)/1000000000) + (clockEnd.tv_sec - clockStart.tv_sec));
			
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
	
		}
		else
		{
			passarFiltroMPI();
		}
	}
	MPI_Finalize();
	return(0);
}

