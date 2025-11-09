#ifndef DSANIM_H
#define DSANIM_H

#include <raylib.h>

typedef struct dsa_bag {
    int NX; 
    int NY; 
    int cell_size;
    int skip;
    int invert;

    double ctrl_red[9];
    double ctrl_green[9];
    double ctrl_blue[9];

    double *field;

    int paused;
    int closed;
    int store;
    double last_frame;

    char *name;

    Font font;
} dsa_bag;

dsa_bag *dsa_init(double *field, int NX, int NY, int cell_size, int skip, int invert, char *name);
void dsa_close(dsa_bag *dsa_params);
void dsa_cmap(double x, Color *output, dsa_bag *dsa_params);
double dsa_min(dsa_bag *dsa_params);
double dsa_max(dsa_bag *dsa_params);
void dsa_input(dsa_bag *dsa_params);
void dsa_frame(dsa_bag *dsa_params);

#endif