#include <raylib.h>
#include <math.h>       // fmin, fmax
#include <stdlib.h>     // malloc, free
#include <stdio.h>      // sprintf
#include <string.h>     // 

#include "dsanim.h"


dsa_bag *dsa_init(double *field, int NX, int NY, int cell_size, int skip, int invert, char *name) {
    int screen_width = NX*cell_size/skip;
    int screen_height = NY*cell_size/skip;
    SetTraceLogLevel(LOG_ERROR); 
    InitWindow(screen_width, screen_height, "dsanim");

    dsa_bag *dsa_params = (dsa_bag*) malloc(sizeof(dsa_bag));
    dsa_params->NX = NX;
    dsa_params->NY = NY;
    dsa_params->cell_size = cell_size;
    dsa_params->invert = invert;
    dsa_params->skip = skip;
    dsa_params->field = field;

    dsa_params->paused = 0;
    dsa_params->closed = 0;
    dsa_params->store = 0;
    dsa_params->last_frame = GetTime();

    dsa_params->name = (char *) malloc((strlen(name)+1)*sizeof(char));
    strcpy(dsa_params->name, name);

    printf("%s\n", dsa_params->name);

    double ctrl_red[9] = {
        0.267004, 0.282327, 0.253935, 0.206756,
        0.163625, 0.127568, 0.134692, 0.477504, 0.993248
    };

    double ctrl_green[9] = {
        0.004874, 0.094955, 0.265254, 0.371758,
        0.471133, 0.566949, 0.658636, 0.821444, 0.906157
    };

    double ctrl_blue[9] = {
        0.329415, 0.417331, 0.529983, 0.553117,
        0.558148, 0.550556, 0.517649, 0.318195, 0.143936
    };

    
    for (int i = 0; i < 9; i++) {
        dsa_params->ctrl_red[i] = ctrl_red[i];
        dsa_params->ctrl_green[i] = ctrl_green[i];
        dsa_params->ctrl_blue[i] = ctrl_blue[i];
    }

    dsa_params->font = LoadFontEx("dsanim/times.ttf", 20, 0, 250);

    return dsa_params;
}

void dsa_close(dsa_bag *dsa_params) {
    CloseWindow();
    free(dsa_params->name);
    free(dsa_params);
}

void dsa_cmap(double x, Color *output, dsa_bag *dsa_params) {
    int invert = dsa_params->invert;
    double *ctrl_red = dsa_params->ctrl_red;
    double *ctrl_green = dsa_params->ctrl_green;
    double *ctrl_blue = dsa_params->ctrl_blue;

    if (invert) x = 1.0-x;

    int i = (int) (x / 0.125);
    double j = x - (double)i*0.125;
    output->r = (char)(255.0*(ctrl_red[i] + j*(ctrl_red[i+1] - ctrl_red[i])/0.125));
    output->g = (char)(255.0*(ctrl_green[i] + j*(ctrl_green[i+1] - ctrl_green[i])/0.125));
    output->b = (char)(255.0*(ctrl_blue[i] + j*(ctrl_blue[i+1] - ctrl_blue[i])/0.125));
}

double dsa_min(dsa_bag *dsa_params) {
    int NX = dsa_params->NX; 
    int NY = dsa_params->NY;
    int skip = dsa_params->skip;
    double *field = dsa_params->field;

    double value;
    double result = field[0];
    for (int i = 0; i < NX; i+=skip) {
        for (int j = 0; j < NY; j+=skip) {
            value = field[i*NY + j];
            if (value < result) result = value;
        }
    }
    return result;
}

double dsa_max(dsa_bag *dsa_params) {
    int NX = dsa_params->NX; 
    int NY = dsa_params->NY;
    int skip = dsa_params->skip;
    double *field = dsa_params->field;

    double value;
    double result = field[0];
    for (int i = 0; i < NX; i+=skip) {
        for (int j = 0; j < NY; j+=skip) {
            value = field[i*NY + j];
            if (value > result) result = value;
        }
    }
    return result;
}

