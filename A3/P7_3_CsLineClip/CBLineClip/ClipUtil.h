#define ID_DRAW_POLYGON   40001
#define ID_CLEAR          40002
#define ID_EXIT           40003
#define ID_DRAW_LINE      40004
#define ID_CLIP           40005

const COLORREF CLR_BG = RGB(255, 255, 255);
const COLORREF CLR_POLY = RGB(255, 0, 0);
const COLORREF CLR_LINE = RGB(0, 0, 0);
const COLORREF CLR_CLIP = RGB(0, 0, 255);

enum MODE
{
	READY_MODE,
	//DRAW_RECTANGLE_MODE,
	DRAW_POLYGON_MODE,
	DRAWN_POLYGON_MODE,
	DRAW_LINE_MODE,
	CLIP_MODE,
	CLIPPED_MODE
};

const int nMaxNoOfCornerPts = 100;

class DRAWING_DATA
{
public:
	HDC hdcMem;
	HBITMAP hbmp;
	MODE drawMode;
	POINT beginPt, endPt;

	POINT cornerPts[nMaxNoOfCornerPts];/* corner points of polygon */
	SIZE maxBoundary;
	int orientation;
	int nCornerPts; /* number of corner points of the polygon */

	int overlap;
	int nIntrscn;
	double lambda[2];
	POINT clippedLineEndPts[2];

	void reset();
	bool checkForConvexity(int, int);
	void intersection();
};

extern DRAWING_DATA gDrawData; // global data

void reDraw(HWND hwnd);
void setDrawMode(MODE mode, HWND hwnd);
void processRightButtonDown(HWND hwnd);
void drawClippedLine();