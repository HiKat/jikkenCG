/* VRML 2.0 Reader 
 *
 * ver1.1 2005/10/06 Masaaki IIYAMA (bug fix)
 * ver1.0 2005/09/27 Masaaki IIYAMA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vrml.h"


/*
/////////////////////////////////////////////////////////////////
*/
#define MWS 256

static int strindex( char *s, char *t)
{
    int         i, j, k;

    for (i = 0; s[i] != '\0'; i++) {
        for (j = i, k = 0;  t[k] != '\0' && s[j] == t[k]; j++, k++) ;
        if (k > 0 && t[k] == '\0')
          return i;
    }
    return -1;
}

static int getword(
		   FILE *fp,
		   char word[],
		   int sl)
{
  int i,c;

  while ( (c = fgetc(fp)) != EOF && ( isspace(c) || c == '#')) {
    if ( c == '#' ) {
      while ( (c = fgetc(fp)) != EOF && c != '\n') ;
      if ( c == EOF ) return (0);
    }
  }
  if ( c == EOF )
    return (0);
  ungetc(c,fp);

  for ( i = 0 ; i < sl - 1 ; i++) {
    word[i] = fgetc(fp);
    if ( isspace(word[i]) )
      break;
  }
  word[i] = '\0';

  return i;
}

static int read_material(
			 FILE *fp,
			 Surface *surface,
			 char *b)
{
  while (getword(fp,b,MWS)>0) {
    if      (strindex(b,"}")>=0) break;
    else if (strindex(b,"diffuseColor") >= 0) {
      getword(fp,b,MWS);
      surface->diff[0] = atof(b);
      getword(fp,b,MWS);
      surface->diff[1] = atof(b);
      getword(fp,b,MWS);
      surface->diff[2] = atof(b);
    }
    else if (strindex(b,"ambientIntensity") >= 0) {
      getword(fp,b,MWS);
      surface->ambi = atof(b);
    }
    else if (strindex(b,"specularColor") >= 0) {
      getword(fp,b,MWS);
      surface->spec[0] = atof(b);
      getword(fp,b,MWS);
      surface->spec[1] = atof(b);
      getword(fp,b,MWS);
      surface->spec[2] = atof(b);
    }
    else if (strindex(b,"shininess") >= 0) {
      getword(fp,b,MWS);
      surface->shine = atof(b);
    }
  }
  return 1;
}

static int count_point(
		       FILE *fp,
		       char *b)
{
  int num=0;
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"[")>=0) break;
  }
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"]")>=0) break;
    else {
      num++;
    }
  }
  if ( num %3 != 0 ) {
    fprintf(stderr,"invalid file type[number of points mismatch]\n");
  }
  return num/3;
}

static int read_point(
		      FILE *fp,
		      Polygon *polygon,
		      char *b)
{
  int num=0;
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"[")>=0) break;
  }
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"]")>=0) break;
    else {
      polygon->vtx[num++] = atof(b);
    }
  }
  return num/3;
}

static int count_index(
		       FILE *fp,
		       char *b)
{
  int num=0;
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"[")>=0) break;
  }
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"]")>=0) break;
    else {
      num++;
    }
  }
  if ( num %4 != 0 ) {
    fprintf(stderr,"invalid file type[number of indices mismatch]\n");
  }
  return num/4;
}

static int read_index(
		      FILE *fp,
		      Polygon *polygon,
		      char *b)
{
  int num=0;
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"[")>=0) break;
  }
  while (getword(fp,b,MWS)>0) {
    if (strindex(b,"]")>=0) break;
    else {
      polygon->idx[num++] = atoi(b);
      if (num%3 == 0) getword(fp,b,MWS);
    }
  }
  return num/3;
}

int read_one_obj(
		 FILE *fp,
		 Polygon *poly,
		 Surface *surface)
{
  char b[MWS];
  int flag_material = 0;
  int flag_point = 0;
  int flag_index = 0;

  /* initialize surface */
  surface->diff[0] = 1.0;
  surface->diff[1] = 1.0;
  surface->diff[2] = 1.0;
  surface->spec[0] = 0.0;
  surface->spec[1] = 0.0;
  surface->spec[2] = 0.0;
  surface->ambi = 0.0;
  surface->shine = 0.2;

  if ( getword(fp,b,MWS) <= 0) return 0;

  poly->vtx_num = 0;
  poly->idx_num = 0;

  while (flag_material==0 || flag_point==0 || flag_index==0) {
    if      (strindex(b,"Material")>=0) {
      getword(fp,b,MWS);
      flag_material = 1;
    }
    else if (strindex(b,"point")>=0) {
      fprintf(stderr,"Counting... [point]\n");
      poly->vtx_num = count_point(fp, b);
      flag_point = 1;
    }
    else if (strindex(b,"coordIndex")>=0) {
      fprintf(stderr,"Counting... [coordIndex]\n");
      poly->idx_num = count_index(fp, b);
      flag_index = 1;
    }
    else if (getword(fp,b,MWS) <= 0) return 0;
  }

  flag_material = 0;
  flag_point = 0;
  flag_index = 0;

  fseek(fp, 0, SEEK_SET);
  poly->vtx = (double *)malloc(sizeof(double)*3*poly->vtx_num);
  poly->idx = (int *)malloc(sizeof(int)*3*poly->idx_num);
  while (flag_material==0 || flag_point==0 || flag_index==0) {
    if      (strindex(b,"Material")>=0) {
      fprintf(stderr,"Reading... [Material]\n");
      read_material(fp,surface,b);
      flag_material = 1;
    }
    else if (strindex(b,"point")>=0) {
      fprintf(stderr,"Reading... [point]\n");
      read_point(fp,poly,b);
      flag_point = 1;
    }
    else if (strindex(b,"coordIndex")>=0) {
      fprintf(stderr,"Reading... [coordIndex]\n");
      read_index(fp,poly,b);
      flag_index = 1;
    }
    else if (getword(fp,b,MWS) <= 0) return 0;
  }

  return 1;
}		 

//#ifdef DEBUG_SAMPLE
int main (int argc, char *argv[])
{
  int i;
  FILE *fp;
  Polygon poly;
  Surface surface;

  fp = fopen(argv[1], "r");
  read_one_obj(fp, &poly, &surface);

  fprintf(stderr,"%d vertice are found.\n",poly.vtx_num);
  fprintf(stderr,"%d triangles are found.\n",poly.idx_num);

  /* i th vertex */
  for ( i = 0 ; i < poly.vtx_num ; i++ ) {
    fprintf(stdout,"%f %f %f # %d th vertex\n", 
	    poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2],
	    i);
  }

  /* i th triangle */
  for ( i = 0 ; i < poly.idx_num ; i++ ) {
    fprintf(stdout,"%d %d %d # %d th triangle\n", 
	    poly.idx[i*3+0], poly.idx[i*3+1], poly.idx[i*3+2],
	    i);
  }

  /* material info */
  fprintf(stderr, "diffuseColor %f %f %f\n", surface.diff[0], surface.diff[1], surface.diff[2]);
  fprintf(stderr, "specularColor %f %f %f\n", surface.spec[0], surface.spec[1], surface.spec[2]);
  fprintf(stderr, "ambientIntensity %f\n", surface.ambi);
  fprintf(stderr, "shininess %f\n", surface.shine);
  return 1;
}
//#endif