void dsa_input(dsa_bag *dsa_params) {
    PollInputEvents(); 
    if (IsKeyReleased(KEY_SPACE)) {
        dsa_params->paused = !dsa_params->paused;
    }
    if (IsKeyReleased(KEY_S)) {
        dsa_params->store = 1;
    }
    if (WindowShouldClose()) {
        dsa_params->closed = 1;
    }
}

void dsa_frame(dsa_bag *dsa_params) {
    double fraction, current_frame, FPS;
    char FPS_string[32];
    char MIN_string[16];
    char MAX_string[16];
    char VALUE_string[32];
    char SPACE_string[] = "[SPACE] to pause";
    char SAVE_string[] = "[S] to output";
    int x_mouse, y_mouse;
    double value_mouse;

    int NX = dsa_params->NX; 
    int NY = dsa_params->NY; 
    int skip = dsa_params->skip;
    int cell_size = dsa_params->cell_size;
    double *field = dsa_params->field;

    Color cell_color = {0,0,0,255};
    Color background_color = {0, 0, 0, 150};

    double min_value = dsa_min(dsa_params);
    double max_value = dsa_max(dsa_params);

    Font font = dsa_params->font;

    char *name = dsa_params->name;
    
    current_frame = GetTime();
    FPS = 1.0/(current_frame-dsa_params->last_frame);
    sprintf(FPS_string, "FPS = %.0f", FPS);
    dsa_params->last_frame = current_frame;

    sprintf(MIN_string, "%.3e", min_value);
    sprintf(MAX_string, "%.3e", max_value);

    x_mouse = GetMouseX()/cell_size;
    y_mouse = (NY/skip*cell_size-GetMouseY())/cell_size;
    value_mouse = field[x_mouse*skip*NY+y_mouse*skip];
    sprintf(VALUE_string, "(%d, %d): %.3e", x_mouse*skip, y_mouse*skip, value_mouse);

    BeginDrawing();

        for (int i = 0; i < NX; i+=skip) {
            for (int j = 0; j < NY; j+=skip) {
                fraction = fmin(fmax((field[i*NY + j]-min_value)/(max_value-min_value), 0.0), 1.0);
                dsa_cmap(fraction, &cell_color, dsa_params);
                DrawRectangle(i/skip*cell_size, (NY/skip-1-j/skip)*cell_size, cell_size, cell_size, cell_color);
            }
        }

        DrawRectangle((NX/skip-1)*cell_size-280, 10, 280, 65, background_color);

        for (int i = 0; i < 100; i++) {
            dsa_cmap((double)i/99.0, &cell_color, dsa_params);
            DrawRectangle((NX/skip-1)*cell_size-240 + i*2, 30, 2, 20, cell_color);
        }

        DrawRectangleLines((NX/skip-1)*cell_size-240, 30, 200, 20, BLACK);

        DrawRectangle(10, 10, 200, 90, background_color);
        DrawRectangle(10, NY/skip*cell_size-50, 195, 40, background_color);

        DrawTextEx(font, FPS_string, (Vector2){20.0, 20.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, SPACE_string, (Vector2){20.0, 45.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, SAVE_string, (Vector2){20.0, 70.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, MIN_string, (Vector2){(float)((NX/skip-1)*cell_size-270), 50.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, MAX_string, (Vector2){(float)((NX/skip-1)*cell_size-100), 50.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, name, (Vector2){(float)((NX/skip-1)*cell_size-240), 10.0f}, 20.0, 1.0, RED);
        DrawTextEx(font, VALUE_string, (Vector2){20.0f, (float)(NY/skip*cell_size-40)}, 20.0, 1.0, RED);

    EndDrawing();
    SwapScreenBuffer();

    if (dsa_params->paused) {
        WaitTime(1.0/120.0);
    }
}