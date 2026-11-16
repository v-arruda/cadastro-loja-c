# Arquitetura e decisões de projeto

## Visão geral

O programa mantém os dados em dois lugares:

- **Arquivo** `produtos.txt`: persistência entre execuções.
- **Memória**: um vetor dinâmico de `Produto`, usado enquanto o programa roda.

```text
        início                                   operações que gravam
          │                                  (inserir / alterar / excluir)
  criarArquivo()                                        │
          │                                             ▼
   carregar() ──► vetor em memória ◄──► menu ──► salvarComTemporario() ──► produtos.txt
                         │
          listar / buscar / ordenar (somente leitura)
```

## Modelo de dados

```c
typedef struct {
    int   id;
    char  nome[50];
    char  categoria[30];
    float preco;
    int   quantidade;
    char  fornecedor[50];
} Produto;
```

## Módulos de função

| Grupo | Funções | Responsabilidade |
|---|---|---|
| Entrada | `lerLinha`, `lerInteiro`, `lerFloat`, `converterInteiro`, `converterFloat` | Leitura segura e validação; retornam -1 em entrada inválida |
| Texto | `aparar`, `limparSeparador`, `normalizarCampo`, `paraMinusculo`, `compararTexto`, `contemTexto` | Limpeza e comparação sem diferenciar maiúsculas |
| Arquivo | `criarArquivo`, `carregar`, `salvarComTemporario` | Persistência |
| Consulta | `gerarId`, `indiceDoId`, `indiceDoNome` | ID automático e detecção de duplicatas |
| Exibição | `cabecalhoTabela`, `linhaTabela`, `rodapeTabela`, `mostrarTabela` | Tabela formatada |
| CRUD | `inserir`, `alterar`, `excluir`, `listar`, `buscar` | Operações do usuário |
| Ordenação | `trocar`, `deveTrocar`, `bubbleSort`, `ordenarEExibir` | Ordenação de uma cópia |
| Exportação | `campoCsv`, `exportarCsv` | Saída em CSV |

## Gravação segura (arquivo temporário)

1. Escreve todos os produtos em `produtos_temp.txt`.
2. Confere o retorno de `fclose` (detecta falha de gravação).
3. Remove `produtos.txt`.
4. Renomeia o temporário para `produtos.txt`.

Se o passo 1 ou 2 falhar, o arquivo original permanece intacto.

## Bubble Sort parametrizável

Existe **um único** `bubbleSort`; o critério (nome A-Z, nome Z-A, preço crescente, preço decrescente) é decidido por `deveTrocar(a, b, criterio)`. A variável `trocou` encerra o laço antecipadamente quando uma passada completa não realiza trocas (vetor já ordenado).

Complexidade: O(n²) no pior caso e O(n) no melhor caso (já ordenado).
