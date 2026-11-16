/* ======================================================================
 * ATIVIDADE AVALIATIVA 04 - CADASTRO DE LOJA
 * Algoritmos de ordenacao + manipulacao de arquivos + structs + ponteiros
 *
 * Compilar:  gcc src/cadastro_loja.c -o cadastro_loja   (ou: make)
 * Executar:  ./cadastro_loja      (Windows: cadastro_loja.exe)
 *
 * OBSERVACAO: os textos do programa e os dados iniciais foram escritos SEM
 * acentos (ex.: "Cafe", "Vestuario") de proposito. Em UTF-8 um acento ocupa
 * 2 bytes e desalinharia a tabela impressa no terminal.
 * ====================================================================== */

#include <stdio.h>   // printf, fopen, fgets, fprintf, remove, rename, getchar, feof
#include <stdlib.h>  // malloc, realloc, free, exit, strtol, strtof
#include <string.h>  // strlen, strcpy, strncpy, strstr, memmove, memcpy, strcspn, strchr
#include <ctype.h>   // tolower, isspace (case-insensitive e limpeza de texto)

/* ---------- CONSTANTES (#define evita "numeros magicos" espalhados) ---------- */
#define ARQ_DADOS  "produtos.txt"      // arquivo de texto com a tabela de dados
#define ARQ_TEMP   "produtos_temp.txt" // arquivo temporario usado ao inserir/alterar/excluir
#define ARQ_CSV    "produtos.csv"      // arquivo gerado na exportacao
#define TAM_NOME   50                  // tamanho maximo do nome (aumente se precisar)
#define TAM_CAT    30                  // tamanho maximo da categoria
#define TAM_FORN   50                  // tamanho maximo do fornecedor
#define TAM_LINHA  256                 // tamanho do buffer de leitura de linhas
#define PADRAO     "N/D"               // valor usado quando categoria/fornecedor ficam vazios
#define MAX_PRECO  1000000000.0f       // limite de seguranca para o preco (rejeita inf/absurdos)
#define MAX_INT    1000000000L         // limite de seguranca para inteiros (evita estouro de int)

/* ---------- ESTRUTURA DE DADOS ----------
 * struct agrupa os campos de um produto em um unico tipo. */
typedef struct {
    int   id;                   // identificador unico (gerado automaticamente)
    char  nome[TAM_NOME];       // nome do produto
    char  categoria[TAM_CAT];   // categoria do produto
    float preco;                // preco em reais
    int   quantidade;           // quantidade em estoque
    char  fornecedor[TAM_FORN]; // nome do fornecedor
} Produto;                      // "Produto" passa a ser o nome do tipo

/* ======================================================================
 * FUNCOES AUXILIARES DE TEXTO E ENTRADA
 * ====================================================================== */

/* Le uma linha do teclado com seguranca e remove o '\n' do final.
 * Se o usuario digitar MAIS do que cabe em 'destino', o excesso e descartado,
 * senao ele "vazaria" para a proxima leitura e desalinharia todas as perguntas. */
void lerLinha(const char *msg, char *destino, int tam) {
    printf("%s", msg);                          // mostra a pergunta ao usuario
    fflush(stdout);                             // garante que a pergunta aparece antes da leitura
    if (fgets(destino, tam, stdin) == NULL) {   // le ate tam-1 caracteres (nunca estoura o buffer)
        destino[0] = '\0';                      // em caso de erro/fim de entrada, devolve texto vazio
        return;                                 // sai da funcao
    }
    if (strchr(destino, '\n') == NULL) {        // sem '\n' = a linha era maior que o buffer
        int c;                                  // guarda cada caractere descartado
        while ((c = getchar()) != '\n' && c != EOF) { } // consome o resto da linha ate o ENTER
    }
    destino[strcspn(destino, "\r\n")] = '\0';   // troca '\n' (ou '\r\n' do Windows) por fim de string
}

/* Converte texto em inteiro validando. Retorna 1 se valido, 0 se invalido.
 * (atoi nao serve: "abc" viraria 0 sem avisar.) */
