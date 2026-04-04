#include "trabalhOS.h"

#define NUM_TASKS 8
#define NUM_THREADS 4

sem_t semaforo;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int prox_tarefa = 0;

// Dados compartilhados para processamento
PGM g_imagem;
Task tarefa[NUM_TASKS];
int g_mode; // MODE_NEG ou MODE_SLICE
int g_t1, g_t2;

void aplicar_negativo(void* arg);
void aplicar_negativo_sem_thread(Task* tarefa, PGM* imagem);

int main(int argc, char* argv[]){
  Header cabecalho;

  //abre a fifo
  const char* path = FIFO_PATH;
  mkfifo(path, 0666); //cria a named pipe

  //recebe os dados enviados pelo sender
  int fd;
  fd = open(path, O_RDONLY);
  printf("Recebendo dados...\n");

  read(fd, &cabecalho, sizeof(Header));
  printf("Cabecalho recebido.\n");
  g_imagem.w = cabecalho.w;
  g_imagem.h = cabecalho.h;
  g_imagem.maxv = cabecalho.maxv;
  printf("altura: %d\n largura: %d\n maxv: %d\n", g_imagem.h, g_imagem.w, g_imagem.maxv);

  g_imagem.data = (unsigned char*)malloc(g_imagem.w * g_imagem.h * sizeof(unsigned char));
  size_t tamanho_esperado = g_imagem.w * g_imagem.h;
  printf("Imagem enviada: %ld bytes\n", tamanho_esperado);
  size_t tamanho_lido = 0;
  while(tamanho_lido < tamanho_esperado){
    size_t n = read(fd, g_imagem.data + tamanho_lido, tamanho_esperado - tamanho_lido);
    tamanho_lido += n;
  }
  printf("Imagem recebida: %ld bytes\n", tamanho_lido);

  close(fd); //terminamos de receber as informações

  pthread_t thread[NUM_THREADS]; //dividir as tarefas em 4 threads
  sem_init(&semaforo, 0, NUM_THREADS); //semaforo para as 4 threads

  int row_por_tarefa = g_imagem.h / NUM_TASKS;
  int sobrou = g_imagem.h % NUM_TASKS; //linhas que sobraram
  int row_atual = 0;
  tarefa[0].row_start = row_atual;
  tarefa[0].row_end = row_por_tarefa + sobrou; //a primeira tarefa fica com as sobras de linhas
  row_atual = tarefa[0].row_end;
  for(int i = 1; i < NUM_TASKS; i++){
    tarefa[i].row_start = row_atual;
    tarefa[i].row_end = row_atual + row_por_tarefa;
    row_atual = tarefa[i].row_end;
  }

  for(int i = 0; i < NUM_THREADS; i++){
    pthread_create(&thread[i], NULL, (void *)aplicar_negativo, NULL);
    //aplicar_negativo_sem_thread(&tarefa[i], &g_imagem);
  }
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  pthread_join(thread[2], NULL);
  pthread_join(thread[3], NULL);

  sem_destroy(&semaforo);
  pthread_mutex_destroy(&mutex);

  write_PGM("saida.pgm", &g_imagem);

  free(g_imagem.data);

  return 0;
}

void aplicar_negativo(void* arg){
  while(1){
    sem_wait(&semaforo); //trava até ter permissão e so podem ter 4 threads existindo

    pthread_mutex_lock(&mutex);
      if(prox_tarefa >= NUM_TASKS){ //verifica se tem proxima tarefa se não tiver quebra a thread
      pthread_mutex_unlock(&mutex);
      sem_post(&semaforo);
      break;
    }
    int tarefa_id = prox_tarefa++;
    pthread_mutex_unlock(&mutex);

    printf("Thread %d processando linhas %d até %d\n", tarefa_id, tarefa[tarefa_id].row_start, tarefa[tarefa_id].row_end);

    for(int i = tarefa[tarefa_id].row_start; i < tarefa[tarefa_id].row_end; i++){
      for(int j = 0; j < g_imagem.w; j++){
        int pos = i * g_imagem.w + j;
        g_imagem.data[pos] = 255 - g_imagem.data[pos];
      }
    }
    sem_post(&semaforo); //libera para a proxima tarefa entrar
  }
}

void aplicar_negativo_sem_thread(Task* tarefa, PGM* imagem){
  int i = tarefa->row_start;

  printf("Processando linhas %d até %d\n", tarefa->row_start, tarefa->row_end);

  for(i; i < tarefa->row_end; i++){
    for(int j = 0; j < g_imagem.w; j++){
      int pos = i * g_imagem.w + j;
      imagem->data[pos] = 255 - imagem->data[pos];
    }
  }
}
