# Fonte do relatorio da Experiencia 1

O relatorio vai em PDF, no mesmo formato das atividades 1, 2 e 4. O texto mora no
`gera.py`, que monta o HTML e insere a listagem de `lib/enlace/enlace.c` lida
direto do repo, para o codigo do relatorio nunca divergir do codigo que roda.

```
python relatorio/gera.py
python relatorio/topdf.py relatorio/relatorio-final.html "Experiencia 1 Hugo e Wender.pdf"
```

O `topdf.py` usa o Chrome headless pelo DevTools Protocol, porque a flag
`--print-to-pdf-no-header` nao funciona em Chrome recente. Precisa do pacote
`websocket-client`.

Duas armadilhas que ja custaram retrabalho:

- **`@page { margin: 0 }` no CSS anula as margens do `printToPDF`**, e o texto
  sai de borda a borda. As margens ficam so no conversor.
- Cabecalho de tabela que atravessa pagina so repete se a linha estiver dentro
  de `<thead>` com `display: table-header-group`.
