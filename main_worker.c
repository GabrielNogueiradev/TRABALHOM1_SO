//Para compilar o código: gcc -o worker main_worker.c -lpthread
//Pra rodar o programa, passando o path (operação negativo): /"nome do programa" path(nome pro caminho, precisa ser igual nos dois(worker e sender)) 0 num_threads 
//Pra rodar o programa, sem passar o path (operação negativo): ./worker 0 num_threads
//Pra rodar o programa, passando o path (operação fatiamento): /"nome do programa" path(nome pro caminho, precisa ser igual nos dois(worker e sender)) 1 limite_inferior limite_superior num_threads 
//Pra rodar o programa, sem passar o path (operação fatiamento): /"nome do programa" 1 limite_inferior limite_superior num_threads 
//num_threads é a quantidade de threads que deseja passar. Se não for informado nenhum valor, ele vai utilizar um predefinido.

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
int g_t1, g_t2; // t1= limite inferior, t2 = limite superior

void aplicar_negativo(void* arg);
void aplicar_fatiamento(void* arg);

int main(int argc, char* argv[]){
  Header cabecalho;
  int num_threads;
  char nome[50]; //nome do arquivo de saida
  const char* path;

  //Caso queira passar o path na hora de executar o programa
  /*
  path = argv[1]; //caminho pra fifo
  g_mode = atoi(argv[2]); //modo de trabalho, fatiamento=1 ou negativo = 0  
  if(g_mode == NEGATIVO){
    if(argc >=4){
      num_threads= atoi(argv[3]);
    }else{
      num_threads=NUM_THREADS;
    }
  }else if(g_mode == SLICE){
    g_t1 = atoi(argv[3]);
    g_t2 = atoi(argv[4]);
    if(argc >=6){
      num_threads = atoi(argv[5]);
    }else{
      num_threads = NUM_THREADS;
    }
  }else{

    printf("Alguma coisa certamente está errada");
    exit(1);
  }
  */

  //pra caso não queira que o path seja passado na hora de executar o programa
  path = FIFO_PATH;
  g_mode = atoi(argv[1]);
  if(g_mode == NEGATIVO){
    if(argc == 2){
      num_threads = NUM_THREADS;
    }else{
      num_threads = atoi(argv[2]);
    }
  }else if(g_mode == SLICE){
      g_t1 = atoi(argv[2]);
      g_t2 = atoi(argv[3]);
    if(argc==4){
      num_threads=NUM_THREADS;
    }
    else{
      num_threads=atoi(argv[4]);
    }
  }else{
    printf("Operação não encontrada.\n 0=Negativo\n1=Fatiamento");
    exit(1);
  }
    

  

  pthread_t thread[num_threads]; //cria as threads

  printf("Threadas a serem criadas: %d\n\n", num_threads);


  //abre a fifo
  mkfifo(path, 0666); //cria a named pipe

  //recebe os dados enviados pelo sender
  int fd;  // file descriptor
  fd = open(path, O_RDONLY);
  printf("Recebendo dados...\n");

  read(fd, &cabecalho, sizeof(Header));
  g_imagem.w = cabecalho.w;
  g_imagem.h = cabecalho.h;
  g_imagem.maxv = cabecalho.maxv;
  printf("\n----------------\nAltura: %d\nLargura: %d\nMaxv: %d\n", g_imagem.h, g_imagem.w, g_imagem.maxv);

  g_imagem.data = (unsigned char*)malloc(g_imagem.w * g_imagem.h * sizeof(unsigned char)); // aloca memoria pra receber a imagem

  size_t tamanho_esperado = g_imagem.w * g_imagem.h;
  size_t tamanho_lido = 0;
  while(tamanho_lido < tamanho_esperado){
    size_t n = read(fd, g_imagem.data + tamanho_lido, tamanho_esperado - tamanho_lido);
    tamanho_lido += n;
  }
  printf("Imagem recebida: %ld bytes\n----------------\n", tamanho_lido);

  printf("Imagem e cabecalho recebidos.\n\n");

  close(fd); //terminamos de receber as informações

  sem_init(&semaforo, 0, num_threads); //semaforo para as threads criadas

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

  int IDs_threads[num_threads];  // vetor pra identificação das threads. Não é necessário, mas estamos usando pra identificar de uma maneira mais fácil
  if(g_mode == NEGATIVO){
    for(int i = 0; i < num_threads; i++){
      IDs_threads[i] = i+1;
      pthread_create(&thread[i], NULL, (void *)aplicar_negativo, &IDs_threads[i]); //inicilia a thread
    }
  }else{
    for(int i = 0; i < num_threads; i++){
      IDs_threads[i] = i+1;
      pthread_create(&thread[i], NULL, (void *)aplicar_fatiamento, &IDs_threads[i]); //inicilia a thread
    }
  }

  for(int i=0; i<num_threads; i++){
      pthread_join(thread[i], NULL); 

  }

  sem_destroy(&semaforo);
  pthread_mutex_destroy(&mutex);

  //Verificação pra caso o arquivo já exista
  int contador=1;
  FILE* teste;
  do{
    snprintf(nome, 50,"saida%d.pgm", contador);
      teste = fopen(nome, "rb");
      if(teste != NULL){
        // arquivo existe
        fclose(teste);
        contador++;
      }
  }while(teste !=NULL);

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

void aplicar_fatiamento(void* arg){
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
        if(g_imagem.data[pos] > g_t2 || g_imagem.data[pos]< g_t1){
          g_imagem.data[pos] = 0;
        }else{
          g_imagem.data[pos]=255;
        }
      }
    }
    sem_post(&semaforo); //libera para a proxima tarefa entrar
  }
}