#ifndef TYPES_H
#define TYPES_H

#ifndef SS_PATH_GUI
#define SS_PATH_GUI ""
#endif

#include <gtk/gtk.h>

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} SsGeometryWindow;

#endif //TYPES_H