/*
 * Waveletによる画像処理サンプルプログラム
 *      by Akinori Ito, June 2002
 *
 * プログラムの説明
 *   このプログラムは，pnm形式の画像を読みこんで，そこからwavelet展開
 *   係数を計算したり，また展開係数から原画像を再構成したりします．処理には
 *   Haar wavelet を使っています．
 *
 * コマンドの使用方法
 *   proc_w -level n infile.pnm outfile.pnm
 *        wavelet展開係数を n レベルまで計算します．
 *
 *   proc_w -level n -compose infile.pnm outfile.pnm
 *        wavelet展開係数を含んだ画像を n レベル復元します．
 *
 * 制限事項
 *   入出力形式は，pnm(ppmまたはpgm)形式だけです．
 *   各種形式から pnm への変換は，netpbm を使いましょう．
 *     http://netpbm.sourceforge.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORMAT_PBM       1
#define FORMAT_PGM       2
#define FORMAT_PPM       3
#define FORMAT_PBMRAW    4
#define FORMAT_PGMRAW    5
#define FORMAT_PPMRAW    6

struct myppm {
  int size;
  int width;
  int height;
  int format;
  struct {
    unsigned char plain[3];
  } *data;
};

#define MyPPM_point(ppm,x,y) ((ppm)->data[(y)*(ppm)->width+(x)].plain)

#define MyPPM_BSIZE 1024

#define BIAS(x)   ((x)+128)
#define UNBIAS(x) ((x)-128)
#define LIMIT_BYTE(x) ((x)<0?0:(x)>255?255:(x))

struct myppm *
MyPPM_new(int width, int height)
{
  struct myppm *ppm = malloc(sizeof(struct myppm));
  ppm->size = width*height*3;
  ppm->width = width;
  ppm->height = height;
  ppm->format = 3;
  ppm->data = malloc(ppm->size);
  return ppm;
}

char *
MyPPM_readline(char *buf, int size, FILE *f)
{
  for (;;) {
    if (fgets(buf,size,f) == NULL)
      return NULL;
    if (buf[0] != '#')
      return buf;
  }
}

struct myppm *
MyPPM_read(char *file)
{
  struct myppm *ppm;
  char buffer[MyPPM_BSIZE];
  FILE *f;
  int x,y,i;

  if ((f = fopen(file,"rb")) == NULL)
    return NULL;
  ppm = malloc(sizeof(struct myppm));
  /* read type */
  if (MyPPM_readline(buffer,MyPPM_BSIZE,f) == NULL)
    return NULL;
  if (strcmp(buffer,"P2¥n") == 0)
    ppm->format = FORMAT_PGM;
  else if (strcmp(buffer,"P3¥n") == 0)
    ppm->format = FORMAT_PPM;
  else if (strcmp(buffer,"P5¥n") == 0)
    ppm->format = FORMAT_PGMRAW;
  else if (strcmp(buffer,"P6¥n") == 0)
    ppm->format = FORMAT_PPMRAW;
  else {
    fprintf(stderr,"PPM format mismatch¥n");
    return NULL;
  }
  /* read width&height */
  for (;;) {
    if (MyPPM_readline(buffer,MyPPM_BSIZE,f) == NULL)
      return NULL;
    if (buffer[0] == '¥n' || buffer[0] == '#')
      continue;
    break;
  }
  if (sscanf(buffer,"%d%d",&ppm->width,&ppm->height) != 2) {
    fprintf(stderr,"PPM size bad¥n");
    return NULL;
  }
  /* read depth */
  if (MyPPM_readline(buffer,MyPPM_BSIZE,f) == NULL)
    return NULL;
  if (strcmp(buffer,"255¥n") != 0) {
    fprintf(stderr,"PPM depth unsupported¥n");
    return NULL;
  }
  ppm->data = malloc(ppm->width*ppm->height*3);
  for (y = 0; y < ppm->height; y++) {
    for (x = 0; x < ppm->width; x++) {
      int pix;
      switch (ppm->format) {
      case FORMAT_PGM:
	fscanf(f,"%d",&pix);
	for (i = 0; i < 3; i++)
	  MyPPM_point(ppm,x,y)[i] = pix;
	break;
      case FORMAT_PPM:
	for (i = 0; i < 3; i++) {
	  fscanf(f,"%d",&pix);
	  MyPPM_point(ppm,x,y)[i] = pix;
	}
	break;
      case FORMAT_PGMRAW:
	pix = fgetc(f);
	for (i = 0; i < 3; i++)
	  MyPPM_point(ppm,x,y)[i] = pix;
	break;
      case FORMAT_PPMRAW:
	for (i = 0; i < 3; i++) {
	  pix = fgetc(f);
	  MyPPM_point(ppm,x,y)[i] = pix;
	}
	break;
      }
    }
  }
  fclose(f);
  return ppm;
}

struct myppm *
MyPPM_write(struct myppm *ppm, char *file)
{
  FILE *f;
  int x,y,i;

  if ((f = fopen(file,"wb")) == NULL)
    return NULL;
  fprintf(f,"P%d¥n",ppm->format);
  fprintf(f,"%d %d¥n",ppm->width,ppm->height);
  fprintf(f,"255¥n");
  for (y = 0; y < ppm->height; y++) {
    for (x = 0; x < ppm->width; x++) {
      switch (ppm->format) {
      case FORMAT_PGM:
	fprintf(f,"%d¥n",MyPPM_point(ppm,x,y)[0]);
	break;
      case FORMAT_PPM:
	for (i = 0; i < 3; i++) {
	  fprintf(f,"%d¥n",MyPPM_point(ppm,x,y)[i]);
	}
	break;
      case FORMAT_PGMRAW:
	fputc(MyPPM_point(ppm,x,y)[0],f);
	break;
      case FORMAT_PPMRAW:
	for (i = 0; i < 3; i++) {
	  fputc(MyPPM_point(ppm,x,y)[i],f);
	}
	break;
      }
    }
  }
  fclose(f);
  return ppm;
}

