#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <cassert>
#include <set>
#include <chrono>
#include <climits>

using namespace std;

vector<pair<double, double>> coordenadas; // Vetor de coordenadas dos vértices
vector<vector<int>> matrizDeDistancias; // Matriz de distâncias entre os vértices
vector<int> rota; // Vetor representando o ciclo Hamiltoniano

// Calcula a distância euclidiana entre os pontos
void calculaDistanciaEuc() {
    int n = coordenadas.size();
    matrizDeDistancias.resize(n, vector<int>(n));
    
    for(int i = 0; i < n; i++) {		
        for(int j = 0; j < n; j++) {
           matrizDeDistancias[i][j] = round(sqrt(pow(coordenadas[j].first - coordenadas[i].first, 2) + pow(coordenadas[j].second - coordenadas[i].second, 2)));                                    
        }   
    }
}

// Calcula a distância geográfica entre os pontos
void calculaDistanciaGeo() {
    const double RADIUS = 6371.0;

    int n = coordenadas.size();
    matrizDeDistancias.resize(n, vector<int>(n));

    for (int i = 0; i < n; i++) {
        double lat1 = coordenadas[i].first * M_PI / 180.0;
        double lon1 = coordenadas[i].second * M_PI / 180.0; 

        for (int j = 0; j < n; j++) {
            if (i == j) {
                matrizDeDistancias[i][j] = 0;
                continue;
            }

            double lat2 = coordenadas[j].first * M_PI / 180.0; 
            double lon2 = coordenadas[j].second * M_PI / 180.0; 

            double dLat = lat2 - lat1;
            double dLon = lon2 - lon1;

            double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
            double c = 2 * atan2(sqrt(a), sqrt(1 - a));

            int distancia = round(RADIUS * c); 
            matrizDeDistancias[i][j] = distancia;
        }
    }
}

// Lê um arquivo TSP no formato padrão
void leArquivoEntrada(string arquivoEntrada){
    ifstream arquivo(arquivoEntrada);
    int dimension;
    string linha, edgeWeightType;
	
    if(arquivo.is_open()) {
        while (getline(arquivo, linha)) {
			
            if (linha.find("DIMENSION") != string::npos) {
                dimension = stoi(linha.substr(linha.find(":") + 1));
            }
			
            else if(linha.find("EDGE_WEIGHT_TYPE") != string::npos) {
                edgeWeightType = linha.substr(linha.find(":") + 1);
                edgeWeightType.erase(0, edgeWeightType.find_first_not_of(" \t"));
            }
            
            else if(linha.find("NODE_COORD_SECTION") != string::npos) {
                coordenadas.resize(dimension);
				
                while(getline(arquivo,linha)) {
                    if (linha.find("EOF") != string::npos) 
                        break; 
						
                    stringstream ss(linha);
                    int index;
                    double x, y;
					
                    ss >> index >> x >> y;
                    coordenadas[index - 1] = {x, y};		
                }			
            }	
        }

        if (edgeWeightType == "EUC_2D")
            calculaDistanciaEuc();
        else if (edgeWeightType == "GEO")
            calculaDistanciaGeo();
        else 
            cout << "Tipo de peso de aresta nao suportado" << endl;

        arquivo.close();
	}

    else {
        cout << "Erro ao ler arquivo" << endl;
    }		
}

// Salva a rota encontrada em um arquivo
void salvaSolucaoEmArquivo(string arquivoSaida) {
    ofstream arquivo(arquivoSaida);

    if (arquivo) {
        int n = matrizDeDistancias.size();
  
        for (int i = 0; i < n; i++) {
            arquivo << rota[i];
            if (i < n - 1) {
                arquivo << " ";
            }
        }
    }
    else {
        cout << "Erro ao salvar solucao" << endl;
    }

    arquivo.close();
}

// Lê os dados de entrada do usuário via entrada padrão no console
void leEntradaPadrao() {
    int dimension;
    string edgeWeightType;
	
    cout << "Dimension: ";
    cin >> dimension;
    
    coordenadas.resize(dimension);

    cout << "Edge Weight Type: ";
    cin >> edgeWeightType;

    for(int i = 0; i < dimension; i++) {
        int index;
        double x, y;
        cin >> index >> x >> y;
        coordenadas[index - 1] = {x, y};
    }

    if (edgeWeightType == "EUC_2D") 
        calculaDistanciaEuc();
    else if (edgeWeightType == "GEO")
        calculaDistanciaGeo();
    else 
        cout << "Tipo de peso de aresta nao suportado" << endl;
}

// Imprime a rota encontrada no console
void imprimeSolucao() {
	int n = matrizDeDistancias.size(); 
    for (int i = 0; i < n; i++) {
        cout << rota[i];
        if (i < n - 1) 
            cout << " ";         
    }
}

