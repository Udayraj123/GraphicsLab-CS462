#define ID_SCLINE 40001
#define ID_BLINE  40004
#define ID_CLEAR  40002
#define ID_EXIT   40003

typedef enum
{
  READY_MODE,
  SCLINE_MODE,
  BLINE_MODE,
  DRAWN_MODE
} MODE;

const int nMaxNoOfPts = 2;

typedef struct
{
  HDC hdcMem; 
  HBITMAP hbmp;
  HPEN hDrawPen;
  MODE drawMode;
  POINT pts[nMaxNoOfPts];/* points of the line */
  SIZE maxBoundary;
  int nPts; /* end points of line */ 
} DRAWING_DATA;
