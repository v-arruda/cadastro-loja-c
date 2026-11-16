#!/usr/bin/env bash
# Testes de integracao: executam o programa com entradas simuladas
# e verificam o comportamento. Uso: make test
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/build/cadastro_loja"
[ -x "$BIN" ] || { echo "Binario nao encontrado. Rode 'make' primeiro."; exit 1; }

PASS=0; FAIL=0
check() {   # check "descricao" "comando de teste"
    if eval "$2" >/dev/null 2>&1; then echo "  [OK]    $1"; PASS=$((PASS+1))
    else echo "  [FALHA] $1"; FAIL=$((FAIL+1)); fi
}
novo_dir() { D=$(mktemp -d); cd "$D" || exit 1; }   # cada teste usa um diretorio limpo
run() { printf "$1" | "$BIN" 2>&1; }

echo "Executando testes..."

novo_dir
SAIDA=$(run '1\n0\n')
check "cria produtos.txt com 4 produtos iniciais" '[ "$(wc -l < produtos.txt)" -eq 4 ]'
check "lista os produtos em tabela"               'echo "$SAIDA" | grep -q "Camiseta Basica"'

novo_dir
SAIDA=$(run '2\ncamiseta basica\n0\n')
check "rejeita nome duplicado (sem diferenciar maiusculas)" 'echo "$SAIDA" | grep -q "ja existe um produto"'

novo_dir
SAIDA=$(run '2\nLivro\nPapelaria\n10,5\n7\nEditora\n0\n')
check "gera ID automatico (5) e aceita virgula decimal" 'grep -q "^5|Livro|Papelaria|10.50|7|Editora$" produtos.txt'

novo_dir
SAIDA=$(run '2\nA\nc\nnan\n2\nB\nc\ninf\n2\nC\nc\n5\n99999999999\n0\n')
check "rejeita preco nan/inf e quantidade com estouro" '[ "$(wc -l < produtos.txt)" -eq 4 ]'

novo_dir
SAIDA=$(run '9\n0\n')
PRIMEIRO=$(echo "$SAIDA" | grep -E '^\| [0-9]' | head -1)
check "ordena por preco crescente (Cafe 18.00 primeiro)" 'echo "$PRIMEIRO" | grep -q "Cafe 500g"'
SAIDA=$(run '10\n0\n')
PRIMEIRO=$(echo "$SAIDA" | grep -E '^\| [0-9]' | head -1)
check "ordena por preco decrescente (Mouse 59.99 primeiro)" 'echo "$PRIMEIRO" | grep -q "Mouse Optico"'
check "ordenacao nao altera o arquivo" '[ "$(head -1 produtos.txt)" = "1|Camiseta Basica|Vestuario|49.90|30|Moda Brasil" ]'

novo_dir
SAIDA=$(run '5\nCAM\n0\n')
check "busca parcial por nome, sem diferenciar maiusculas" 'echo "$SAIDA" | grep -q "1 resultado"'
SAIDA=$(run '6\nalim\n0\n')
check "busca parcial por categoria" 'echo "$SAIDA" | grep -q "Cafe 500g"'

novo_dir
SAIDA=$(run '3\n2\n\n\n99,90\n\n\n0\n')
check "altera apenas o campo informado (preco)" 'grep -q "^2|Mouse Optico USB|Informatica|99.90|15|TecnoShop$" produtos.txt'

novo_dir
SAIDA=$(run '4\n1\ns\n0\n')
check "exclui produto apos confirmacao" '! grep -q "^1|" produtos.txt && [ "$(wc -l < produtos.txt)" -eq 3 ]'
check "nao deixa arquivo temporario para tras" '[ ! -e produtos_temp.txt ]'

novo_dir
SAIDA=$(run '11\n0\n')
check "exporta CSV com cabecalho e 4 linhas" '[ "$(head -1 produtos.csv)" = "ID,Produto,Categoria,Preco,Quantidade,Fornecedor" ] && [ "$(wc -l < produtos.csv)" -eq 5 ]'

novo_dir
printf '1\n' | timeout 5 "$BIN" >/dev/null 2>&1; RC=$?
check "encerra normalmente quando a entrada acaba (EOF)" '[ '$RC' -eq 0 ]'

echo; echo "Resultado: $PASS passaram, $FAIL falharam."
[ "$FAIL" -eq 0 ]