int converterInteiro(const char *txt, int *saida) {
    char *fim;                                  // aponta para onde a conversao parou
    long v = strtol(txt, &fim, 10);             // converte em base 10
    if (fim == txt) return 0;                   // nenhum digito foi lido: invalido
    while (isspace((unsigned char)*fim)) fim++; // ignora espacos apos o numero
    if (*fim != '\0') return 0;                 // sobrou lixo ("12abc"): invalido
    if (v < 0 || v > MAX_INT) return 0;         // negativo ou grande demais: invalido (evita estouro do int)
    *saida = (int)v;                            // grava o valor convertido (agora e seguro converter)
    return 1;                                   // sucesso
}

/* Converte texto em float validando. Aceita virgula ou ponto decimal. */
int converterFloat(const char *txt, float *saida) {
    char copia[TAM_LINHA];                      // copia para poder trocar a virgula
    strncpy(copia, txt, sizeof(copia) - 1);     // copia no maximo tamanho-1 caracteres
    copia[sizeof(copia) - 1] = '\0';            // garante o terminador
    for (char *p = copia; *p; p++)              // percorre com ponteiro
        if (*p == ',') *p = '.';                // virgula brasileira vira ponto
    char *fim;                                  // onde a conversao parou
    float v = strtof(copia, &fim);              // converte texto em float
    if (fim == copia) return 0;                 // nada convertido: invalido
    while (isspace((unsigned char)*fim)) fim++; // ignora espacos finais
    if (*fim != '\0') return 0;                 // lixo no final: invalido
    if (v != v) return 0;                       // v != v so e verdadeiro para NaN ("nan"): invalido
    if (v < 0 || v > MAX_PRECO) return 0;       // negativo, "inf" ou valor absurdo: invalido
    *saida = v;                                 // grava o valor
    return 1;                                   // sucesso
}

/* Pergunta e le um inteiro. Retorna -1 se a entrada nao for um inteiro valido
 * (assim "abc" no menu nunca e confundido com a opcao 0 = sair). */
int lerInteiro(const char *msg) {
    char buf[TAM_LINHA];                        // buffer da linha digitada
    int v;                                      // valor convertido
    lerLinha(msg, buf, sizeof(buf));            // le o texto
    if (!converterInteiro(buf, &v)) return -1;  // invalido -> -1
    return v;                                   // valido -> o numero
}

/* Pergunta e le um float. Retorna -1 se invalido. */
float lerFloat(const char *msg) {
    char buf[TAM_LINHA];                        // buffer da linha digitada
    float v;                                    // valor convertido
    lerLinha(msg, buf, sizeof(buf));            // le o texto
    if (!converterFloat(buf, &v)) return -1;    // invalido -> -1
    return v;                                   // valido -> o numero
}

/* Remove espacos no inicio e no fim do texto (ex.: "  Cafe  " -> "Cafe"). */
void aparar(char *s) {
    char *ini = s;                              // ponteiro que vai achar o 1o caractere util
    while (*ini == ' ' || *ini == '\t') ini++;  // pula espacos iniciais
    memmove(s, ini, strlen(ini) + 1);           // desloca o texto para o comeco (+1 leva o '\0')
    int n = (int)strlen(s);                     // tamanho atual do texto
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t')) // enquanto terminar em espaco
        s[--n] = '\0';                          // corta o espaco final
}

/* Troca '|' por '-' pois '|' e o separador do arquivo.
 * Se o usuario digitasse '|', o arquivo ficaria corrompido. */
void limparSeparador(char *s) {
    for (; *s; s++)                             // percorre cada caractere via ponteiro
        if (*s == '|') *s = '-';                // substitui o separador
}

/* Aplica limpeza padrao a um campo de texto digitado: aparar + proteger separador.
 * Se ficar vazio, usa PADRAO (evita campo vazio, que quebraria a leitura do arquivo). */
void normalizarCampo(char *s) {
    aparar(s);                                  // tira espacos das pontas
    limparSeparador(s);                         // protege o formato do arquivo
    if (s[0] == '\0') strcpy(s, PADRAO);        // vazio vira "N/D"
}

/* Converte a string inteira para minusculas (altera a string recebida). */
void paraMinusculo(char *s) {
    for (; *s; s++)                             // percorre cada caractere
        *s = (char)tolower((unsigned char)*s);  // (unsigned char evita comportamento indefinido)
}

