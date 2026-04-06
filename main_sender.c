#include "trabalhOS.h"

int main(int argc, char* argv[]){
  PGM imagem;
  Header cabecalho;
   // nome da imagem a ser alterada

  //um caminho que não muda
  //const char* path = FIFO_PATH;
  const char* path = argv[1];
  const  char* nome_arquivo = argv[2];
  mkfifo(path, 0666); //cria a named pipe

  //lê e salva a imagem na struct PGM, em imagem.data
  read_PGM(nome_arquivo, &imagem);

  //prepara o header para enviar para o worker
  cabecalho.w = imagem.w;
  cabecalho.h = imagem.h;
  cabecalho.maxv = imagem.maxv;
  cabecalho.mode = NEGATIVO;  // aqui da par colocar 0, o worker vai sobreescrever 
  cabecalho.t1 = 0;
  cabecalho.t2 = 0;

  //abre a named pipe para que o worker receba a imagem e o cabeçalho
  printf("Esperando pelo Worker...\n");
  int fd;
  fd = open(path, O_WRONLY);
  printf("Worker conectado. Enviando imagem e cabecalho\n");

  //envia a imagem e o cabecalho para o main_worker.c
  write(fd, &cabecalho, sizeof(Header));

  size_t tamanho_esperado = imagem.w * imagem.h;
  size_t tamanho_enviado = 0;
  while(tamanho_enviado < tamanho_esperado){
    size_t n = write(fd, imagem.data + tamanho_enviado, tamanho_esperado - tamanho_enviado);
    tamanho_enviado += n;
  }

  printf("Dados enviados.\n");

  close(fd);
  free(imagem.data);

  return 0;
}
