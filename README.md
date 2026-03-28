# Simulador de Sistemas Operacionais

Este repositório contém a implementação de um simulador de SO desenvolvido em C. O projeto explora conceitos fundamentais como gerenciamento de processos, escalonamento e abstração de hardware.

---

##  Estrutura do Projeto

* `main.cpp`: Ponto de entrada e loop principal do simulador.
* `include/`: Definições das estruturas (Ex: `PCB.h`, `Memoria.h`).
* `src/`: Implementação das funções e algoritmos de SO.
* `scripts/`: Scripts auxiliares para testes de carga e processos.
* `.gitignore`: Filtro para não subir binários e arquivos de log.

---

## ⚙️ Como Compilar e Executar

Como o projeto lida com recursos do sistema, utilize os comandos abaixo no terminal do Codespaces:

### 1. Compilação Simples
```bash
gcc *.cpp -o simulador -lpthread