/* Compara duas strings IGNORANDO maiusculas/minusculas.
 * Retorna <0, 0 ou >0 como strcmp. Usada na ordenacao e na duplicidade. */
int compararTexto(const char *a, const char *b) {
    while (*a && *b) {                          // enquanto nenhuma string terminou
        int ca = tolower((unsigned char)*a);    // caractere de 'a' em minuscula
        int cb = tolower((unsigned char)*b);    // caractere de 'b' em minuscula
        if (ca != cb) return ca - cb;           // primeira diferenca decide o resultado
        a++; b++;                               // avanca os dois ponteiros
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b); // uma terminou: a menor vem antes
}

/* Verifica se 'texto' CONTEM 'trecho', sem diferenciar maiusculas.
 * Retorna 1 se contem, 0 caso contrario (busca parcial). */
int contemTexto(const char *texto, const char *trecho) {
    char t1[TAM_LINHA], t2[TAM_LINHA];          // copias, para nao alterar os originais
    strncpy(t1, texto, sizeof(t1) - 1);         // copia o texto
    t1[sizeof(t1) - 1] = '\0';                  // garante terminador
    strncpy(t2, trecho, sizeof(t2) - 1);        // copia o trecho procurado
    t2[sizeof(t2) - 1] = '\0';                  // garante terminador
    paraMinusculo(t1);                          // tudo minusculo
    paraMinusculo(t2);                          // tudo minusculo
    return strstr(t1, t2) != NULL;              // strstr procura a substring
}

/* ======================================================================
 * ARQUIVO E LISTA EM MEMORIA (vetor dinamico acessado por ponteiros)
 * ====================================================================== */

/* CRIAR: garante que o arquivo exista; se nao existir, cria com os 4
 * produtos iniciais da tabela do enunciado. */
void criarArquivo(void) {
    FILE *f = fopen(ARQ_DADOS, "r");            // tenta abrir para leitura
    if (f != NULL) {                            // abriu = o arquivo ja existe
        fclose(f);                              // fecha
        return;                                 // nada a fazer
    }
    f = fopen(ARQ_DADOS, "w");                  // cria o arquivo para escrita
    if (f == NULL) {                            // falhou (sem permissao, disco cheio...)
        printf("ERRO: nao foi possivel criar %s\n", ARQ_DADOS);
        exit(1);                                // encerra com codigo de erro
    }
    // Formato de cada linha: ID|Produto|Categoria|Preco|Quantidade|Fornecedor
    fprintf(f, "1|Camiseta Basica|Vestuario|49.90|30|Moda Brasil\n");   // produto 1
    fprintf(f, "2|Mouse Optico USB|Informatica|59.99|15|TecnoShop\n");  // produto 2
    fprintf(f, "3|Shampoo 300ml|Higiene Pessoal|22.50|40|Beleza Pura\n"); // produto 3
    fprintf(f, "4|Cafe 500g|Alimentos|18.00|25|Graos do Sul\n");        // produto 4
    fclose(f);                                  // fecha para gravar de fato
    printf("Arquivo %s criado com 4 produtos iniciais.\n", ARQ_DADOS);   // avisa o usuario
}

/* Le o arquivo inteiro para a memoria. Devolve o vetor (alocado com malloc)
 * e atualiza *n com a quantidade de produtos lidos. */
