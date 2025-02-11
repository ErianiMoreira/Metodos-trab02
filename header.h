#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#define MAX_ITERACOES 1000
#define TAM_TABU 10

//Alterar nos testes
const int MAX_NOS = 5;
const int HUBS = 3;
const int repeticoes = 1;
const char* arqEntrada = "inst5.txt";
const char* arqSolucao = "solucao5.txt";

// Constantes dos fatores de custo
const double BETA = 1.0;
const double ALPHA = 0.75;
const double LAMBDA = 1.0;

typedef struct {
    //int id;
    double x, y;
} No;

typedef struct {
    int hubs[HUBS];
    int caminhos[MAX_NOS][MAX_NOS];
    double custos[MAX_NOS][MAX_NOS];
    double objetivo;
} Solucao;

void calcularDistancia(const No* n1, const No* n2, double* distancia);
void calcularHubs(double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs, int* hubs);
void lerInstancia(const char* nomeArquivo, No nos[], int* numNos, int* numHubs);
void calcularCustos(No nos[], int numNos, double custos[MAX_NOS][MAX_NOS]);
void calcularSolucaoInicial(Solucao* solucao, No nos[], double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs);
void calcularFO(Solucao* solucao, double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs);
Solucao clonarSolucao(const Solucao* original, int numNos);
void escreverSolucao(Solucao solucao, int numNos, const char* nomeArquivo) ;
void lerSolucao(Solucao *solucao, int *numNos, const char* nomeArquivo);
void buscaTabu(Solucao* solucao, No nos[], int numNos, int numHubs);

#endif
