#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <unordered_map>

#define MAX_TABU_SIZE 100


#include "header.h"
using namespace std;


double custos[MAX_NOS][MAX_NOS];

int main() {

    //Alterar nos testes
    const char* arqEntrada = "inst20.txt";
    const char* arqSolucao = "solucao20.txt";
    //

    No nos[MAX_NOS];
    int numNos, numHubs;
    Solucao solucao;
    Solucao solucaoInicial;
    lerInstancia(arqEntrada, nos, &numNos, &numHubs);
    calcularCustos(nos, numNos, custos);
    clock_t inicio = clock();
    calcularSolucaoInicial(&solucao, nos, custos, numNos, numHubs);
    calcularFO(solucao, custos, numNos, numHubs);
    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Funcao Objetivo (FO): %.2f\n", solucao.objetivo);
    buscaTabu(solucaoInicial, nos, custos, numNos, numHubs);
    printf("FO após Busca Tabu: %.2f\n", solucaoInicial.objetivo);

    escreverSolucao(solucao, numNos, arqSolucao);
    //liberarMemoria(&solucao, numNos);
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
        for (int j = i; j < numNos; j++) {
            calcularDistancia(&nos[i], &nos[j], &custos[i][j]);
        }
    }
}

void calcularDistancia(const No* n1, const No* n2, double* distancia) {
    *distancia = sqrt(pow(n1->x - n2->x, 2) + pow(n1->y - n2->y, 2));
}

void calcularSolucaoInicial(Solucao* solucao, No nos[], double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs) {
    calcularHubs(custos, numNos, numHubs, solucao->hubs);
}

void calcularHubs(double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs, int* hubs) {
    double somaDistancias[MAX_NOS]; // Array estático
    for (int i = 0; i < numNos; i++) {
        somaDistancias[i] = 0.0;
        for (int j = 0; j < numNos; j++) {
            somaDistancias[i] += custos[i][j];
        }

    }
    for (int h = 0; h < numHubs; h++) {
        double minSoma = DBL_MAX;
        int minIndex = -1;
        for (int i = 0; i < numNos; i++) {
            if (somaDistancias[i] < minSoma) {
                minSoma = somaDistancias[i];
                minIndex = i;
            }
        }
        hubs[h] = minIndex;
        somaDistancias[minIndex] = DBL_MAX; // Marca o nó como "já selecionado"
    }
}


void calcularFO(Solucao& solucao, double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs) {
    solucao.objetivo = 0.0;
    double custosEntreHubs[MAX_NOS][MAX_NOS];
    for (int h1 = 0; h1 < numHubs; h1++) {
        for (int h2 = 0; h2 < numHubs; h2++) {
            custosEntreHubs[h1][h2] = ALPHA * custos[solucao.hubs[h1]][solucao.hubs[h2]];
        }
    }
    for (int i = 0; i < numNos; i++) {
        for (int j = 0; j < numNos; j++) {
            double menorCusto = DBL_MAX;
            int melhorCaminho = -1;
            for (int h1 = 0; h1 < numHubs; h1++) {
                double custoAteH1 = BETA * custos[i][solucao.hubs[h1]];
                for (int h2 = 0; h2 < numHubs; h2++) {
                    double custoTotal = custoAteH1 + custosEntreHubs[h1][h2] + LAMBDA * custos[solucao.hubs[h2]][j];
                    if (custoTotal < menorCusto) {
                        menorCusto = custoTotal;
                        melhorCaminho = solucao.hubs[h1] * 100 + solucao.hubs[h2];
                    }
                }
            }
            solucao.custos[i][j] = menorCusto;
            solucao.caminhos[i][j] = melhorCaminho;
            if (menorCusto > solucao.objetivo) {
                solucao.objetivo = menorCusto;
            }
        }
    }
}


