/*
 Detective Quest - Sistema de pistas
 Implementação em C conforme requisitos.

 Estruturas:
 - Árvore binária de cômodos (Room)
 - Árvore binária de busca (BST) para pistas (ClueNode)
 - Tabela hash simples para mapear pista -> suspeito

 Funções documentadas conforme solicitado.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 101
#define MAX_INPUT 256

/* ----------------------------- Estruturas ----------------------------- */

// Nó da árvore de cômodos
typedef struct Room {
    char *name;
    struct Room *left;
    struct Room *right;
} Room;

// Nó da BST de pistas
typedef struct ClueNode {
    char *clue;
    struct ClueNode *left;
    struct ClueNode *right;
} ClueNode;

// Entrada da tabela hash (cadeia simples)
typedef struct HashEntry {
    char *key;       // pista
    char *suspect;   // suspeito associado
    struct HashEntry *next;
} HashEntry;

/* ----------------------------- Auxiliares ----------------------------- */

// Duplicador seguro de string
char *strdup_safe(const char *s) {
    if (!s) return NULL;
    char *dup = malloc(strlen(s) + 1);
    if (!dup) { perror("malloc"); exit(EXIT_FAILURE); }
    strcpy(dup, s);
    return dup;
}

// Trim newline de fgets
void trim_newline(char *s) {
    if (!s) return;
    size_t L = strlen(s);
    if (L>0 && s[L-1]=='\n') s[L-1] = '\0';
}

