#include "trabalhOS.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Dados compartilhados para processamento
PGM imagem, imagem_out;
int g_mode; // MODE_NEG ou MODE_SLICE
int g_t1, g_t2;

void aplicar_negativo(void* arg);

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
  imagem.w = cabecalho.w;
  imagem.h = cabecalho.h;
  imagem.maxv = cabecalho.maxv;
  imagem.data = (unsigned char*)malloc(imagem.w * imagem.h * sizeof(unsigned char));
  read(fd, imagem.data, imagem.w * imagem.h);
  printf("Imagem recebida.\n");

  close(fd); //terminamos de receber as informações

  pthread_t thread[4]; //dividir as tarefas em 4 threads
  Task tarefa[4];

  //divide as 874 linhas da imagem entre as tarefas - DEPOIS CRIAR UMA FUNÇÃO QUE FAÇA AUTOMATICO COM BASE NO TAMANHO DA IMAGEM ENVIADA
  tarefa[0].row_start = 0;
  tarefa[0].row_end = 218;
  tarefa[1].row_start = 218;
  tarefa[1].row_end = 437;
  tarefa[2].row_start = 437;
  tarefa[2].row_end = 656;
  tarefa[3].row_start = 656;
  tarefa[3].row_end = 874;

  for(int i = 0; i < 4; i++){
    pthread_create(&thread[i], NULL, (void *)aplicar_negativo, &tarefa[i]);
  }
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  pthread_join(thread[2], NULL);
  pthread_join(thread[3], NULL);

  pthread_mutex_destroy(&mutex);

  write_PGM("saida.pgm", &imagem);

  free(imagem.data);

  return 0;
}

void aplicar_negativo(void* arg){
  Task* temp_task = (Task*)arg;
  int i = temp_task->row_start;

  printf("Thread processando linhas %d até %d\n", temp_task->row_start, temp_task->row_end);

  for(i; i < temp_task->row_end; i++){
    for(int j = 0; j < imagem.w; j++){
      int pos = i * imagem.w + j;
      pthread_mutex_lock(&mutex);
      imagem.data[pos] = 255 - imagem.data[pos];
      pthread_mutex_unlock(&mutex);
    }
  }
}

