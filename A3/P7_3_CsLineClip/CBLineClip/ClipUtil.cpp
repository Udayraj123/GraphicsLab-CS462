#include <windows.h>
#include <math.h>
#include "ClipUtil.h"
#include "Clip.h"

DRAWING_DATA gDrawData; // global data

void DRAWING_DATA::reset()
{
	beginPt.x = beginPt.y = 0;
	endPt.x = endPt.y = 0;

	clippedLineEndPts[0].x = clippedLineEndPts[0].y = 0;
	clippedLineEndPts[1].x = clippedLineEndPts[1].y = 0;

	nCornerPts = 0;
	orientation = 0;

	drawMode = READY_MODE;
}

int signum(long L)
{
	return (L > 0) ? 1 : ((L < 0) ? -1 : 0);
}

bool DRAWING_DATA::checkForConvexity(int x, int y)
{
	long startPointCP, prevPointCP, currPointCP;
	switch (nCornerPts)
	{
	case 0:
	case 1:
		return true;
	default:
		startPointCP = (cornerPts[0].x - x)*(cornerPts[1].y - cornerPts[0].y) - (cornerPts[1].x - cornerPts[0].x)*(cornerPts[0].y - y);
		startPointCP = signum(startPointCP);
		if (nCornerPts == 2)
			break;
		prevPointCP = (cornerPts[nCornerPts - 1].x - cornerPts[nCornerPts - 2].x)*(y - cornerPts[nCornerPts - 1].y) - (x - cornerPts[nCornerPts - 1].x)*(cornerPts[nCornerPts - 1].y - cornerPts[nCornerPts - 2].y);
		prevPointCP = signum(prevPointCP);
		currPointCP = (x - cornerPts[nCornerPts - 1].x)*(cornerPts[0].y - y) - (cornerPts[0].x - x)*(y - cornerPts[nCornerPts - 1].y);
		currPointCP = signum(currPointCP);
	}
	if (orientation == 0)
	{
		orientation = startPointCP;
		return true;
	}
	return !(startPointCP*orientation < 0 || prevPointCP * orientation < 0 || currPointCP * orientation < 0);
}

void reDraw(HWND hwnd)
{
	InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
	SetPixel(gDrawData.hdcMem, x, y, CLR_POLY);
}

void drawPartialPoly(HDC hdc = gDrawData.hdcMem)
{
	MoveToEx(hdc, gDrawData.cornerPts[0].x, gDrawData.cornerPts[0].y, NULL);
	for (int i = 1; i < gDrawData.nCornerPts; i++)
	{
		LineTo(hdc, gDrawData.cornerPts[i].x, gDrawData.cornerPts[i].y);
	}
}

void drawPoly(HDC hdc = gDrawData.hdcMem)
{
	SelectObject(hdc, CreatePen(PS_SOLID, 1, CLR_POLY));
	drawPartialPoly(hdc);
	LineTo(hdc, gDrawData.cornerPts[0].x, gDrawData.cornerPts[0].y);
}

void setupMenus(HWND hwnd)
{
	HMENU hmenu, hsubmenu;
	hmenu = GetMenu(hwnd);
	hsubmenu = GetSubMenu(hmenu, 0);

	switch (gDrawData.drawMode)
	{
	case READY_MODE:
		//enable 'Draw Rectangle',disable 'Draw Line','Clip' menus
		EnableMenuItem(hsubmenu, ID_DRAW_POLYGON, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hsubmenu, ID_DRAW_LINE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_CLIP, MF_BYCOMMAND | MF_GRAYED);
		break;
	case DRAWN_POLYGON_MODE:
		//enable 'Draw Line',disable 'Draw Rectangle','Clip' menus
		EnableMenuItem(hsubmenu, ID_DRAW_POLYGON, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_DRAW_LINE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hsubmenu, ID_CLIP, MF_BYCOMMAND | MF_GRAYED);
		break;
	case DRAW_LINE_MODE:
		//enable 'Clip',disable 'Draw Line, Rectangle' menus
		EnableMenuItem(hsubmenu, ID_DRAW_POLYGON, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_DRAW_LINE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_CLIP, MF_BYCOMMAND | MF_ENABLED);
		break;

	case DRAW_POLYGON_MODE:
	case CLIP_MODE:
		//disable 'Clip' and 'Draw Line, Rectangle' menus
		EnableMenuItem(hsubmenu, ID_DRAW_POLYGON, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_DRAW_LINE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_CLIP, MF_BYCOMMAND | MF_GRAYED);
		break;
	case CLIPPED_MODE:
		EnableMenuItem(hsubmenu, ID_DRAW_POLYGON, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hsubmenu, ID_DRAW_LINE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hsubmenu, ID_CLIP, MF_BYCOMMAND | MF_GRAYED);
		break;
	}
}

