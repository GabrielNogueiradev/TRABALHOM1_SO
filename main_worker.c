#include "trabalhOS.h"

int main(int argc, char* argv[]){

  PGM imagem;
  Header cabecalho;

  //abre a fifo
  const char* path = FIFO_PATH;
  mkfifo(path, 0666); //cria a named pipe

  //recebe os dados enviados pelo sender
  int fd;
  fd = open(path, O_RDONLY);
  printf("Recebendo dados...");

  read(fd, &cabecalho, sizeof(Header));
  printf("Cabecalho recebido.");
  imagem.w = cabecalho.w;
  imagem.h = cabecalho.h;
  imagem.maxv = cabecalho.maxv;
  imagem.data = (unsigned char*)malloc(imagem.w * imagem.h * sizeof(unsigned char*));
  read(fd, imagem.data, imagem.w * imagem.h);
  printf("Imagem recebida.");

  close(fd); //terminamos de receber as informações

  //FALTA IMPLEMENTAR A FILA DE TAREFAS, FALTA CRIAR AS TAREFAS, FALTA CRIAR AS THREADS E DIVIDIR AS TAREFAS ENTRE AS THREADS POR MEIO DA FILA
  //FALTA ENTENDER AS VARIAVEIS GLOBAIS QUE O PROFESSOR PASSOU

  return 0;
}