Produto *carregar(int *n) {
    *n = 0;                                     // comeca com 0 produtos
    int capacidade = 10;                        // capacidade inicial do vetor
    Produto *lista = malloc(capacidade * sizeof(Produto)); // reserva memoria
    if (lista == NULL) { printf("Sem memoria!\n"); exit(1); } // malloc pode falhar

    FILE *f = fopen(ARQ_DADOS, "r");            // abre para leitura
    if (f == NULL) return lista;                // sem arquivo: devolve lista vazia

    char linha[TAM_LINHA];                      // buffer para cada linha lida
    while (fgets(linha, sizeof(linha), f)) {    // le linha por linha ate o fim do arquivo
        linha[strcspn(linha, "\r\n")] = '\0';   // remove a quebra de linha
        if (*n == capacidade) {                 // vetor cheio?
            capacidade *= 2;                    // dobra a capacidade
            Produto *novo = realloc(lista, capacidade * sizeof(Produto)); // amplia o bloco
            if (novo == NULL) { free(lista); fclose(f); printf("Sem memoria!\n"); exit(1); }
            lista = novo;                       // passa a usar o bloco ampliado
        }
        Produto *p = &lista[*n];                // ponteiro para a proxima posicao livre
        // %[^|] le ate o proximo '|'; o numero limita o tamanho (evita estouro de buffer)
        int lidos = sscanf(linha, "%d|%49[^|]|%29[^|]|%f|%d|%49[^\n]",
                           &p->id, p->nome, p->categoria,
                           &p->preco, &p->quantidade, p->fornecedor);
        if (lidos == 6) (*n)++;                 // so aceita a linha se os 6 campos foram lidos
    }
    fclose(f);                                  // fecha o arquivo
    return lista;                               // devolve o vetor preenchido
}

/* Grava a lista no arquivo usando ARQUIVO TEMPORARIO (tecnica anti-corrupcao):
 * 1) escreve tudo em produtos_temp.txt; 2) apaga o original; 3) renomeia o temporario.
 * Se algo falhar durante o passo 1, o arquivo original continua intacto. */
