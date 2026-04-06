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
void aplicar_fatiamento();
void aplicar_fatiamento_sem_therad();

int main(int argc, char* argv[]){
  Header cabecalho;
  int borda_inferior_fatiamento, borda_superior_fatiamento, modo_trabalho, num_threads;
  char nome[50]; //nome do arquivo

  const char* path = argv[1]; //caminho pra fifo
  int modo = atoi(argv[2]); //modo de trabalho, fatiamento=1 ou negativo = 0  
  if(modo == NEGATIVO){
    modo_trabalho = NEGATIVO;
    if(argc >=4){
      num_threads= atoi(argv[3]);
    }else{
      num_threads=NUM_THREADS;
    }
  }else if(modo == SLICE){
    modo_trabalho = SLICE;
    borda_inferior_fatiamento = atoi(argv[3]);
    borda_superior_fatiamento = atoi(argv[4]);
    if(argc >=6){
      num_threads = atoi(argv[5]);
    }else{
      num_threads = NUM_THREADS;
    }
  }else{
    exit(1);
  }

  pthread_t thread[num_threads]; //divide as tarefas, se for fatiamento tem q passar o parametro por quando for chamar o programa, se for negativo é o valor setado em NUM_THREADS

    printf("Threadas criadas %d\n", num_threads);


  //abre a fifo
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
  cabecalho.mode = modo_trabalho;
  cabecalho.t1 = borda_inferior_fatiamento;
  cabecalho.t2 = borda_superior_fatiamento;
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

  sem_init(&semaforo, 0, num_threads); //semaforo para as 4 threads

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

  int IDs_threads[num_threads]; 
  if(cabecalho.mode == NEGATIVO){
    for(int i = 0; i < num_threads; i++){
      IDs_threads[i] = i+1;
      pthread_create(&thread[i], NULL, (void *)aplicar_negativo, &IDs_threads[i]);
      //aplicar_negativo_sem_thread(&tarefa[i], &g_imagem);
    }
  }

  for(int i=0; i<num_threads; i++){
      pthread_join(thread[i], NULL);

  }
  /*
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  pthread_join(thread[2], NULL);
  pthread_join(thread[3], NULL);
  */

  sem_destroy(&semaforo);
  pthread_mutex_destroy(&mutex);

  //Verificação pra caso o arquivo já exista
  int contador=1, verifica;
  do{
    snprintf(nome, 50,"saida%d.pgm", contador);
    verifica= 0;
    FILE* teste = fopen(nome, "rb");
      if(teste != NULL){
        // arquivo existe
        fclose(teste);
        contador++;
        verifica = 1;
      }
  }while(verifica ==1);

  write_PGM(nome, &g_imagem);


  free(g_imagem.data);

  return 0;
}

void aplicar_negativo(void* arg){
  int thread_id = *(int *)arg;
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

    printf("Thread: %d, tarefa %d processando linhas %d até %d\n", thread_id, tarefa_id, tarefa[tarefa_id].row_start, tarefa[tarefa_id].row_end);

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