void setDrawMode(MODE mode, HWND hwnd)
{
	gDrawData.drawMode = mode;
	setupMenus(hwnd);
}

void createMemoryBitmap(HDC hdc)
{
	gDrawData.hdcMem = CreateCompatibleDC(hdc);
	gDrawData.hbmp = CreateCompatibleBitmap(hdc, gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
	SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
	PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
	gDrawData.reset();

	gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
	gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);

	createMemoryBitmap(hdc);
	setupMenus(hwnd);
}

void reset(HWND hwnd)
{
	gDrawData.reset();

	PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy, PATCOPY);

	reDraw(hwnd);
	setupMenus(hwnd);
}

void cleanup()
{
	DeleteDC(gDrawData.hdcMem);
}

void addPointToPolygon(HWND hwnd, int x, int y)
{
	/* the coordinates of the points are stored in an array */

	if (gDrawData.nCornerPts < nMaxNoOfCornerPts)
	{
		SelectObject(gDrawData.hdcMem, CreatePen(PS_SOLID, 1, CLR_POLY));
		drawPoint(x, y);
		gDrawData.cornerPts[gDrawData.nCornerPts].x = x;
		gDrawData.cornerPts[gDrawData.nCornerPts].y = y;
		gDrawData.nCornerPts++;
	}
	else
	{
		MessageBox(hwnd,
			"Maximum number of points reached, closing the polygon",
			"Warning", MB_OK);
		processRightButtonDown(hwnd);
	}
	drawPartialPoly();
}

