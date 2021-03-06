# include <mcu.h>
# include <math.h>
# include <stdint.h>
# include <stdlib.h>
# include <dct.h>

float c(int ksi)
{
    if (ksi==0) {
      return 1/sqrt(2);
    }
    return 1;
}

int16_t phi(struct array_mcu *mcus, size_t canal, size_t i_mcu, size_t i_bloc,\
            int i, int j, uint8_t h, uint8_t v, float *cos_bloc)
{
    float somme = 0;
    for (int x=0; x<8; x++) {
        for (int y=0; y<8; y++) {
            somme += ((float)mcus->data[canal][(i_mcu*h*v + i_bloc)*64 + x + 8*y])\
            *cos_bloc[8*x+i]*cos_bloc[8*y+j];
        }
    }
    int16_t result = c(i)*c(j)*somme/4;
    return(result);
}

void dct_bloc(struct array_mcu *mcus, size_t canal, size_t i_mcu,\
              size_t i_bloc, uint8_t h, uint8_t v, float* cos_bloc)
{
    // offset
    for (int x=0; x<8; x++){
        for (int y=0; y<8; y++){
            mcus->data[canal][(i_mcu*h*v + i_bloc)*64 + x + 8*y] -= 128;
        }
    }

    int16_t *bloc_frequentiel = malloc(64*sizeof(int16_t));
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
          bloc_frequentiel[i + 8*j] = phi(mcus, canal, i_mcu, i_bloc, i, j, h, v, cos_bloc);
        }
    }
    for (int x=0; x<8; x++){
        for (int y=0; y<8; y++){
            mcus->data[canal][(i_mcu*h*v + i_bloc)*64 + x + 8*y] = bloc_frequentiel[x+8*y];
        }
    }
    free(bloc_frequentiel);
}

void dct(struct array_mcu *mcus)
{
    /* pré-calcul des cosinus sur une idée de Thibault qui fait ganger beaucoup
    de temps */
    float *cos_bloc = malloc(64*sizeof(float));
    for (int x=0; x<8; x++) {
        for (int i = 0; i<8; i++) {
          cos_bloc[8*x + i] = cos((2*x+1)*i*3.1415926535897932/16);
        }
    }
    for (size_t canal=0; canal<mcus->ct; canal ++) {
        uint8_t h = mcus->sf[2*canal];
        uint8_t v = mcus->sf[2*canal + 1];
        for (size_t i_mcu=0; i_mcu < mcus->height*mcus->width; i_mcu ++) {
            for (size_t i_bloc=0; i_bloc < h*v; i_bloc ++) {
                dct_bloc(mcus, canal, i_mcu, i_bloc, h, v, cos_bloc);
            }
        }
    }
    free(cos_bloc);
}
