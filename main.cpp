#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <tuple>

#include "header.h"

using namespace std;

double custos[MAX_NOS][MAX_NOS];
#include <unordered_set>
unordered_set<int> listaTabu;  // Usa um conjunto para busca mais eficiente

int main() {
    No nos[MAX_NOS];
    int numNos, numHubs;
    clock_t h, f;
    Solucao solucao;

    lerInstancia(arqEntrada, nos, &numNos, &numHubs);
    calcularCustos(nos, numNos, custos);

    h = clock();
    calcularSolucaoInicial(&solucao, nos, custos, numNos, numHubs);
    h = clock() - h;
    double tempoSolucaoInicial = (double)h / CLOCKS_PER_SEC;

    f = clock();
    buscaTabu(&solucao, nos, numNos, numHubs);
    f = clock() - f;
    double tempoBuscaTabu = (double)f / CLOCKS_PER_SEC;

    printf("Funcao Objetivo (FO) apos Busca Tabu: %.2f\n", solucao.objetivo);
    printf("Tempo para gerar solucao inicial: %.20f segundos\n", tempoSolucaoInicial);
    printf("Tempo para Busca Tabu: %.20f segundos\n", tempoBuscaTabu);

    return 0;
}


void lerInstancia(const char* nomeArquivo, No nos[], int* numNos, int* numHubs) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }
    fscanf(arquivo, "%d", numNos);
    if (*numNos > MAX_NOS) {
        fprintf(stderr, "Erro: Número de nós excede o limite máximo de %d\n", MAX_NOS);
        exit(1);
    }
    for (int i = 0; i < *numNos; i++) {
        fscanf(arquivo, "%lf %lf", &nos[i].x, &nos[i].y);
        //nos[i].id = i;
    }
    *numHubs = HUBS;
    fclose(arquivo);
}

void calcularCustos(No nos[], int numNos, double custos[MAX_NOS][MAX_NOS]) {
    for (int i = 0; i < numNos; i++) {
        for (int j = i + 1; j < numNos; j++) {
            double distancia;
            calcularDistancia(&nos[i], &nos[j], &distancia);
            custos[i][j] = custos[j][i] = distancia;
        }
        custos[i][i] = 0.0;
    }
}


void calcularDistancia(const No* n1, const No* n2, double* distancia) {
    *distancia = sqrt(pow(n1->x - n2->x, 2) + pow(n1->y - n2->y, 2));
}

void calcularSolucaoInicial(Solucao* solucao, No nos[], double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs) {
    calcularHubs(custos, numNos, numHubs, solucao->hubs);
}



void calcularHubs(double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs, int* hubs) {
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> heap;

    for (int i = 0; i < numNos; i++) {
        double soma = 0.0;
        for (int j = 0; j < numNos; j++) {
            soma += custos[i][j];
        }
        heap.push({soma, i});  // Menor soma primeiro
    }

    for (int h = 0; h < numHubs; h++) {
        hubs[h] = heap.top().second;  // Pega o nó com menor soma de distâncias
        heap.pop();
    }
}



void calcularFO(Solucao* solucao, double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs) {
    solucao->objetivo = 0.0;

    for (int i = 0; i < numNos; i++) {
        for (int j = i + 1; j < numNos; j++) {
            double menorCusto = DBL_MAX;
            int melhorH1 = -1, melhorH2 = -1;

            for (int h1 = 0; h1 < numHubs; h1++) {
                double custoAteH1 = BETA * custos[i][solucao->hubs[h1]];
                for (int h2 = 0; h2 < numHubs; h2++) {
                    double custoTotal = custoAteH1 + ALPHA * custos[solucao->hubs[h1]][solucao->hubs[h2]] + LAMBDA * custos[solucao->hubs[h2]][j];

                    if (custoTotal < menorCusto) {
                        menorCusto = custoTotal;
                        melhorH1 = solucao->hubs[h1];
                        melhorH2 = solucao->hubs[h2];
                    }
                }
            }

            solucao->custos[i][j] = solucao->custos[j][i] = menorCusto;
            solucao->caminhos[i][j] = solucao->caminhos[j][i] = melhorH1 * 100 + melhorH2;
            solucao->objetivo = max(solucao->objetivo, menorCusto);
        }
    }
}




