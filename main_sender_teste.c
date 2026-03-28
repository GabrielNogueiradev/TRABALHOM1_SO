#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PGM{
  int w, h, maxv;      //maxv = 255
  unsigned char* data; //w*h bytes
}PGM;

int read_PGM(const char* path, PGM* img);
int write_PGM(const char* path, const PGM* img);

int main(int argc, char* argv[]){
  PGM imagem;
  //salva a imagem na struct PGM, em imagem.data
  read_PGM("img_mux.pgm", &imagem);

  //TORNA NEGATIVA A IMAGEM ORIGINAL
  for(register int i=0; i < imagem.h * imagem.w; i++){
    imagem.data[i] = 255 - imagem.data[i];
  }

  write_PGM("NEG_img_mux.pgm", &imagem);

  return 0;
}

int read_PGM(const char* path, PGM* img){
  FILE* file = fopen(path, "rb");
  if(!file){
    perror("Erro ao abrir o arquivo.");
    return -1;
  }

  char magic[4]; //DEVE ACHAR "P5" NA PRIMEIRA LINHA SEMPRE
  /*Aqui o numero mágico é P5 que está na primeira fila mas a linha é P5\n ai precisamos deixar espaço para 3 +1
  o +1 é por que o fgets coloca '\0' no final do vetor, entao são 3 espaços para P5\n +1 para o \0
  por isso fica magic[4] */
  if(!fgets(magic, sizeof(magic), file)){
    printf("Arquivo vazio.\n");
    fclose(file);
    return -1;
  }else{
    if(magic[0] != 'P' || magic[1] != '5'){ //verifica se tem o P5 na primeira linha
      printf("Não é um arquivo PGM.");
      fclose(file);
      return -1;
    }
  }

  //retira os comentários
  int ch;
  while((ch = fgetc(file)) == '#'){
    while(fgetc(file) != '\n');
  }
  ungetc(ch, file);

  fscanf(file, "%d %d %d", &img->w, &img->h, &img->maxv);

  fgetc(file);

  //Aloca memoria para o data e então lê a imagem
  img->data = (unsigned char*)malloc(img->w * img->h * sizeof(unsigned char));
  fread(img->data, 1, img->w * img->h, file);

  fclose(file);
  return 0;
}

int write_PGM(const char* path, const PGM* img){
  FILE* file = fopen(path, "wb");
  fprintf(file, "P5\n%d %d\n %d\n", img->w, img->h, img->maxv);

  fwrite(img->data, 1, img->w * img->h, file);

  fclose(file);
  return 0;
}