// Encontra um limite superior para o problema usando Nearest Neighbor
int encontraLimiteSuperior() {
    int n = matrizDeDistancias.size(); 
    vector<bool> visitado(n, false);
    int atual = 0;
    visitado[atual] = true;

    int limiteSuperior = 0;

    for (int i = 1; i < n; i++) {
        int menorDistancia = INT_MAX;
        int proximo = -1;

        for (int j = 0; j < n; j++) {
            if (!visitado[j] and matrizDeDistancias[atual][j] < menorDistancia) {
                menorDistancia = matrizDeDistancias[atual][j];
                proximo = j;
            }
        }
        limiteSuperior = max(limiteSuperior, menorDistancia);

        atual = proximo;
        visitado[atual] = true;
    }

    limiteSuperior = max(limiteSuperior, matrizDeDistancias[atual][0]);

    return limiteSuperior;
} 

// Encontra um limite inferior para o problema 
int encontraLimiteInferior() {
    int n = matrizDeDistancias.size();
    int maxCusto = -INT_MAX;

    for (int i = 0; i < n; i++) {
        int menor = INT_MAX; 
        int segundoMenor = INT_MAX;

        for (int j = 0; j < n; j++) {
            if (i == j) 
                continue;

            int custo = matrizDeDistancias[i][j];

            if (custo < menor) {
                segundoMenor = menor;
                menor = custo;
            } 
            else if (custo < segundoMenor) {
                segundoMenor = custo;
            }
        }
        maxCusto = max(maxCusto, segundoMenor);
    }
    return maxCusto;
}

//  Calcula o custo total da ciclo
int calculaCustoTotal() {
    int distanciaTotal = 0;
    int n = matrizDeDistancias.size();
    for (int i = 0; i < n; i++) {
        distanciaTotal += matrizDeDistancias[i][rota[i]];
    }
    return distanciaTotal;
}

// Calcula o valor da maior aresta contida no ciclo
int calculaMaiorAresta() {
    int maiorAresta = 0;
    int n = matrizDeDistancias.size();
    for (int i = 0; i < n; i++) {
        int distanciaAtual = matrizDeDistancias[i][rota[i]];
        if (distanciaAtual > maiorAresta) {
            maiorAresta = distanciaAtual;
        }
    }
    return maiorAresta;
}

// Inverte um segmento da rota entre os índices 'inicio' e 'fim'.
void inverteSegmento(int inicio, int fim) {
    int atual = inicio;
    int proximo = rota[inicio];
    int nextNext;

    do {
        nextNext = rota[proximo];
        rota[proximo] = atual;

        atual = proximo;
        proximo = nextNext;
    } while (atual != fim);
}

// Valida se a rota é um ciclo Hamiltoniano válido
bool validaRota() {
    int n = matrizDeDistancias.size();
    vector<bool> visitados(n, false);
    int atual = 0; 

    for (int i = 0; i < n; i++) {
        if (visitados[atual]) return false;
        visitados[atual] = true; 
        atual = rota[atual];
    }

    return (atual == 0);
}

pair<int, int> criaParOrdenado(int x, int y) {
    if (x < y) 
        return make_pair(x, y);
    else 
        return make_pair(y, x);
}

// Algoritmo de Lin-Kernighan para otimizar o ciclo
void linKernighan(int inicio) {
    set<pair<int,int>> arestasRemovidas, arestasAdicionadas;
    pair<int, int> arestaRemovida;
    vector<int> novaRota = rota;
    int proximoVertice, proximoVerticeInicial, ultimoCadindatoProximoV;
    double melhorCusto, valorArestaRemovida, melhorGanhoLocal, ganhoLocal;
    double melhorGanho = 0;
    double ganho = 0;
    int lastproximoVertice = inicio;
    int fromV = rota[lastproximoVertice];

    int initialTourDistance = calculaCustoTotal();

    do {
        proximoVertice = -1;

        arestaRemovida = criaParOrdenado(lastproximoVertice, fromV);
        valorArestaRemovida = matrizDeDistancias[arestaRemovida.first][arestaRemovida.second];
	// Evita ciclos revisando arestas já modificadas
        if (arestasAdicionadas.count(arestaRemovida) > 0) 
            break;
        // Procura por trocas que ofereçam ganho positivo 
        for (int possibleproximoVertice = rota[fromV]; proximoVertice == -1 and possibleproximoVertice != inicio; possibleproximoVertice = rota[possibleproximoVertice]) {
            ganhoLocal = valorArestaRemovida - matrizDeDistancias[fromV][possibleproximoVertice];
		
            // Critérios para aceitar a troca
            if(!(
                arestasRemovidas.count(criaParOrdenado(fromV, possibleproximoVertice)) == 0 and
                ganho + ganhoLocal > 0 and
                arestasAdicionadas.count(criaParOrdenado(ultimoCadindatoProximoV, possibleproximoVertice)) == 0 and
                rota[possibleproximoVertice] != 0 and
                possibleproximoVertice != rota[fromV]
            )) {
                ultimoCadindatoProximoV = possibleproximoVertice;
                continue;
            }
            
            proximoVertice = possibleproximoVertice;
        }

        if (proximoVertice != -1) {
 	    // Atualiza estruturas de controle de arestas
            arestasRemovidas.insert(arestaRemovida);
            arestasAdicionadas.insert(criaParOrdenado(fromV, proximoVertice));
		
	    // Calcula o melhor ganho possível com esta troca
            melhorCusto = matrizDeDistancias[fromV][inicio];
            melhorGanhoLocal = ganho + (valorArestaRemovida - melhorCusto);
		
            // Atualiza a melhor solução encontrada
            if (melhorGanhoLocal > melhorGanho) {
                melhorGanho = melhorGanhoLocal;
                novaRota = rota;
                novaRota[inicio] = fromV;
            }

            ganho += valorArestaRemovida - matrizDeDistancias[fromV][proximoVertice];
		
	    // Inverte o segmento da rota para realizar a troca
            inverteSegmento(fromV, ultimoCadindatoProximoV);

            proximoVerticeInicial = ultimoCadindatoProximoV;

            rota[fromV] = proximoVertice;
      
            lastproximoVertice = proximoVertice;
            fromV = proximoVerticeInicial;
        }

    } while(proximoVertice != -1);

    rota = novaRota;
    int distanceAfter = calculaCustoTotal();
	
   // Garante que a otimização não aumentou o custo e que a rota é válida
    assert(distanceAfter <= initialTourDistance);
    assert (validaRota());
}

