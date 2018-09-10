#define ID_SET_RADII       40001
#define ID_CLEAR           40002
#define ID_EXIT            40003
#define ID_FILL            40004
#define ID_CANCEL   40005 
#define ID_EB1      40006
#define ID_EB2      40007
#define ID_EB3      40008
#define ID_LABEL1   40009
#define ID_LABEL2   40010
#define ID_LABEL3   40011
#define ID_OK       40012

/* Defining colours constant*/
const int DEFAULT_RADIUS = 150;
const COLORREF CLR_FILL = RGB(0, 0, 200);
const COLORREF CLR_BOUNDARY = RGB(0, 0, 0);
const COLORREF CLR_BG = RGB(255, 255, 255);
const COLORREF CLR_XY = RGB(0, 0, 255);
const COLORREF CLR_YZ = RGB(0, 255, 0);
const COLORREF CLR_ZX = RGB(255, 255, 0); //yellow
const COLORREF CLR_XYZ = RGB(255, 0, 0);
const COLORREF CLR_REST = RGB(0, 255, 255);
const COLORREF penClrs[] = {CLR_REST,CLR_XY,CLR_YZ,CLR_ZX,CLR_XYZ};

/* mode enum*/
typedef enum
{
  READY_MODE,
  DRAW_MODE,
  DRAWN_MODE,
  FILL_MODE,
  FILLED_MODE
} MODE;

const int nMaxCircles = 3;

typedef struct
{
  HDC hdcMem; 
  HBITMAP hbmp;
  HPEN hDrawPen, hFillPens[5];
  MODE drawMode;
  POINT centerPts[nMaxCircles];/* corner points of polygon */
  SIZE maxBoundary;
  int nCenters; /* number of corner points of the polygon */ 
  int r1;
  int r2;
  int r3;
} DRAWING_DATA;

typedef struct
{
  int yMin, yMax;
  double x;
  double mInv;
} EDGE_ENTRY;

extern DRAWING_DATA gDrawData; // global data being shared across files

void setupMenus(HWND hwnd);
void processRightButtonDown(HWND hwnd);
void processCommand(int cmd, HWND hwnd);
void setDrawMode(MODE mode, HWND hwnd);
void reDraw(HWND hwnd);
void processCommonCommand(int cmd, HWND hwnd);