Solucao clonarSolucao(const Solucao* original, int numNos) {
    Solucao copia= *original;
    return copia;
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

bool VerificarListaTabu(const Movimento& movimento, const std::vector<Movimento>& tabuList) {
    for (const auto& tabu : tabuList) {
        if ((tabu.hub1 == movimento.hub1 && tabu.hub2 == movimento.hub2) ||
            (tabu.hub1 == movimento.hub2 && tabu.hub2 == movimento.hub1)) {
            return true;
        }
    }
    return false;
}


void buscaTabu(Solucao& solucaoInicial, const No nos[],  double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs) {
    Solucao melhorSolucao = solucaoInicial;
    Solucao solucaoAtual = solucaoInicial;

    std::vector<Movimento> tabuList;
    std::vector<Solucao> solucoesVizinhas;

    Solucao melhorVizinho;
    Movimento movimentoSelecionado;
    bool encontrouVizinho;
    int Interacoes_Max = 50;

    for (int iter = 0; iter < Interacoes_Max; ++iter) {
        solucoesVizinhas.clear();
        gerarSolucoesVizinhas(solucaoAtual, nos, custos, numNos, numHubs, solucoesVizinhas);

        //encontrouVizinho = false; (para na 1 interação)

        // Encontrar o melhor vizinho que não está na lista tabu
        for (const auto& vizinho : solucoesVizinhas) {
            Movimento movimento;
            movimento.hub1 = -1;
            movimento.hub2 = -1;

            for (int h = 0; h < numHubs; h++) {
                if (vizinho.hubs[h] != solucaoAtual.hubs[h]) {
                    movimento.hub1 = solucaoAtual.hubs[h];
                    movimento.hub2 = vizinho.hubs[h];
                    break;
                }
            }

            if (!VerificarListaTabu(movimento, tabuList) && (!encontrouVizinho || vizinho.objetivo < melhorVizinho.objetivo)) {
                melhorVizinho = vizinho;
                encontrouVizinho = true;
                movimentoSelecionado = movimento;
            }
        }

        // Se não encontrou vizinho válido, encerra a busca
        if (!encontrouVizinho) {
            break;
        }

        // Atualiza a solução atual
        solucaoAtual = melhorVizinho;

        // Atualiza a melhor solução encontrada
        if (solucaoAtual.objetivo < melhorSolucao.objetivo) {
            melhorSolucao = solucaoAtual;
        }

        // Atualiza a lista tabu
        tabuList.push_back(movimentoSelecionado);
        if (tabuList.size() > MAX_TABU_SIZE) {
            tabuList.erase(tabuList.begin());
        }

        // Debug: imprimir progresso
        printf("Iteracao %d: FO = %.2f\n", iter, solucaoAtual.objetivo);
    }

    // Copiar a melhor solução de volta para o ponteiro de entrada
    solucaoInicial = melhorSolucao;
}


void gerarSolucoesVizinhas(const Solucao& solucaoAtual, const No nos[],  double custos[MAX_NOS][MAX_NOS], int numNos, int numHubs, std::vector<Solucao>& solucoesVizinhas) {
    Solucao vizinha;

    // Copia a solução atual para criar vizinhos
    for (int i = 0; i < numHubs; i++) {
        for (int j = 0; j < numHubs; j++) {
            if (i != j) {
                vizinha = solucaoAtual;

                // Tentar trocar dois hubs
                std::swap(vizinha.hubs[i], vizinha.hubs[j]);

                // Recalcular os custos e caminhos para a nova configuração
                calcularFO(vizinha, custos, numNos, numHubs);

                // Adicionar a solução vizinha gerada à lista
                solucoesVizinhas.push_back(vizinha);
            }
        }
    }

    // Alterar a alocação de nós aleatoriamente para gerar novas vizinhas
    for (int i = 0; i < numNos; i++) {
        for (int h = 0; h < numHubs; h++) {
            vizinha = solucaoAtual;

            // Alterar o caminho de um nó para outro hub
            vizinha.caminhos[i][h] = solucaoAtual.hubs[(h + 1) % numHubs];

            // Recalcular custos
            calcularFO(vizinha, custos, numNos, numHubs);

            // Adicionar a nova solução vizinha
            solucoesVizinhas.push_back(vizinha);
        }
    }


}


