#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../dsanim.h"

// --- Animation constants ---
#define animation_field velocity
#define animation_field_name "velocity"
int NFRAME = 8;
int cell_size = 7;
int skip = 1;
int invert = 0;

// --- Timing constants ---
int NTIME = 1000000;

// --- Lattice constants ---
int NX = 240;
int NY = 80;
int NP = 9;

// --- Physics constants ---
double tau = 0.52;
double F_p = 1e-5;
int object_width = 20;
int object_height = 40;
int object_y_offset = 1;

// --- Simulation ---
#define INDEX_2D(i, j) NY *(i) + (j)
#define INDEX_3D(i, j, p) NY * NP * (i) + NP *(j) + (p)

double cs2 = 1.0 / 3.0;
int cx[9] = {0, 1, 0, -1, 0, 1, -1, -1, 1};
int cy[9] = {0, 0, 1, 0, -1, 1, 1, -1, -1};
double w[9] = {4.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0};
int p_bounceback[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};

double *rho, *u, *v, *velocity;
double *f1, *f2;

inline int mod(int x, int n) {
    if (x < 0) return n + x;
    else if (x > n-1) return x - n;
    return x;
}

int main(void)
{

    double uhat, u2, uc;
    double rho_i, u_i, v_i, feq, S;
    int ic, jc;

    rho = (double *)malloc(NX * NY * sizeof(double));
    u = (double *)malloc(NX * NY * sizeof(double));
    v = (double *)malloc(NX * NY * sizeof(double));
    velocity = (double *)malloc(NX * NY * sizeof(double));

    f1 = (double *)malloc(NX * NY * NP * sizeof(double));
    f2 = (double *)malloc(NX * NY * NP * sizeof(double));

    dsa_bag *dsa_params = dsa_init(animation_field, NX, NY, cell_size, skip, invert, animation_field_name);

    for (int i = 0; i < NX; i++)
    {
        for (int j = 0; j < NY; j++)
        {
            u[INDEX_2D(i, j)] = 0.0;
            v[INDEX_2D(i, j)] = 0.0;
            velocity[INDEX_2D(i,j)] = 0.0;
            if ((i >= NX / 2 - object_width / 2) && (i < NX / 2 + object_width / 2) &&
                (j >= NY / 2 - object_height / 2 + object_y_offset) && (j < NY / 2 + object_width / 2 + object_y_offset))
            {
                rho[INDEX_2D(i, j)] = 0.0;
            }
            else
            {
                rho[INDEX_2D(i, j)] = 1.0;
            }
        }
    }

    for (int i = 0; i < NX; i++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int p = 0; p < NP; p++)
            {
                uc = uhat * (double)cx[p];
                f1[INDEX_3D(i, j, p)] = w[p] * (1.0 + uc / cs2 + (uc * uc) / (2.0 * cs2 * cs2) - u2 / (2.0 * cs2));
            }
        }
    }

    int t = 0;
    while (t < NTIME)
    {

        // Get input
        dsa_input(dsa_params);

        if (dsa_params->closed)
            break;

        if (dsa_params->paused)
        {
            dsa_frame(dsa_params);
            continue;
        }

        if ((t > 0) && (t % NFRAME == 0))
        {
            dsa_frame(dsa_params);
        }

        for (int i = 0; i < NX; i++)
        {
            for (int j = 0; j < NY; j++)
            {
                rho_i = rho[INDEX_2D(i, j)];
                u_i = u[INDEX_2D(i, j)];
                v_i = v[INDEX_2D(i, j)];
                u2 = u_i * u_i + v_i * v_i;
                for (int p = 0; p < NP; p++)
                {
                    uc = u_i * (double)cx[p] + v_i * (double)cy[p];
                    feq = w[p] * rho_i * (1.0 + uc / cs2 + (uc * uc) / (2.0 * cs2 * cs2) - u2 / (2.0 * cs2));
                    S = (1.0 - 1.0 / (2.0 * tau)) * w[p] * (((double)cx[p] - u_i) / cs2 + uc / (cs2 * cs2) * (double)cx[p]) * F_p;
                    f2[INDEX_3D(i, j, p)] = (1.0 - 1.0 / tau) * f1[INDEX_3D(i, j, p)] + 1.0 / tau * feq + S;
                }
            }
        }

        for (int i = 0; i < NX; i++)
        {
            for (int j = 0; j < NY; j++)
            {
                for (int p = 0; p < NP; p++)
                {
                    ic = mod(i - cx[p], NX);
                    jc = j - cy[p];

                    if ((i >= NX / 2 - object_width / 2) && (i < NX / 2 + object_width / 2) &&
                        (j >= NY / 2 - object_height / 2 + object_y_offset) && (j < NY / 2 + object_width / 2 + object_y_offset))
                    {
                        continue;
                    }

                    if (((ic >= NX / 2 - object_width / 2) && (ic < NX / 2 + object_width / 2) &&
                        (jc >= NY / 2 - object_height / 2 + object_y_offset) && (jc < NY / 2 + object_width / 2 + object_y_offset)) || 
                        (jc < 0) || (jc > NY-1))
                    {
                        f1[INDEX_3D(i, j, p)] = f2[INDEX_3D(i, j, p_bounceback[p])];
                    }

                    else
                    {
                        f1[INDEX_3D(i, j, p)] = f2[INDEX_3D(ic, jc, p)];
                    }
                }
            }
        }

        for (int i = 0; i < NX; i++) {
            for (int j = 0; j < NY; j++) {

                if ((i >= NX / 2 - object_width / 2) && (i < NX / 2 + object_width / 2) &&
                    (j >= NY / 2 - object_height / 2 + object_y_offset) && (j < NY / 2 + object_width / 2 + object_y_offset))
                {
                    rho[INDEX_2D(i,j)] = 0.0;
                    u[INDEX_2D(i,j)] = 0.0;
                    v[INDEX_2D(i,j)] = 0.0;
                    velocity[INDEX_2D(i,j)] = 0.0;
                }

                else 
                {
                    rho_i = 0.0;
                    u_i = 0.0;
                    v_i = 0.0;
                    for (int p = 0; p < NP; p++) {
                        rho_i += f1[INDEX_3D(i,j,p)];
                        u_i += f1[INDEX_3D(i,j,p)]*(double)cx[p];
                        v_i += f1[INDEX_3D(i,j,p)]*(double)cy[p];
                    }
                    rho[INDEX_2D(i,j)] = rho_i;
                    u[INDEX_2D(i,j)] = u_i/rho_i + F_p/(2.0*rho_i);
                    v[INDEX_2D(i,j)] = v_i/rho_i;
                    velocity[INDEX_2D(i,j)] = sqrt(u[INDEX_2D(i,j)]*u[INDEX_2D(i,j)] + v[INDEX_2D(i,j)]*v[INDEX_2D(i,j)]);
                }
                
            }
        }

        t++;
    }

    dsa_close(dsa_params);
    printf("\nDone!\n");

    return 0;
}