void
MyPPM_free(struct myppm *ppm)
{
  free(ppm->data);
  free(ppm);
}

struct myppm *
MyPPM_dup(struct myppm *from_ppm)
{
  struct myppm *newppm;
  newppm = MyPPM_new(from_ppm->width,from_ppm->height);
  newppm->format = from_ppm->format;
  memcpy(newppm->data,from_ppm->data,newppm->size);
  return newppm;
}
  

void
do_wavelet(struct myppm *ppm,int width, int height)
{
  struct myppm *ppm2;
  int x,y,i,half;
  int p,p2;

  ppm2 = MyPPM_new(width,height);
  /* wavelet for X */
  half = width/2;
  for (y = 0; y < height; y++) {
    for (i = 0; i < 3; i++) {
      for (x = 0; x < half; x++) {
	p = MyPPM_point(ppm,2*x,y)[i];
	if (2*x+1 < width)
	  p2 = MyPPM_point(ppm,2*x+1,y)[i];
	else
	  p2 = 0;
	/* Haar */
	MyPPM_point(ppm2,x,y)[i] = (p+p2)/2;
	MyPPM_point(ppm2,x+half,y)[i] = BIAS((p-p2)/2);
      }
    }
  }
  /* wavelet for Y */
  half = height/2;
  for (x = 0; x < width; x++) {
    for (i = 0; i < 3; i++) {
      for (y = 0; y < half; y++) {
	p = MyPPM_point(ppm2,x,2*y)[i];
	if (2*y+1 < height)
	  p2 = MyPPM_point(ppm2,x,2*y+1)[i];
	else
	  p2 = 0;
	/* Haar */
	MyPPM_point(ppm,x,y)[i] = (p+p2)/2;
	MyPPM_point(ppm,x,y+half)[i] = BIAS((p-p2)/2);
      }
    }
  }
  MyPPM_free(ppm2);
}

void
do_compose(struct myppm *ppm,int width, int height)
{
  struct myppm *ppm2;
  int x,y,i,half;
  int p,p2;

  ppm2 = MyPPM_new(width,height);

  /* wavelet for X */
  half = width/2;
  for (y = 0; y < height; y++) {
    for (i = 0; i < 3; i++) {
      for (x = 0; x < half; x++) {
	p = MyPPM_point(ppm,x,y)[i];
	p2 = UNBIAS(MyPPM_point(ppm,x+half,y)[i]);
	/* Haar */
	MyPPM_point(ppm2,2*x,y)[i] = LIMIT_BYTE(p+p2);
	MyPPM_point(ppm2,2*x+1,y)[i] = LIMIT_BYTE(p-p2);
      }
    }
  }
  /* wavelet for Y */
  half = height/2;
  for (x = 0; x < width; x++) {
    for (i = 0; i < 3; i++) {
      for (y = 0; y < half; y++) {
	p = MyPPM_point(ppm2,x,y)[i];
	p2 = UNBIAS(MyPPM_point(ppm2,x,y+half)[i]);
#if 0
	if (p+p2 < 0 || 255< p+p2) {printf("P(%d,%d)=%d¥n",x,2*y,p+p2);}
	if (p-p2 < 0 || 255< p-p2) {printf("P(%d,%d)=%d¥n",x,2*y+1,p-p2);}
#endif
	/* Haar */
	MyPPM_point(ppm,x,2*y)[i] = LIMIT_BYTE(p+p2);
	MyPPM_point(ppm,x,2*y+1)[i] = LIMIT_BYTE(p-p2);
      }
    }
  }
  MyPPM_free(ppm2);
}

void
usage()
{
  fprintf(stderr,"usage: proc_w [-compose] [-level n] infile outfile¥n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  struct myppm *ppm;
  int i,level,width,height;
  char *infile,*outfile;
  int compose = 0;

  if (argc < 3) 
    usage();
  level = 1;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-')
      break;
    if (strcmp(argv[i],"-level") == 0)
      level = atoi(argv[++i]);
    else if (strcmp(argv[i],"-compose") == 0)
      compose = 1;
  }
  if (i+1 >= argc)
    usage();
  infile = argv[i];
  outfile = argv[i+1];

  ppm = MyPPM_read(infile);
  if (ppm == NULL) {
    fprintf(stderr,"Can't open %s¥n",infile);
    return 1;
  }
  width = ppm->width;
  height = ppm->height;
  if (compose) {
    int w,h;
    for (i = level-1; i >= 0; i--) {
      w = width/(1<<i);
      h = height/(1<<i);
      do_compose(ppm,w,h);
    }
  }
  else {
    for (i = 0; i < level; i++) {
      do_wavelet(ppm,width,height);
      width /= 2;
      height /= 2;
    }
  }
  if (MyPPM_write(ppm,outfile) == NULL) {
    fprintf(stderr,"Can't open %s¥n",outfile);
    return 1;
  }
  return 0;
}

