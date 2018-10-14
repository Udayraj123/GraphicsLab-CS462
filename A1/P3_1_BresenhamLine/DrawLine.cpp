#include <windows.h>
#include "Line.h"
#include "DrawLine.h"

DRAWING_DATA gDrawData; // global data

COLORREF clrScanLine = RGB(0,0,255);
COLORREF clrBresLine = RGB(0,0,0);

void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
	//red points
  Ellipse(gDrawData.hdcMem,x-2,y-2,x+2,y+2);
}

void setupMenus(HWND hwnd)
{
  HMENU hmenu, hsubmenu;
  hmenu=GetMenu(hwnd);
  hsubmenu=GetSubMenu(hmenu, 0);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      // enable Scan and Bresenham's line' menus
      EnableMenuItem(hsubmenu, ID_SCLINE,
                     MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(hsubmenu, ID_BLINE,
                     MF_BYCOMMAND|MF_ENABLED);
      break;
    case SCLINE_MODE :
    case BLINE_MODE:
    case DRAWN_MODE:
      // disable Scan and Bresenham's line' menus
      EnableMenuItem(hsubmenu, ID_SCLINE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_BLINE,
                     MF_BYCOMMAND|MF_GRAYED);
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
  gDrawData.hbmp = CreateCompatibleBitmap(hdc, 
       gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
           gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

  gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
  gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);

  createMemoryBitmap(hdc);
  setDrawMode(READY_MODE, hwnd);
}

void cleanup()
{
  DeleteDC(gDrawData.hdcMem);
}

void reset(HWND hwnd)
{
  for (int i = 0; i < nMaxNoOfPts; i++)
    gDrawData.pts[i].x = gDrawData.pts[i].y = 0;

  gDrawData.drawMode = READY_MODE;
  gDrawData.nPts = 0;

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);

  reDraw(hwnd);
  setupMenus(hwnd);
}

void drawLine()
{
  switch(gDrawData.drawMode)
  {
    case SCLINE_MODE:
      drawScanLineSegment(gDrawData.hdcMem, gDrawData.pts[0], 
                          gDrawData.pts[1], clrScanLine);
      break;

    case BLINE_MODE:
      drawLineSegment(gDrawData.hdcMem, gDrawData.pts[0], 
                      gDrawData.pts[1], clrBresLine);
      break;

    default:
      break;
  }
}

void addPoint(HWND hwnd, int x, int y)
{
  /* the coordinates of the points are stored 
    in two array for x-ordinate and for y-ordinate*/

  if (gDrawData.nPts < nMaxNoOfPts)
  {
    SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
    drawPoint(x,y);
    gDrawData.pts[gDrawData.nPts].x = x;
    gDrawData.pts[gDrawData.nPts].y = y;
    gDrawData.nPts++;
    if (gDrawData.nPts == 2)
    {
      drawLine();
      setDrawMode(DRAWN_MODE, hwnd);
    }
  }
  else
  {
    MessageBox(hwnd,
     "Line already drawn, now you can clear the area", 
            "Warning",MB_OK);
  }
}

void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx, 
       gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case SCLINE_MODE:
    case BLINE_MODE:
      addPoint(hwnd,x,y);
      reDraw(hwnd);
      break;

    default:
      return;
  }
}

void processCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      setDrawMode(READY_MODE, hwnd);
      break;
    case ID_SCLINE:
      setDrawMode(SCLINE_MODE, hwnd);
      break;
    case ID_BLINE:
      setDrawMode(BLINE_MODE, hwnd);
      break;
    case ID_EXIT:
        //response=MessageBox(hwnd,"Quit the program?", "EXIT", MB_YESNO);
        //if(response==IDYES) 
        PostQuitMessage(0);
        break;
    default:
      break;
  }
}

LRESULT CALLBACK WindowF(HWND hwnd,UINT message,WPARAM wParam,
                          LPARAM lParam) 
{
  HDC hdc;
  PAINTSTRUCT ps;
  int x,y;

  switch(message)
  {
    case WM_CREATE:
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      ReleaseDC(hwnd, hdc);
      break;
    
    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;

    case WM_LBUTTONDOWN:
      x = LOWORD(lParam);
      y = HIWORD(lParam);
      processLeftButtonDown(hwnd, x,y);
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
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}