// Lowercase a string (modifica in-place)
void str_tolower_inplace(char *s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

/* ----------------------------- Funções Requeridas ----------------------------- */

/**
 * criarSala()
 * Cria dinamicamente um cômodo com o nome fornecido.
 * Retorna um ponteiro para Room alocado.
 */
Room *criarSala(const char *name) {
    Room *r = (Room*)malloc(sizeof(Room));
    if (!r) { perror("malloc"); exit(EXIT_FAILURE); }
    r->name = strdup_safe(name);
    r->left = r->right = NULL;
    return r;
}

/**
 * inserirPista()
 * Insere uma pista na árvore BST de pistas em ordem lexicográfica.
 * Evita inserção duplicada (se já existe, não insere).
 * Retorna o nó raiz (possivelmente atualizado).
 */
ClueNode *inserirPista(ClueNode *root, const char *clue) {
    if (!clue) return root;
    if (!root) {
        ClueNode *n = (ClueNode*)malloc(sizeof(ClueNode));
        if (!n) { perror("malloc"); exit(EXIT_FAILURE); }
        n->clue = strdup_safe(clue);
        n->left = n->right = NULL;
        return n;
    }
    int cmp = strcmp(clue, root->clue);
    if (cmp == 0) return root; // já existe
    if (cmp < 0) root->left = inserirPista(root->left, clue);
    else root->right = inserirPista(root->right, clue);
    return root;
}

/**
 * adicionarPista()
 * Função wrapper que registra a pista (chama inserirPista) e informa o jogador.
 */
ClueNode *adicionarPista(ClueNode *root, const char *clue) {
    if (!clue) return root;
    printf("\n> Pista encontrada: \"%s\"\n", clue);
    root = inserirPista(root, clue);
    printf("Pista adicionada ao caderno do jogador.\n\n");
    return root;
}

/**
 * inserirNaHash()
 * Insere uma associação pista -> suspeito na tabela hash.
 * Não insere duplicatas de chave (substitui se já existir).
 */
unsigned int hash_func(const char *s) {
    unsigned long h = 5381;
    while (*s) h = ((h << 5) + h) + (unsigned char)(*s++);
    return (unsigned int)(h % HASH_SIZE);
}

void inserirNaHash(HashEntry *table[], const char *pista, const char *suspeito) {
    if (!pista || !suspeito) return;
    unsigned int idx = hash_func(pista);
    HashEntry *cur = table[idx];
    while (cur) {
        if (strcmp(cur->key, pista) == 0) {
            // substitui o suspeito existente
            free(cur->suspect);
            cur->suspect = strdup_safe(suspeito);
            return;
        }
        cur = cur->next;
    }
    // novo entry
    HashEntry *e = (HashEntry*)malloc(sizeof(HashEntry));
    if (!e) { perror("malloc"); exit(EXIT_FAILURE); }
    e->key = strdup_safe(pista);
    e->suspect = strdup_safe(suspeito);
    e->next = table[idx];
    table[idx] = e;
}

/**
 * encontrarSuspeito()
 * Consulta a tabela hash para encontrar o suspeito associado a uma pista.
 * Retorna NULL se não encontrado.
 */
char *encontrarSuspeito(HashEntry *table[], const char *pista) {
    if (!pista) return NULL;
    unsigned int idx = hash_func(pista);
    HashEntry *cur = table[idx];
    while (cur) {
        if (strcmp(cur->key, pista) == 0) return cur->suspect;
        cur = cur->next;
    }
    return NULL;
}

/**
 * explorarSalas()
 * Navega pela árvore de cômodos de forma interativa.
 * Ao visitar um cômodo, identifica a pista associada (se houver) e a coleta automaticamente.
 */
void explorarSalas(Room *root, ClueNode **collected, HashEntry *table[]) {
    Room *atual = root;
    char input[MAX_INPUT];

    printf("\n--- Início da exploração da mansão ---\n");
    while (atual) {
        printf("Você está na sala: %s\n", atual->name);
        // identificar pista associada
        // lógica de pistas: função getClueForRoom
        const char *clue = NULL;
        // Definimos pistas estáticas por nome
        // (a lógica poderia ser mais complexa, mas está hard-coded conforme requisição)
        if (strcmp(atual->name, "Entrada") == 0) clue = "Pegadas lamacentas";
        else if (strcmp(atual->name, "Salão") == 0) clue = "Vidro quebrado";
        else if (strcmp(atual->name, "Cozinha") == 0) clue = "Faca com impressões";
        else if (strcmp(atual->name, "Biblioteca") == 0) clue = "Livro deslocado";
        else if (strcmp(atual->name, "Escritório") == 0) clue = "Carta rasgada";
        else if (strcmp(atual->name, "Quarto") == 0) clue = "Frascos vazios";
        else if (strcmp(atual->name, "Varanda") == 0) clue = "Fibra vermelha";
        else if (strcmp(atual->name, "Sótão") == 0) clue = "Marcas de arraste";
        else if (strcmp(atual->name, "Porão") == 0) clue = "Pegada pequena";
        else clue = NULL; // sala sem pista específica

        if (clue) *collected = adicionarPista(*collected, clue);
        else printf("Não há pistas aparentes nesta sala.\n\n");

        // controle de navegação
        printf("Escolha: (e) esquerdo, (d) direito, (s) sair da exploração\n");
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        trim_newline(input);
        if (strlen(input) == 0) continue;
        char c = input[0];
        if (c == 'e' || c == 'E') {
            if (atual->left) atual = atual->left;
            else printf("Não há sala à esquerda.\n\n");
        } else if (c == 'd' || c == 'D') {
            if (atual->right) atual = atual->right;
            else printf("Não há sala à direita.\n\n");
        } else if (c == 's' || c == 'S') {
            printf("Saindo da exploração...\n");
            break;
        } else {
            printf("Opção inválida. Use e, d ou s.\n\n");
        }
    }
    printf("--- Fim da exploração ---\n\n");
}

/**
 * verificarSuspeitoFinal()
 * Conduz a fase de julgamento final: percorre as pistas coletadas e verifica
 * quantas apontam para o suspeito acusado. Exibe o veredito com base na regra
 * de pelo menos duas pistas apontando para o mesmo suspeito.
 */

// Helper: percorre BST em-ordem e conta quantas pistas correspondem ao suspeito
int count_clues_for_suspect(ClueNode *root, HashEntry *table[], const char *suspect) {
    if (!root) return 0;
    int cnt = 0;
    cnt += count_clues_for_suspect(root->left, table, suspect);
    char *s = encontrarSuspeito(table, root->clue);
    if (s && strcmp(s, suspect) == 0) cnt++;
    cnt += count_clues_for_suspect(root->right, table, suspect);
    return cnt;
}

void listarPistas(ClueNode *root) {
    if (!root) return;
    listarPistas(root->left);
    printf(" - %s\n", root->clue);
    listarPistas(root->right);
}

void verificarSuspeitoFinal(ClueNode *collected, HashEntry *table[]) {
    if (!collected) {
        printf("Nenhuma pista foi coletada. Não é possível acusar com fundamento.\n");
        return;
    }
    printf("Pistas coletadas:\n");
    listarPistas(collected);

    char accused[MAX_INPUT];
    printf("\nDigite o nome do suspeito que você deseja acusar: ");
    if (!fgets(accused, sizeof(accused), stdin)) return;
    trim_newline(accused);
    if (strlen(accused) == 0) {
        printf("Nenhum suspeito informado. Encerrando julgamento.\n");
        return;
    }
    // Normalizar a entrada (case-insensitive matching) - assumimos nomes na tabela com mesma capitalização
    // Contamos pistas que apontam para esse suspeito
    int count = count_clues_for_suspect(collected, table, accused);
    printf("\nVocê acusou: %s\n", accused);
    printf("Pistas que apontam para %s: %d\n", accused, count);

    if (count >= 2) {
        printf("DESFECHO: Acusação válida! Há provas suficientes para sustentar o caso.\n");
    } else {
        printf("DESFECHO: Acusação fraca. Pelo menos 2 pistas são necessárias para condenar.\n");
    }
}

/* ----------------------------- Construção do cenário ----------------------------- */

// Constrói a mansão (árvore de cômodos) - exemplo balanceado simples
Room *construirMansao() {
    Room *rEntrada = criarSala("Entrada");
    Room *rSala = criarSala("Salão");
    Room *rCozinha = criarSala("Cozinha");
    Room *rBiblioteca = criarSala("Biblioteca");
    Room *rEscritorio = criarSala("Escritório");
    Room *rQuarto = criarSala("Quarto");
    Room *rVaranda = criarSala("Varanda");
    Room *rSotao = criarSala("Sótão");
    Room *rPorão = criarSala("Porão");

    // montar conexões (árvore binária)
    rEntrada->left = rSala;    rEntrada->right = rCozinha;
    rSala->left = rBiblioteca; rSala->right = rEscritorio;
    rCozinha->left = rQuarto;  rCozinha->right = rVaranda;
    rBiblioteca->left = rSotao; // para tornar a árvore mais extensa
    rEscritorio->right = rPorão;

    return rEntrada;
}

// Inicializa a tabela hash com associações pista -> suspeito
void popularTabelaHash(HashEntry *table[]) {
    // limpar
    for (int i=0;i<HASH_SIZE;i++) table[i] = NULL;

    inserirNaHash(table, "Pegadas lamacentas", "Sr. Verde");
    inserirNaHash(table, "Vidro quebrado", "Sra. Rosa");
    inserirNaHash(table, "Faca com impressões", "Sr. Preto");
    inserirNaHash(table, "Livro deslocado", "Sra. Rosa");
    inserirNaHash(table, "Carta rasgada", "Sr. Preto");
    inserirNaHash(table, "Frascos vazios", "Dr. Azul");
    inserirNaHash(table, "Fibra vermelha", "Sra. Rosa");
    inserirNaHash(table, "Marcas de arraste", "Sr. Verde");
    inserirNaHash(table, "Pegada pequena", "Sra. Rosa");
}

/* ----------------------------- Limpeza de memória ----------------------------- */

void freeRooms(Room *r) {
    if (!r) return;
    freeRooms(r->left);
    freeRooms(r->right);
    free(r->name);
    free(r);
}

void freeClues(ClueNode *n) {
    if (!n) return;
    freeClues(n->left);
    freeClues(n->right);
    free(n->clue);
    free(n);
}

void freeHash(HashEntry *table[]) {
    for (int i=0;i<HASH_SIZE;i++) {
        HashEntry *cur = table[i];
        while (cur) {
            HashEntry *tmp = cur;
            cur = cur->next;
            free(tmp->key);
            free(tmp->suspect);
            free(tmp);
        }
        table[i] = NULL;
    }
}

/* ----------------------------- main ----------------------------- */
int main(void) {
    // preparar
    Room *mansao = construirMansao();
    HashEntry *table[HASH_SIZE];
    popularTabelaHash(table);

    ClueNode *collected = NULL;

    // explorar salas
    explorarSalas(mansao, &collected, table);

    // fase de julgamento
    verificarSuspeitoFinal(collected, table);

    // limpeza
    freeRooms(mansao);
    freeClues(collected);
    freeHash(table);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
