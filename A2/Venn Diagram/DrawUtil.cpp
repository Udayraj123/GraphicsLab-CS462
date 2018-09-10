#include <windows.h>
#include "Fill.h"

DRAWING_DATA gDrawData; // global data
LRESULT CALLBACK RadiusDialog(HWND hdlg, UINT mess, WPARAM more, LPARAM pos); //function definition for taking inputs from the dialogue box.

void reDraw(HWND hwnd)
{
	InvalidateRect(hwnd, NULL, 1);
}

/* function to plot symmetrical points */
void plot_sympoint(int idx, int ex, int ey, COLORREF clr)
{
	int cx = gDrawData.centerPts[idx].x;  //x-coordinate of the center of the circle
	int cy = gDrawData.centerPts[idx].y;  //y-coordinate of the center of the circle
										  //for plotting the symmetrical points
	SetPixel(gDrawData.hdcMem, ex + cx, cy - ey, clr);
	SetPixel(gDrawData.hdcMem, -ex + cx, cy - ey, clr);
	SetPixel(gDrawData.hdcMem, -ex + cx, cy + ey, clr);
	SetPixel(gDrawData.hdcMem, ex + cx, cy + ey, clr);
	SetPixel(gDrawData.hdcMem, ey + cx, cy - ex, clr);
	SetPixel(gDrawData.hdcMem, -ey + cx, cy - ex, clr);
	SetPixel(gDrawData.hdcMem, -ey + cx, cy + ex, clr);
	SetPixel(gDrawData.hdcMem, ey + cx, cy + ex, clr);
}

/* plots a circle using mid-point method*/
void drawCircle(int idx)
{
	int x = 0;
	int y, radius;
	if (idx == 0)
		radius = gDrawData.r1;
	else if (idx == 1)
		radius = gDrawData.r2;
	else
		radius = gDrawData.r3;
	y = radius;

	plot_sympoint(idx, x, y, CLR_BOUNDARY);         //plot symmetrical points
	double p = 1 - radius;               //decision parameter for drawing circle
	while (x<y) {
		if (p<0)
			x = x + 1;
		else {
			x = x + 1;
			y = y - 1;
		}
		if (p<0)
			p = p + 2.0*x + 1.0;
		else
			p = p + 2.0*(x - y) + 1.0;
		plot_sympoint(idx, x, y, CLR_BOUNDARY);  //plot symmetrical points
	}

}

/* sets the mode of the drawing panel*/
void setDrawMode(MODE mode, HWND hwnd)
{
	gDrawData.drawMode = mode;
	setupMenus(hwnd);
}

/* for creating the bimap
A bitmap is a graphical object used to create,
manipulate (scale, scroll, rotate, and paint),
and store images as files on a disk.*/
void createMemoryBitmap(HDC hdc)
{
	gDrawData.hdcMem = CreateCompatibleDC(hdc);
	gDrawData.hbmp = CreateCompatibleBitmap(hdc,
		gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
	SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
	PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
		gDrawData.maxBoundary.cy, PATCOPY);
}

/* intializing drawing pen colour and fill pen colour*/
void initialize(HWND hwnd, HDC hdc)
{
	gDrawData.nCenters = 0;
	gDrawData.hDrawPen = CreatePen(PS_SOLID, 1, CLR_REST);                //creating pen for drawing
	//creating 5 color pens for filling
	for (int i = 0; i < 5; i++)
		gDrawData.hFillPens[i] = CreatePen(PS_SOLID, 1, penClrs[i]);	
	gDrawData.r1 = gDrawData.r2 = gDrawData.r3 = DEFAULT_RADIUS;
	gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
	gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);
	createMemoryBitmap(hdc);
	setDrawMode(READY_MODE, hwnd);
}

/* For cleaning the drawing panel*/
void cleanup()
{
	DeleteDC(gDrawData.hdcMem);
}

/* resetting the parameters, drawing mode*/
void reset(HWND hwnd)
{
	gDrawData.nCenters = 0;
	gDrawData.drawMode = READY_MODE;

	PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
		gDrawData.maxBoundary.cy, PATCOPY);

	reDraw(hwnd);
	setupMenus(hwnd);
}

/* Adds points to the panel, three circles three points*/
void addCircle(HWND hwnd, int x, int y)
{
	/* the coordinates of the points are stored in an array */
	SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
	//draw center
	SetPixel(gDrawData.hdcMem, x, y, CLR_BOUNDARY);
	//store center points of circle
	gDrawData.centerPts[gDrawData.nCenters].x = x;
	gDrawData.centerPts[gDrawData.nCenters].y = y;
	drawCircle(gDrawData.nCenters);
	gDrawData.nCenters++;
}


void drawImage(HDC hdc)
{
	/*bit-block transfer of the color data corresponding to a rectangle
	of pixels from the specified source device context into a destination
	device context.*/
	BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx,
		gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

/* For processing left button click*/
void processLeftButtonDown(HWND hwnd, int x, int y)
{
	switch (gDrawData.drawMode)
	{
	case DRAW_MODE:
		if (gDrawData.nCenters == nMaxCircles) {
			// Move to fill color directly
			setDrawMode(DRAWN_MODE, hwnd);
			processCommand(ID_FILL, hwnd);//calls redraw
			for (int i = 0; i < 3; i++)
				drawCircle(i);
		}
		else {
			// n < max
			addCircle(hwnd, x, y);          //add point to the drawing panel		
			reDraw(hwnd);
		}
		break;
	case DRAWN_MODE: // clear for next draws
		processCommand(ID_CLEAR, hwnd);
		break;
	default:
		return;
	}
}

void processCommonCommand(int cmd, HWND hwnd)
{
	switch (cmd)
	{
	case ID_CLEAR:
		reset(hwnd);
		setDrawMode(DRAW_MODE, hwnd);
		break;
	case ID_SET_RADII:
		DialogBox(NULL, "MyDB", hwnd, (DLGPROC)RadiusDialog);
		setDrawMode(DRAW_MODE, hwnd);
		break;

	case ID_EXIT:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
}
/* performs action depending upon whether user clicks ok or cancel*/
LRESULT CALLBACK RadiusDialog(HWND hdlg, UINT mess, WPARAM more, LPARAM pos)
{
	char str[20];
	switch (mess)
	{
	case WM_COMMAND:
		switch (more)
		{
		case ID_OK:
			GetDlgItemText(hdlg, ID_EB1, str, 20);             //getting text from the dialogue
			gDrawData.r1 = (long int)(atof(str));
			GetDlgItemText(hdlg, ID_EB2, str, 20);            //getting text from the dialogue
			gDrawData.r2 = (long int)(atof(str));
			GetDlgItemText(hdlg, ID_EB3, str, 20);            //getting text from the dialogue
			gDrawData.r3 = (long int)(atof(str));
			EndDialog(hdlg, TRUE);
			return 1;
			break;

		case ID_CANCEL:
			EndDialog(hdlg, TRUE);
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}
/* processes the messages */
LRESULT CALLBACK WindowF(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam)
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
		setDrawMode(DRAW_MODE, hwnd);
		break;

	case WM_COMMAND:
		processCommand(LOWORD(wParam), hwnd);
		break;

	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		processLeftButtonDown(hwnd, x, y);
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
	/* ensures that every message is processed */
	return DefWindowProc(hwnd, message, wParam, lParam);
}