// Aplica o algoritmo de Lin-Kernighan várias vezes para melhorar a rota
void MelhoraRota(int numeroDeInteracoes) {
    int diferenca;
    int custoAnterior = 0;
    int custoAtual = 0;
    int n = matrizDeDistancias.size();

    for (int i = 0; i < numeroDeInteracoes; i++) {
        for (int j = 0; j < n; j++) {
            linKernighan(j);
        }
        custoAtual = calculaCustoTotal();
        diferenca = custoAnterior - custoAtual;

	// Para se não houver mais melhorias
        if (i != 0) {
            assert(diferenca >= 0);
            if (diferenca == 0 or custoAnterior == 0) 
                break;
        }

        custoAnterior = custoAtual;
    }
}

// Inicializa a rota como uma sequência simples 0->1->2->...->n->0 formando um ciclo
void inicializaRota() {   
    int n = matrizDeDistancias.size();
    rota = vector<int>(n, 0);

    for (int i = 0; i < n; i++) 
        rota[i] = (i + 1) % n;
}

// Minimiza a maior aresta presente na solução
// 1. Em cada iteração, define um valor intermediário (mid) utilizando limiteInferior e limiteSuperior e cria uma matriz auxiliar onde distâncias <= mid são consideradas 0
// 2. Chama MelhoraRota para verificar se é possível encontrar uma solução viável sem usar arestas maiores que mid
// 2. Se a solução encontrada não utilizar nenhuma aresta com valor maior que mid (custoTotal == 0), significa que mid ainda pode ser reduzido. Caso contrário, mid é aumentado
// 4. O processo se repete até que o menor limite superior viável seja encontrado
void minimizaMaiorAresta(int numeroDeInteracoes) {
    inicializaRota();

    int limiteInferior = encontraLimiteInferior();
    auto limiteSuperior = encontraLimiteSuperior();

    // Busca binária para encontrar o menor limite superior viável
    while(limiteInferior < limiteSuperior) {
        int mid = (limiteInferior + limiteSuperior) / 2;

        vector<vector<int>> aux = matrizDeDistancias;
        int n = aux.size();
	    
	// Cria uma matriz auxiliar onde distâncias <= mid são consideradas 0
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                if(aux[i][j] <= mid)
                    aux[i][j] = 0;
            }
        }

        swap(matrizDeDistancias, aux);
        MelhoraRota(numeroDeInteracoes);
        int custoTotal = calculaCustoTotal();
        swap(matrizDeDistancias, aux); // Restaura a matriz original

        if(custoTotal == 0) 
            limiteSuperior = mid; 
        else
            limiteInferior = mid + 1;
    }
}

int main() { 
    string arquivoEntrada, arquivoSaida;
    // Descomente para entrada via arquivo
    //cout << "Arquivo de entrada: ";
    //cin >> arquivoEntrada;
	
    cout << endl << "Arquivo de saida: ";
    cin >> arquivoSaida;

    int numeroDeInteracoes;
    cout << "Numero de interacoes do algoritmo (sugestao: 10): ";
    cin >> numeroDeInteracoes;

    //leArquivoEntrada(arquivoEntrada);
    leEntradaPadrao(); // Comente esta linha se for usar arquivo
    
    auto inicio = chrono::high_resolution_clock::now();

    minimizaMaiorAresta(numeroDeInteracoes);

    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double> duracao = (fim - inicio);

    salvaSolucaoEmArquivo(arquivoSaida);
    //imprimeSolucao();

    cout << "Maior aresta: "<< calculaMaiorAresta() << endl;
    //cout << "Tempo de execucao em segundos: "<< duracao.count() << endl;
     
    return 0;
}