void buscaTabu(Solucao* solucao, No nos[], int numNos, int numHubs) {
    Solucao melhorSolucao = *solucao;
    Solucao solucaoAtual = *solucao;
    int iteracoes = 0;
    deque<int> ordemTabu;

    while (iteracoes < MAX_ITERACOES) {
        Solucao melhorVizinho = solucaoAtual; // Inicializa com a solução atual
        double melhorFO = DBL_MAX;
        int melhorTroca = -1;

        for (int i = 0; i < numHubs; i++) {
            for (int j = 0; j < numNos; j++) {
                if (listaTabu.count(j)) continue;

                Solucao novaSolucao = solucaoAtual;
                novaSolucao.hubs[i] = j;
                calcularFO(&novaSolucao, custos, numNos, numHubs);

                if (novaSolucao.objetivo < melhorFO) {
                    melhorFO = novaSolucao.objetivo;
                    melhorVizinho = novaSolucao;
                    melhorTroca = j;
                }
            }
        }

        if (melhorTroca == -1) break;

        solucaoAtual = melhorVizinho;
        listaTabu.insert(melhorTroca);
        ordemTabu.push_back(melhorTroca);

        if (ordemTabu.size() > TAM_TABU) {
            int remover = ordemTabu.front();
            ordemTabu.pop_front();
            listaTabu.erase(remover);
        }

        if (solucaoAtual.objetivo < melhorSolucao.objetivo) {
            melhorSolucao = solucaoAtual;
        }
        iteracoes++;
    }
    *solucao = melhorSolucao;
}


Solucao clonarSolucao(const Solucao* original) {
    return *original;
}

void escreverSolucao(Solucao solucao, int numNos, const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        fprintf(stderr, "Erro ao criar o arquivo de saída: %s\n", nomeArquivo);
        exit(1);
    }

    fprintf(arquivo, "FO: %.2f\n", solucao.objetivo);
    fprintf(arquivo, "HUBS: ");
    for (int h = 0; h < HUBS; h++) {
        fprintf(arquivo, "%d ", solucao.hubs[h]);
    }
    fprintf(arquivo, "\nOR H1 H2 DS CUSTO\n");
    for (int i = 0; i < numNos; i++) {
        for (int j = 0; j < numNos; j++) {
            int h1 = solucao.caminhos[i][j] / 100;
            int h2 = solucao.caminhos[i][j] % 100;
            fprintf(arquivo, "%d %d %d %d %.2f\n", i, h1, h2, j, solucao.custos[i][j]);
        }
    }

    fclose(arquivo);
   printf("Arquivo de solucao escrito com sucesso: %s\n", nomeArquivo);
}

void lerSolucao(Solucao *solucao, int *numNos, const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada: %s\n", nomeArquivo);
        exit(1);
    }
    fscanf(arquivo, "FO: %lf\n", &solucao->objetivo);
    fscanf(arquivo, "HUBS: ");
    for (int h = 0; h < HUBS; h++) {
        fscanf(arquivo, "%d ", &solucao->hubs[h]);
    }
    char linha[100];
    fgets(linha, sizeof(linha), arquivo);
    *numNos = 0;
    int i, j, h1, h2;
    double custo;
    while (fscanf(arquivo, "%d %d %d %d %lf\n", &i, &h1, &h2, &j, &custo) == 5) {
        solucao->caminhos[i][j] = h1 * 100 + h2;
        solucao->custos[i][j] = custo;
        if (i >= *numNos) *numNos = i + 1;
        if (j >= *numNos) *numNos = j + 1;
    }

    fclose(arquivo);
    printf("Arquivo de solucao lido com sucesso: %s\n", nomeArquivo);
}