int salvarComTemporario(Produto *lista, int n) {
    FILE *tmp = fopen(ARQ_TEMP, "w");           // cria o arquivo temporario
    if (tmp == NULL) { printf("ERRO ao criar arquivo temporario.\n"); return 0; }
    for (int i = 0; i < n; i++) {               // percorre todos os produtos
        Produto *p = &lista[i];                 // ponteiro para o produto atual
        fprintf(tmp, "%d|%s|%s|%.2f|%d|%s\n",   // uma linha por produto, separada por '|'
                p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
    }
    if (fclose(tmp) != 0) {                     // fclose devolve erro se a gravacao falhou
        printf("ERRO ao gravar arquivo temporario.\n");
        remove(ARQ_TEMP);                       // descarta o temporario ruim
        return 0;                               // original preservado
    }
    remove(ARQ_DADOS);                          // apaga o arquivo original
    if (rename(ARQ_TEMP, ARQ_DADOS) != 0) {     // o temporario passa a ser o original
        printf("ERRO ao substituir o arquivo original. Seus dados estao em %s\n", ARQ_TEMP);
        return 0;                               // falha (os dados continuam salvos no temporario)
    }
    return 1;                                   // sucesso
}

/* Gera o proximo ID automaticamente: maior ID existente + 1. */
int gerarId(Produto *lista, int n) {
    int maior = 0;                              // lista vazia => primeiro ID sera 1
    for (int i = 0; i < n; i++)                 // percorre todos os produtos
        if (lista[i].id > maior) maior = lista[i].id; // guarda o maior ID visto
    return maior + 1;                           // proximo ID livre
}

/* Procura produto pelo ID. Retorna o indice no vetor ou -1 se nao existir.
 * Tambem serve para a VERIFICACAO DE ID DUPLICADO na insercao. */
int indiceDoId(Produto *lista, int n, int id) {
    for (int i = 0; i < n; i++)                 // percorre o vetor
        if (lista[i].id == id) return i;        // achou: devolve a posicao
    return -1;                                  // nao achou
}

/* Procura produto pelo nome EXATO (ignorando maiusculas). Retorna o indice
 * ou -1. 'ignorarId' permite ignorar o proprio produto ao alterar
 * (use 0 para nao ignorar nenhum). */
int indiceDoNome(Produto *lista, int n, const char *nome, int ignorarId) {
    for (int i = 0; i < n; i++) {               // percorre o vetor
        if (lista[i].id != ignorarId && compararTexto(lista[i].nome, nome) == 0)
            return i;                           // nome igual em outro produto: duplicata
    }
    return -1;                                  // nenhuma duplicata
}

/* ======================================================================
 * EXIBICAO EM FORMATO DE TABELA
 * ====================================================================== */

/* Imprime o topo da tabela: linha tracejada, titulos das colunas e outra linha. */
void cabecalhoTabela(void) {
    printf("\n--------------------------------------------------------------------------------------\n"); // linha superior
    printf("| %-4s | %-20s | %-15s | %10s | %5s | %-14s |\n",
           "ID", "Produto", "Categoria", "Preco (R$)", "Qtd", "Fornecedor"); // titulos das colunas
    printf("--------------------------------------------------------------------------------------\n");   // separa titulo e dados
}

/* Imprime UM produto como uma linha da tabela. */
void linhaTabela(const Produto *p) {
    // %-20.20s alinha a esquerda e CORTA em 20 caracteres, para nao quebrar a tabela
    printf("| %-4d | %-20.20s | %-15.15s | %10.2f | %5d | %-14.14s |\n",
           p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
}

/* Imprime a linha tracejada que fecha a tabela. */
void rodapeTabela(void) {
    printf("--------------------------------------------------------------------------------------\n");   // linha inferior
}

/* Mostra um vetor de produtos em tabela (nao altera o arquivo). */
void mostrarTabela(Produto *lista, int n) {
    if (n == 0) { printf("\nNenhum produto para exibir.\n"); return; } // lista vazia
    cabecalhoTabela();                          // imprime o topo
    for (int i = 0; i < n; i++)                 // uma linha por produto
        linhaTabela(&lista[i]);
    rodapeTabela();                             // imprime a base
    printf("Total: %d produto(s)\n", n);        // contagem
}

/* ======================================================================
 * OPERACOES DO CRUD
 * ====================================================================== */

/* INSERIR: pede os dados, valida, checa duplicata e gera ID automatico.
 * Recebe Produto ** e int * para poder alterar o vetor e o contador do main. */
void inserir(Produto **lista, int *n) {
    Produto novo;                               // produto temporario com os dados digitados

    lerLinha("Nome: ", novo.nome, TAM_NOME);    // le o nome
    aparar(novo.nome);                          // tira espacos das pontas
    limparSeparador(novo.nome);                 // protege o separador do arquivo
    if (novo.nome[0] == '\0') {                 // nome vazio nao e permitido
        printf("Nome nao pode ser vazio.\n");
        return;
    }

    // VERIFICACAO DE DUPLICATA: mesmo nome (ignorando maiusculas) nao e cadastrado de novo
    if (indiceDoNome(*lista, *n, novo.nome, 0) != -1) {
        printf("ERRO: ja existe um produto com o nome \"%s\".\n", novo.nome);
        return;
    }

    lerLinha("Categoria: ", novo.categoria, TAM_CAT); // le a categoria
    normalizarCampo(novo.categoria);            // limpa; vazio vira "N/D"

    novo.preco = lerFloat("Preco (R$): ");      // le o preco (-1 se invalido)
    if (novo.preco < 0) { printf("Preco invalido. Use um numero maior ou igual a 0.\n"); return; }

    novo.quantidade = lerInteiro("Quantidade: "); // le a quantidade (-1 se invalida)
    if (novo.quantidade < 0) { printf("Quantidade invalida. Use um inteiro maior ou igual a 0.\n"); return; }

    lerLinha("Fornecedor: ", novo.fornecedor, TAM_FORN); // le o fornecedor
    normalizarCampo(novo.fornecedor);           // limpa; vazio vira "N/D"

    novo.id = gerarId(*lista, *n);              // ID incremental automatico (sem digitar)
    while (indiceDoId(*lista, *n, novo.id) != -1) // verificacao extra de ID duplicado
        novo.id++;                              // se por algum motivo existir, avanca

    Produto *tmp = realloc(*lista, (*n + 1) * sizeof(Produto)); // aumenta o vetor em 1 posicao
    if (tmp == NULL) { printf("Sem memoria!\n"); return; } // se falhar, o vetor antigo segue valido
    *lista = tmp;                               // atualiza o ponteiro do chamador
    (*lista)[*n] = novo;                        // copia o novo produto para o fim do vetor
    (*n)++;                                     // agora ha um produto a mais

    if (salvarComTemporario(*lista, *n))        // grava no arquivo
        printf("Produto cadastrado com sucesso! ID gerado: %d\n", novo.id);
}

/* ALTERAR: localiza pelo ID e permite trocar cada campo (ENTER mantem o atual).
 * Trabalha numa COPIA do produto e so aplica no final, para que um erro no meio
 * (ex.: preco invalido) nao deixe o produto parcialmente alterado. */
void alterar(Produto *lista, int n) {
    int id = lerInteiro("ID do produto a alterar: "); // pede o ID
    int i = indiceDoId(lista, n, id);           // procura o indice
    if (i == -1) { printf("Produto com ID %d nao encontrado.\n", id); return; }

    Produto *p = &lista[i];                     // ponteiro para o produto original
    Produto copia = *p;                         // copia de trabalho
    char buf[TAM_LINHA];                        // buffer de leitura
    printf("Alterando: %s (ENTER mantem o valor atual)\n", p->nome);

    lerLinha("Novo nome: ", buf, TAM_NOME);     // novo nome (opcional)
    aparar(buf); limparSeparador(buf);          // limpeza
    if (buf[0] != '\0') {                       // so altera se digitou algo
        if (indiceDoNome(lista, n, buf, p->id) != -1) { // duplicata com OUTRO produto?
            printf("ERRO: ja existe outro produto com esse nome.\n");
            return;                             // cancela a alteracao
        }
        strcpy(copia.nome, buf);                // aceita o novo nome
    }

    lerLinha("Nova categoria: ", buf, TAM_CAT); // nova categoria (opcional)
    aparar(buf); limparSeparador(buf);          // limpeza
    if (buf[0] != '\0') strcpy(copia.categoria, buf); // so troca se digitou algo

    lerLinha("Novo preco: ", buf, sizeof(buf)); // novo preco (opcional)
    aparar(buf);                                // limpeza
    if (buf[0] != '\0') {                       // digitou algo
        float v;                                // valor convertido
        if (!converterFloat(buf, &v)) { printf("Preco invalido. Alteracao cancelada.\n"); return; }
        copia.preco = v;                        // aceita o novo preco
    }

    lerLinha("Nova quantidade: ", buf, sizeof(buf)); // nova quantidade (opcional)
    aparar(buf);                                // limpeza
    if (buf[0] != '\0') {                       // digitou algo
        int q;                                  // valor convertido
        if (!converterInteiro(buf, &q)) { printf("Quantidade invalida. Alteracao cancelada.\n"); return; }
        copia.quantidade = q;                   // aceita a nova quantidade
    }

    lerLinha("Novo fornecedor: ", buf, TAM_FORN); // novo fornecedor (opcional)
    aparar(buf); limparSeparador(buf);          // limpeza
    if (buf[0] != '\0') strcpy(copia.fornecedor, buf); // so troca se digitou algo

    *p = copia;                                 // tudo valido: aplica as mudancas
    if (salvarComTemporario(lista, n))          // regrava o arquivo via temporario
        printf("Produto alterado com sucesso!\n");
}

/* EXCLUIR: remove do vetor deslocando os itens seguintes e regrava o arquivo. */
void excluir(Produto *lista, int *n) {
    int id = lerInteiro("ID do produto a excluir: "); // pede o ID
    int i = indiceDoId(lista, *n, id);          // procura o indice
    if (i == -1) { printf("Produto com ID %d nao encontrado.\n", id); return; }

    char r[TAM_LINHA];                          // resposta de confirmacao
    printf("Excluir \"%s\"?\n", lista[i].nome); // mostra qual sera excluido
    lerLinha("Confirma (s/n): ", r, sizeof(r)); // pede confirmacao
    if (tolower((unsigned char)r[0]) != 's') {  // qualquer coisa diferente de 's' cancela
        printf("Operacao cancelada.\n");
        return;
    }

    for (int j = i; j < *n - 1; j++)            // "puxa" os proximos uma posicao para tras
        lista[j] = lista[j + 1];
    (*n)--;                                     // um produto a menos

    if (salvarComTemporario(lista, *n))         // regrava o arquivo via temporario
        printf("Produto excluido com sucesso!\n");
}

/* LISTAR: mostra todos os produtos na ordem do arquivo. */
void listar(Produto *lista, int n) {
    mostrarTabela(lista, n);                    // reaproveita a funcao de tabela
}

/* BUSCAR por nome ou categoria (parcial e sem diferenciar maiusculas).
 * campo: 1 = nome, 2 = categoria. */
void buscar(Produto *lista, int n, int campo) {
    char termo[TAM_LINHA];                      // texto procurado
    lerLinha(campo == 1 ? "Nome (ou parte): " : "Categoria (ou parte): ", termo, sizeof(termo));
    aparar(termo);                              // tira espacos das pontas
    if (termo[0] == '\0') { printf("Digite algum texto para buscar.\n"); return; }

    int achou = 0;                              // contador de resultados
    for (int i = 0; i < n; i++) {               // examina cada produto
        const char *alvo = (campo == 1) ? lista[i].nome : lista[i].categoria; // campo escolhido
        if (contemTexto(alvo, termo)) {         // compara ignorando maiusculas
            if (!achou) cabecalhoTabela();      // cabecalho so no primeiro resultado
            linhaTabela(&lista[i]);             // imprime o produto encontrado
            achou++;                            // conta
        }
    }
    if (achou) { rodapeTabela(); printf("%d resultado(s)\n", achou); } // fecha a tabela e conta
    else printf("Nenhum produto encontrado.\n");
}

/* ======================================================================
 * ORDENACAO (BUBBLE SORT) - trabalha numa COPIA, o arquivo nao muda
 * ====================================================================== */

/* Troca dois produtos usando ponteiros. */
void trocar(Produto *a, Produto *b) {
    Produto aux = *a;                           // guarda o conteudo de a
    *a = *b;                                    // a recebe b
    *b = aux;                                   // b recebe o antigo a
}

/* Decide se 'a' deve vir DEPOIS de 'b' (ou seja, se precisam trocar).
 * 1 = nome A-Z | 2 = nome Z-A | 3 = preco menor->maior | 4 = preco maior->menor */
int deveTrocar(const Produto *a, const Produto *b, int criterio) {
    switch (criterio) {                         // escolhe a regra pelo criterio
        case 1: return compararTexto(a->nome, b->nome) > 0; // A-Z: troca se a > b
        case 2: return compararTexto(a->nome, b->nome) < 0; // Z-A: troca se a < b
        case 3: return a->preco > b->preco;     // preco crescente: troca se a > b
        case 4: return a->preco < b->preco;     // preco decrescente: troca se a < b
    }
    return 0;                                   // criterio desconhecido: nao troca
}

/* Bubble Sort: compara vizinhos e "borbulha" o maior para o fim a cada passada. */
void bubbleSort(Produto *v, int n, int criterio) {
    for (int i = 0; i < n - 1; i++) {           // cada passada fixa 1 elemento no lugar certo
        int trocou = 0;                         // otimizacao: detecta vetor ja ordenado
        for (int j = 0; j < n - 1 - i; j++) {   // ignora os que ja estao posicionados no final
            if (deveTrocar(&v[j], &v[j + 1], criterio)) { // vizinhos fora de ordem?
                trocar(&v[j], &v[j + 1]);       // troca os dois
                trocou = 1;                     // registra que houve troca
            }
        }
        if (!trocou) break;                     // nenhuma troca = ja ordenado, encerra antes
    }
}

/* Cria uma copia da lista, ordena a copia e exibe. O original nao muda. */
void ordenarEExibir(Produto *lista, int n, int criterio) {
    if (n == 0) { printf("\nNenhum produto para ordenar.\n"); return; }
    Produto *copia = malloc(n * sizeof(Produto)); // aloca espaco para a copia
    if (copia == NULL) { printf("Sem memoria!\n"); return; }
    memcpy(copia, lista, n * sizeof(Produto));  // copia todos os produtos
    bubbleSort(copia, n, criterio);             // ordena apenas a copia
    mostrarTabela(copia, n);                    // exibe em tabela
    free(copia);                                // libera a memoria da copia
}

/* ======================================================================
 * EXPORTAR CSV (separado por virgula)
 * ====================================================================== */

/* Escreve um texto entre aspas, duplicando aspas internas, para que
 * virgulas dentro do texto nao quebrem as colunas do CSV. */
void campoCsv(FILE *f, const char *s) {
    fputc('"', f);                              // abre aspas
    for (; *s; s++) {                           // cada caractere do texto
        if (*s == '"') fputc('"', f);           // aspas internas viram ""
        fputc(*s, f);                           // escreve o caractere
    }
    fputc('"', f);                              // fecha aspas
}

/* Exporta todos os produtos para produtos.csv (colunas separadas por virgula). */
void exportarCsv(Produto *lista, int n) {
    FILE *f = fopen(ARQ_CSV, "w");              // abre/cria o CSV
    if (f == NULL) { printf("ERRO ao criar %s\n", ARQ_CSV); return; }
    fprintf(f, "ID,Produto,Categoria,Preco,Quantidade,Fornecedor\n"); // cabecalho
    for (int i = 0; i < n; i++) {               // uma linha por produto
        fprintf(f, "%d,", lista[i].id);         // ID
        campoCsv(f, lista[i].nome);      fputc(',', f); // nome
        campoCsv(f, lista[i].categoria); fputc(',', f); // categoria
        fprintf(f, "%.2f,%d,", lista[i].preco, lista[i].quantidade); // preco (ponto) e quantidade
        campoCsv(f, lista[i].fornecedor);       // fornecedor
        fputc('\n', f);                         // fim da linha
    }
    fclose(f);                                  // fecha o arquivo
    printf("Dados exportados para %s (%d produto(s)).\n", ARQ_CSV, n);
}

/* ======================================================================
 * MENU PRINCIPAL
 * ====================================================================== */

/* Imprime o menu de opcoes na tela (uma opcao por linha). */
void menu(void) {
    printf("\n================ CADASTRO DE LOJA ================\n"); // titulo
    printf(" 1  - Listar produtos\n");                               // opcao 1
    printf(" 2  - Inserir produto\n");                               // opcao 2
    printf(" 3  - Alterar produto\n");                              // opcao 3
    printf(" 4  - Excluir produto\n");                              // opcao 4
    printf(" 5  - Buscar por nome\n");                              // opcao 5
    printf(" 6  - Buscar por categoria\n");                         // opcao 6
    printf(" 7  - Ordenar por nome (A-Z)\n");                       // opcao 7
    printf(" 8  - Ordenar por nome (Z-A)\n");                       // opcao 8
    printf(" 9  - Ordenar por preco (menor -> maior)\n");           // opcao 9
    printf(" 10 - Ordenar por preco (maior -> menor)\n");           // opcao 10
    printf(" 11 - Exportar para CSV\n");                            // opcao 11
    printf(" 0  - Sair\n");                                         // opcao 0
    printf("==================================================\n"); // rodape
}

int main(void) {
    criarArquivo();                             // garante que o arquivo exista
    int n = 0;                                  // quantidade de produtos em memoria
    Produto *lista = carregar(&n);              // carrega o arquivo (passa &n para receber a quantidade)
    int opcao;                                  // opcao escolhida no menu

    do {                                        // repete ate o usuario escolher sair
        menu();                                 // mostra as opcoes
        opcao = lerInteiro("Escolha: ");        // le a opcao (-1 se nao for numero)
        if (feof(stdin)) opcao = 0;             // fim da entrada (Ctrl+D/Ctrl+Z): encerra, evitando laco infinito
        switch (opcao) {                        // chama a funcao correspondente
            case 1:  listar(lista, n);              break;
            case 2:  inserir(&lista, &n);           break; // & pois a funcao altera lista e n
            case 3:  alterar(lista, n);             break;
            case 4:  excluir(lista, &n);            break; // & pois n diminui
            case 5:  buscar(lista, n, 1);           break; // 1 = por nome
            case 6:  buscar(lista, n, 2);           break; // 2 = por categoria
            case 7:  ordenarEExibir(lista, n, 1);   break; // nome A-Z
            case 8:  ordenarEExibir(lista, n, 2);   break; // nome Z-A
            case 9:  ordenarEExibir(lista, n, 3);   break; // preco crescente
            case 10: ordenarEExibir(lista, n, 4);   break; // preco decrescente
            case 11: exportarCsv(lista, n);         break; // exporta CSV
            case 0:  printf("Encerrando...\n");     break; // sair
            default: printf("Opcao invalida!\n");          // inclui letras (-1)
        }
    } while (opcao != 0);                       // 0 encerra o laco

    free(lista);                                // libera a memoria alocada
    return 0;                                   // fim normal do programa
}
