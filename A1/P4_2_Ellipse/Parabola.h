#define ID_PARABOLA  40001
#define ID_CLEAR    40002
#define ID_EXIT     40003
#define ID_OK       40004 
#define ID_CANCEL   40005 
#define ID_EB1      40006
#define ID_EB2      40007
#define ID_LABEL1   40008
#define ID_LABEL2   40009

typedef enum
{
  READY_MODE,
  DRAW_MODE,
  DRAWN_MODE
} MODE;

typedef struct
{
  HDC hdcMem; 
  HBITMAP hbmp;
  HPEN hDrawPen;
  MODE drawMode;
  POINT centre;
  double a;
  int yLim;
  SIZE maxBoundary;
} DRAWING_DATA;
