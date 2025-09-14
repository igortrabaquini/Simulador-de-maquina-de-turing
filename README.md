# 🖥️ Simulador de Máquina de Turing em C

Este projeto implementa um **simulador de Máquina de Turing** em linguagem **C**, seguindo especificações fornecidas em um arquivo **JSON**.  

O programa lê:
- **Arquivo de regras (`.json`)** → define estados, transições, símbolo branco etc.
- **Arquivo de entrada (`.txt`)** → contém a fita inicial.
- **Arquivo de saída (`.txt`)** → conterá o resultado final da fita após a execução.

Ao final, o programa imprime na saída padrão:
- `1` → se a máquina **aceitou** (atingiu um estado final).
- `0` → se a máquina **rejeitou** (nenhuma transição aplicável).

---

## 📂 Estrutura dos Arquivos

### Arquivo JSON (regras)
Um exemplo simplificado:

```json
{
  "initial": 0,
  "finals": [1],
  "white": "_",
  "transitions": [
    { "from": 0, "read": "a", "to": 0, "write": "a", "dir": "R" },
    { "from": 0, "read": "_", "to": 1, "write": "_", "dir": "R" }
  ]
}
