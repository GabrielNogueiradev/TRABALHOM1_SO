#include "trabalhOS.h"

int main(int argc, char* argv[]){
  PGM imagem;
  Header cabecalho;

  //um caminho que não muda
  const char* path = FIFO_PATH;
  mkfifo(path, 0666); //cria a named pipe

  //lê e salva a imagem na struct PGM, em imagem.data
  read_PGM("img_mux.pgm", &imagem);

  //prepara o header para enviar para o worker
  cabecalho.w = imagem.w;
  cabecalho.h = imagem.h;
  cabecalho.maxv = imagem.maxv;
  cabecalho.mode = NEGATIVO;
  cabecalho.t1 = 0;
  cabecalho.t2 = 0;

  //abre a named pipe para que o worker receba a imagem e o cabeçalho
  printf("Esperando pelo Worker...");
  int fd;
  fd = open(path, O_WRONLY);
  printf("Worker conectado. Enviando imagem e cabecalho");

  //envia a imagem e o cabecalho para o main_worker.c
  write(fd, &cabecalho, sizeof(Header));
  write(fd, imagem.data, imagem.w * imagem.h);

  printf("Dados enviados.");

  close(fd);
  free(imagem.data);

  return 0;
}
