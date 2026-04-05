#ifndef TYPES_H
#define TYPES_H

#ifndef SS_PATH_GUI
#define SS_PATH_GUI ""
#endif

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkTextBuffer GtkTextBuffer;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} SsGeometryWindow;

#endif //TYPES_H