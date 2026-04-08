//Para compilar o sender: gcc -o sender main_sender.c -lpthread
//Pra rodar o códogo, passando o caminho: ./"nome do programa" path(nome pro caminho, precisa ser igual nos dois(worker e sender)) "nome da imagem"
//Pra rodar o códogo, sem passar o caminho: ./"nome do programa" "nome da imagem"

#include "trabalhOS.h"

int main(int argc, char* argv[]){
  PGM imagem;
  Header cabecalho;

  //caso não queira passar o caminho
  const char* path = FIFO_PATH;
  const  char* nome_arquivo = argv[1];

  /* //pra caso queira passar o caminho no momento de rodar o programa
  const char* path = argv[1];
  const  char* nome_arquivo = argv[2];
  */

  mkfifo(path, 0666); //cria a named pipe

  //lê e salva a imagem na struct PGM, em imagem.data
  read_PGM(nome_arquivo, &imagem);

  //prepara o header para enviar para o worker
  cabecalho.w = imagem.w;
  cabecalho.h = imagem.h;
  cabecalho.maxv = imagem.maxv;
  cabecalho.mode = 0;  // aqui da par colocar 0, o worker vai sobreescrever 
  cabecalho.t1 = 0;
  cabecalho.t2 = 0;

  //abre a named pipe para que o worker receba a imagem e o cabeçalho
  printf("Esperando pelo Worker...\n");
  int fd;
  fd = open(path, O_WRONLY);
  printf("Worker conectado. Enviando imagem e cabecalho\n");
  printf("\n----------------\nAltura: %d\nLargura: %d\nMaxv: %d\n", cabecalho.h, cabecalho.w, cabecalho.maxv);


  //envia a imagem e o cabecalho para o main_worker.c
  write(fd, &cabecalho, sizeof(Header));

  size_t tamanho_esperado = imagem.w * imagem.h;
  size_t tamanho_enviado = 0;
  while(tamanho_enviado < tamanho_esperado){
    size_t n = write(fd, imagem.data + tamanho_enviado, tamanho_esperado - tamanho_enviado);
    tamanho_enviado += n;
  }

  printf("Imagem enviada: %ld bytes\n----------------\n", tamanho_esperado);

  printf("Dados enviados.\n\n");

  close(fd);
  free(imagem.data);

  return 0;
}
