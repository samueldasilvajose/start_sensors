#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>

#ifndef SS_PATH_GUI
#define SS_PATH_GUI ""
#endif

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} SsGeometryWindow;

#endif //TYPES_H