void drawImage(HDC hdc)
{
	BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

void performRubberBanding(HWND hwnd, int x, int y)
{
	int r = 0;
	HDC hdc = GetDC(hwnd);

	switch (gDrawData.drawMode)
	{
	case DRAW_LINE_MODE:
		//drawLineSegment(hdc, gDrawData.beginPt, gDrawData.endPt, CLR_BG);
		MoveToEx(hdc, gDrawData.beginPt.x, gDrawData.beginPt.y, NULL);
		SelectObject(hdc, CreatePen(PS_SOLID, 1, CLR_BG));
		LineTo(hdc, gDrawData.endPt.x, gDrawData.endPt.y);
		//drawRectangle(hdc, gDrawData.rectCornerPts[0], gDrawData.rectCornerPts[1], CLR_RECT);
		gDrawData.endPt.x = x;
		gDrawData.endPt.y = y;
		//drawLineSegment(hdc, gDrawData.beginPt, gDrawData.endPt, CLR_LINE);
		drawPoly(hdc);
		MoveToEx(hdc, gDrawData.beginPt.x, gDrawData.beginPt.y, NULL);
		SelectObject(hdc, CreatePen(PS_SOLID, 1, CLR_LINE));
		LineTo(hdc, gDrawData.endPt.x, gDrawData.endPt.y);
		//UpdateWindow(hwnd);
		break;

		//case DRAW_RECTANGLE_MODE:
		//	//drawRectangle(hdc, gDrawData.beginPt, gDrawData.endPt, CLR_BG);
		//	gDrawData.endPt.x = x;
		//	gDrawData.endPt.y = y;
		//	//drawRectangle(hdc, gDrawData.beginPt, gDrawData.endPt, CLR_RECT);
		//	break;
	default:
		break;
	}
	ReleaseDC(hwnd, hdc);
}

void processRightButtonDown(HWND hwnd)
{
	if (gDrawData.drawMode == DRAW_POLYGON_MODE)
	{
		//Beep(777, 500);
		drawPoly();
		setDrawMode(DRAWN_POLYGON_MODE, hwnd);
		reDraw(hwnd);
	}
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
	switch (gDrawData.drawMode)
	{
		/*case DRAW_POLYGON_MODE:
			if (gDrawData.checkForConvexity(x, y))
			{
				addPointToPolygon(hwnd, x, y);
				reDraw(hwnd);
			}
			else
				Beep(500, 200);
			break;*/
	case DRAW_LINE_MODE:
	//case DRAW_RECTANGLE_MODE:
		gDrawData.beginPt.x = gDrawData.endPt.x = x;
		gDrawData.beginPt.y = gDrawData.endPt.y = y;
		reDraw(hwnd);
		break;

	default:
		return;
	}
}

void processLeftButtonUp(HWND hwnd, int x, int y)
{
	switch (gDrawData.drawMode)
	{
	case DRAW_POLYGON_MODE:
		if (gDrawData.checkForConvexity(x, y))
		{
			addPointToPolygon(hwnd, x, y);
			reDraw(hwnd);
		}
		else
			Beep(500, 200);
		break;

	default:
		return;
	}
}

void processMouseMove(HWND hwnd, int x, int y)
{
	switch (gDrawData.drawMode)
	{
	case DRAW_LINE_MODE:
	//case DRAW_RECTANGLE_MODE:
		performRubberBanding(hwnd, x, y);
		break;
	default:
		break;
	}
}

void draw()
{
	switch (gDrawData.drawMode)
	{
	/*case DRAW_LINE_MODE:
		gDrawData.lineEndPts[0].x = gDrawData.beginPt.x;
		gDrawData.lineEndPts[0].y = gDrawData.beginPt.y;

		gDrawData.lineEndPts[1].x = gDrawData.endPt.x;
		gDrawData.lineEndPts[1].y = gDrawData.endPt.y;

		drawLineSegment(gDrawData.hdcMem, gDrawData.beginPt, gDrawData.endPt, CLR_LINE);
		break;

	case DRAW_RECTANGLE_MODE:
		gDrawData.rectCornerPts[0].x = gDrawData.beginPt.x;
		gDrawData.rectCornerPts[0].y = gDrawData.beginPt.y;

		gDrawData.rectCornerPts[1].x = gDrawData.endPt.x;
		gDrawData.rectCornerPts[1].y = gDrawData.endPt.y;

		drawRectangle(gDrawData.hdcMem, gDrawData.rectCornerPts[0], gDrawData.rectCornerPts[1], CLR_RECT);
		break;*/

	default:
		return;
	}
}

void processCommonCommand(int cmd, HWND hwnd)
{
	int response;
	switch (cmd)
	{
	case ID_CLEAR:
		reset(hwnd);
		break;
	case ID_EXIT:
		response =
			MessageBox(hwnd, "Quit the program?", "EXIT", MB_YESNO);
		if (response == IDYES)
			PostQuitMessage(0);
		break;
	default:
		break;
	}
}

void swap(LONG& x, LONG& y)
{
	LONG tmp;
	tmp = x;
	x = y;
	y = tmp;
}

bool isValidRectangle(POINT a, POINT b)
{
	if ((abs(a.x - b.x) >= 10) &&
		(abs(a.y - b.y) >= 10))
		return true;
	else
		return false;
}

bool isValidLine(POINT a, POINT b)
{
	double length;
	double xDiff, yDiff;
	xDiff = a.x - b.x;
	yDiff = a.y - b.y;
	length = sqrt(xDiff * xDiff + yDiff * yDiff);
	if (length >= 10)
		return true;
	else
		return false;
}

void processCommand(int cmd, HWND hwnd)
{
	switch (cmd)
	{
	case ID_DRAW_LINE:
		if (gDrawData.orientation!=0)
		{
			//valid polygon
			draw();
			setDrawMode(DRAW_LINE_MODE, hwnd);
		}
		else
		{
			MessageBox(hwnd, "Please draw a valid Polygon!", "Warning", MB_OK);
			reset(hwnd);
			setDrawMode(DRAW_POLYGON_MODE,hwnd);
			setupMenus(hwnd);
		}
		break;
	case ID_DRAW_POLYGON:
		setDrawMode(DRAW_POLYGON_MODE, hwnd);
		break;
	case ID_CLIP:
		if (isValidLine(gDrawData.beginPt, gDrawData.endPt))
		{
			// valid line, then switch to next mode
			// draw the line first as drawn through rubber banding
			//draw();
			gDrawData.clippedLineEndPts[0] = gDrawData.beginPt;
			gDrawData.clippedLineEndPts[1] = gDrawData.endPt;
			setDrawMode(CLIP_MODE, hwnd);
			gDrawData.intersection();
			drawClippedLine();
			reDraw(hwnd);
		}
		else
		{
			MessageBox(hwnd, "Please draw a valid line of length at least 10 pixels", "Warning!", MB_OK);
		}
		break;
	default:
		processCommonCommand(cmd, hwnd);
		return;
	}
}

LRESULT CALLBACK WindowF(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int x, y;

	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		initialize(hwnd, hdc);
		ReleaseDC(hwnd, hdc);
		break;

	case WM_COMMAND:
		processCommand(LOWORD(wParam), hwnd);
		break;

	case WM_RBUTTONDOWN:
		processRightButtonDown(hwnd);
		break;

	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		processLeftButtonDown(hwnd, x, y);
		break;

	case WM_LBUTTONUP:
		//drawRectangle(GetDC(hwnd), gDrawData.beginPt, gDrawData.endPt, CLR_RECT);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		processLeftButtonUp(hwnd, x, y);
		break;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON)
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			performRubberBanding(hwnd, x, y);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		drawImage(hdc);
		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		cleanup();
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	// Call the default window handler
	return DefWindowProc(hwnd, message, wParam, lParam);
}
