# Cadastro de Loja em C

![Linguagem](https://img.shields.io/badge/linguagem-C11-00599C?logo=c&logoColor=white)
![Build](https://img.shields.io/badge/build-make%20%7C%20gcc-success)
![Testes](https://img.shields.io/badge/testes-15%20passando-brightgreen)
![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-blue)

Sistema de **cadastro de produtos para loja** executado no terminal, escrito em **C puro** (sem bibliotecas externas). O projeto combina **estruturas de dados (`struct`)**, **ponteiros e memória dinâmica**, **manipulação de arquivos** e **algoritmos de ordenação**, com persistência em arquivo de texto e exportação para CSV.

> Projeto desenvolvido em grupo como **Atividade Avaliativa 04**.

---

## Sumário

- [Funcionalidades](#funcionalidades)
- [Demonstração](#demonstração)
- [Como executar](#como-executar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Formato dos dados](#formato-dos-dados)
- [Decisões técnicas](#decisões-técnicas)
- [Conceitos aplicados](#conceitos-aplicados)
- [Testes](#testes)
- [Limitações e melhorias futuras](#limitações-e-melhorias-futuras)
- [Equipe](#equipe)
- [Licença](#licença)

---

## Funcionalidades

| Recurso | Descrição |
|---|---|
| **Criar** | Gera o arquivo `produtos.txt` com 4 produtos iniciais se ele não existir |
| **Listar** | Exibe todos os produtos em tabela formatada |
| **Inserir** | Valida os dados, impede nomes duplicados e gera o **ID automaticamente** (incremental) |
| **Alterar** | Edita campo a campo (ENTER mantém o valor atual), sem deixar o registro parcialmente alterado em caso de erro |
| **Excluir** | Remove o produto após confirmação |
| **Buscar** | Busca **parcial** por nome ou categoria, sem diferenciar maiúsculas e minúsculas |
| **Ordenar** | Nome (A-Z / Z-A) e preço (crescente / decrescente) com **Bubble Sort**, sem alterar o arquivo |
| **Exportar** | Gera `produtos.csv` separado por vírgula, com campos protegidos por aspas |

**Regras de negócio e robustez**

- Verificação de **duplicidade de nome** (ignorando maiúsculas) e de **ID duplicado**.
- Entrada validada: `abc` no lugar de número é rejeitado, e a vírgula decimal brasileira (`10,5`) é aceita.
- O separador `|` digitado pelo usuário é neutralizado para não corromper o arquivo.
- Gravação com **arquivo temporário** (escreve, apaga o original e renomeia), evitando corrupção de dados em caso de falha.
- Encerramento seguro quando a entrada termina (Ctrl+D / EOF), sem laço infinito.

---

## Demonstração

```text
================ CADASTRO DE LOJA ================
 1  - Listar produtos
 2  - Inserir produto
 3  - Alterar produto
 4  - Excluir produto
 5  - Buscar por nome
 6  - Buscar por categoria
 7  - Ordenar por nome (A-Z)
 8  - Ordenar por nome (Z-A)
 9  - Ordenar por preco (menor -> maior)
 10 - Ordenar por preco (maior -> menor)
 11 - Exportar para CSV
 0  - Sair
==================================================
Escolha: 9

--------------------------------------------------------------------------------------
| ID   | Produto              | Categoria       | Preco (R$) |   Qtd | Fornecedor     |
--------------------------------------------------------------------------------------
| 4    | Cafe 500g            | Alimentos       |      18.00 |    25 | Graos do Sul   |
| 3    | Shampoo 300ml        | Higiene Pessoal |      22.50 |    40 | Beleza Pura    |
| 1    | Camiseta Basica      | Vestuario       |      49.90 |    30 | Moda Brasil    |
| 2    | Mouse Optico USB     | Informatica     |      59.99 |    15 | TecnoShop      |
--------------------------------------------------------------------------------------
Total: 4 produto(s)
```

---

## Como executar

**Requisitos:** compilador `gcc` (Linux, macOS, ou Windows via [MinGW](https://www.mingw-w64.org/) / WSL).

### Com `make` (recomendado)

```bash
git clone https://github.com/<v-arruda>/cadastro-loja-c.git
cd cadastro-loja-c

make          # compila para build/cadastro_loja
make run      # compila e executa
make test     # roda a suíte de testes
make clean    # remove binários e arquivos gerados
```

### Sem `make`

```bash
gcc -Wall -Wextra -std=c11 src/cadastro_loja.c -o cadastro_loja
./cadastro_loja          # Windows: cadastro_loja.exe
```

> O programa cria/usa `produtos.txt` na **pasta onde é executado**.

---

## Estrutura do projeto

```text
cadastro-loja-c/
├── src/
│   └── cadastro_loja.c        # código-fonte completo, comentado linha a linha
├── tests/
│   └── test.sh                # testes de integração automatizados
├── examples/
│   └── produtos_exemplo.txt   # exemplo do formato de dados
├── docs/
│   └── ARQUITETURA.md         # fluxo, funções e decisões de projeto
├── .github/workflows/ci.yml   # integração contínua (compila e testa a cada push)
├── Makefile
├── LICENSE
└── README.md
```

---

## Formato dos dados

Arquivo de texto, **uma linha por produto**, campos separados por `|`:

```text
ID|Produto|Categoria|Preco|Quantidade|Fornecedor
1|Camiseta Basica|Vestuario|49.90|30|Moda Brasil
```

O pipe foi escolhido como separador porque raramente aparece em nomes de produto. Ainda assim, o programa o substitui por `-` se for digitado, protegendo o formato.

---

## Decisões técnicas

| Decisão | Motivo |
|---|---|
| `fgets` + `strtol`/`strtof` em vez de `scanf`/`atoi` | `fgets` não estoura o buffer; `strtol`/`strtof` permitem detectar entradas inválidas (`atoi("abc")` devolveria 0 em silêncio) |
| Vetor dinâmico (`malloc`/`realloc`) | O número de produtos não é conhecido de antemão; a capacidade dobra ao encher |
| `Produto **lista` em `inserir` | O `realloc` pode mover o vetor na memória; a função precisa atualizar o ponteiro do chamador |
| Ordenar uma **cópia** da lista | A ordenação é só de exibição; a ordem do arquivo e da memória original permanece |
| Edição em **cópia de trabalho** em `alterar` | Um erro no meio da edição não deixa o produto meio alterado |
| Funções próprias de comparação case-insensitive | `strcasecmp` não faz parte do C padrão e muda de nome entre sistemas |
| Dados sem acento | Em UTF-8 um acento ocupa 2 bytes e desalinharia a tabela do terminal |
| Limites `MAX_INT` e `MAX_PRECO` | Evitam estouro de `int` e aceitação de `nan`/`inf` como preço |

Mais detalhes em [`docs/ARQUITETURA.md`](docs/ARQUITETURA.md).

---

## Conceitos aplicados

- `struct` e `typedef` para modelar o produto
- Ponteiros, ponteiros para ponteiros e passagem de parâmetros por endereço
- Alocação dinâmica de memória (`malloc`, `realloc`, `free`)
- Leitura e escrita de arquivos de texto (`fopen`, `fgets`, `fprintf`, `remove`, `rename`)
- Algoritmo **Bubble Sort** com critério de ordenação parametrizável
- Validação de entrada e tratamento de erros
- Exportação de dados em formato CSV
- Automação de build (`Makefile`), testes e CI (GitHub Actions)

---

## Testes

A suíte em `tests/test.sh` executa o programa com entradas simuladas e verifica **15 comportamentos**, entre eles: criação do arquivo inicial, rejeição de duplicatas, geração de ID, validação de preço e quantidade, as ordenações, as buscas, alteração, exclusão, exportação CSV e encerramento em EOF.

```bash
make test
```

O workflow em `.github/workflows/ci.yml` compila e testa automaticamente a cada *push* e *pull request*.

---

## Limitações e melhorias futuras

- Ordenação com Bubble Sort tem custo O(n²); para catálogos grandes, trocar por `qsort` ou Merge Sort.
- Persistência em texto reescreve o arquivo inteiro a cada alteração; um banco de dados (ex.: SQLite) seria mais adequado em escala.
- Comparação sem diferenciar maiúsculas funciona apenas para caracteres ASCII (por isso os dados não têm acento).
- IDs de produtos excluídos no fim da lista são reaproveitados.
- Possíveis extensões: relatórios de estoque baixo, valor total do estoque, suporte a UTF-8.

---

## Equipe

|---|---|
| Vitor Arruda dos Santos | 47351241 |
| Guilherme Lima Araujo | 47058773 |
| Diogo Cora | 47071974 |
| Júlio César Rocha | 47422432 |
| Giovanni Henrique Tartaro | 47048026 |
| Arthur Henrique Alves | 47332361 |

---

## Licença

Distribuído sob a licença MIT. Veja o arquivo [`LICENSE`](LICENSE).
