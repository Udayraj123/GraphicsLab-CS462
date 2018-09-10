#include <windows.h>
#include "Fill.h"

void fill();

double roundVal(double x)
{
  int ix;
  if (x >= 0) 
    ix = (int)(x + 0.5);
  else
    ix = (int)(x - 0.5);
  return (double)(ix);
}

void swap(int &x, int &y)
{ 
  int tmp;
  tmp = x;
  x = y;
  y = tmp;
}

void setupMenus(HWND hwnd)
{
  HMENU hmenu, hsubmenu;
  hmenu=GetMenu(hwnd);
  hsubmenu=GetSubMenu(hmenu, 0);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      // enable 'Draw Polygon', disable 'Filling' menu
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAW_MODE :
    case FILL_MODE :
    case FILLED_MODE :
      // disable 'Draw Polygon', 'Filling' menu 
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAWN_MODE :
      // enable 'Filling' menus, disable 'Draw Polygon' menu
      EnableMenuItem(hsubmenu,ID_DRAW_POLY,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu,ID_FILL,
                     MF_BYCOMMAND|MF_ENABLED);
      break;
  }
}

void performFilling(HWND hwnd)
{
  setDrawMode(FILL_MODE, hwnd);
  SelectObject(gDrawData.hdcMem, gDrawData.hFillPen);
  fill();
  reDraw(hwnd);
  setDrawMode(FILLED_MODE, hwnd);
}

void processCommand(int cmd, HWND hwnd)
{
  switch(cmd)
  {
    case ID_FILL:
      performFilling(hwnd);
      break;
    default:
      processCommonCommand(cmd, hwnd);
      break;
  }
}

bool adjustHorizontallyForBorderPixel(int& x, int y)
{
  if (GetPixel(gDrawData.hdcMem, x, y) != CLR_BOUNDARY)  
  {
    // if (x,y) is not R(border), R could be on either left or right
    if (GetPixel(gDrawData.hdcMem, x-1, y) == CLR_BOUNDARY)  
      x--;
    else if (GetPixel(gDrawData.hdcMem, x+1, y) == CLR_BOUNDARY)  
      x++;
    else
    {
      // neither self or left or right neighbor contains border, skip
      return false; 
    }
  }
  return true;   
}   

bool adjustForDrawingLine(int& x1, int&x2, int y)
{
  // make adjustments for special cases
  if (adjustHorizontallyForBorderPixel(x1, y) == false)
    return false;  
  // start with R...
  if (adjustHorizontallyForBorderPixel(x2, y) == false)
    return false;  
  // end with R...
  // now we have R...R    
  if (x2>x1)
  {
    // skip RRR.. to move to R...
    while ((GetPixel(gDrawData.hdcMem, x1, y)) == CLR_BOUNDARY)
    {
      x1++;
    }
    while ((GetPixel(gDrawData.hdcMem, x2, y)) == CLR_BOUNDARY)
    {
      x2--;
    }
    x2++;
    if (x2>x1)
    {
      // x1 is set at W (not R), x2 set at R
      // Windows API LineTo draws upto not including x2
      return true; 
    }
  }
  return false; // no point of drawing line
}

void drawLine(int x1, int x2, int y)
{
  // draw fill-line from (x1,y) to (x2-1,y)
  if (adjustForDrawingLine(x1, x2, y))
  {
    MoveToEx (gDrawData.hdcMem, x1, y, NULL);
    LineTo (gDrawData.hdcMem, x2, y);
  }
}

int edge_cmp(const void *lvp, const void *rvp)
{
  /* convert from void pointers to structure pointers */
  const EDGE_ENTRY *lp = (const EDGE_ENTRY *)lvp;
  const EDGE_ENTRY *rp = (const EDGE_ENTRY *)rvp;

  /* if the y minimum values are different, compare on minimum y */
  if (lp->yMin != rp->yMin)
    return lp->yMin - rp->yMin;

  /* otherwise, if the current x values are different, 
     compare on current x */
  return ((int)(roundVal(lp->x))) - ((int)(roundVal(rp->x)));
}

void fill()
{
  EDGE_ENTRY *pGET; // global edge table 
  int     nGET = 0; // no of records in global edge table 
  int     cge = 0;  // index to current global edge table entry

  EDGE_ENTRY *pAET; // active edge table 
  int nAET = 0; // no of records in active edge table 
  int x1, y1, x2, y2;// begin and end points of an edge
  int     i, y, left, right;

  if (gDrawData.nCornerPts < 3) 
  {
    // error, polygons require at least three edges (triangle) 
    return;
  }
  pGET = (EDGE_ENTRY *) calloc(gDrawData.nCornerPts, 
                               sizeof(EDGE_ENTRY));
  pAET = (EDGE_ENTRY *) calloc(gDrawData.nCornerPts, 
                               sizeof(EDGE_ENTRY));

  if ((pGET == NULL) || (pAET == NULL)) 
  {
    //error, couldn't allocate one or both of the needed tables 
    if (pGET)
      free(pGET);
    if (pAET)
      free(pAET);
    return;
  }
  // setup the global edge table 
  for (i = 0; i < gDrawData.nCornerPts; i++) 
  {
    x1 = gDrawData.cornerPts[i].x;
    y1 = gDrawData.cornerPts[i].y;    
    x2 = gDrawData.cornerPts[(i + 1) % 
                      gDrawData.nCornerPts].x;
    y2 = gDrawData.cornerPts[(i + 1) % 
                      gDrawData.nCornerPts].y;
    if (y1 != y2) 
    {
      if (y1 > y2) 
      {
        swap(x1, x2);
        swap(y1, y2);
      }
      pGET[nGET].yMin = y1;//corresponds to point having min y
      pGET[nGET].x = x1; //corresponds to point having min y
      pGET[nGET].yMax = y2;//corresponds to point having max y         
      pGET[nGET].mInv = x2 - x1;
      pGET[nGET].mInv /= y2 - y1;
      nGET++;
    }
  }

  qsort(pGET, nGET, sizeof(EDGE_ENTRY), edge_cmp);

  /* start with the lowest y in the table */
  y = pGET[0].yMin;

  do 
  {
    /* add edges to the active table from the global table */
    while ((nGET > 0) && (pGET[cge].yMin == y)) 
    {
      memcpy(&(pAET[nAET]), &(pGET[cge++]), sizeof(EDGE_ENTRY));
      nGET--;
      pAET[nAET++].yMin = 0;
    }
    qsort(pAET, nAET, sizeof(EDGE_ENTRY), edge_cmp);

    /* using odd parity, connect alternating line segments */
    for (i = 1; i < nAET; i += 2) 
    {
      left = (int) (roundVal(pAET[i - 1].x));
      right = (int)(roundVal(pAET[i].x));
      if (right > left)
        drawLine(left, right, y);
    }
    /* go to the next scan line */
    y++;

    /* remove inactive edges from the active edge table */
    /* or update the current x position of active edges */
    for (i = 0; i < nAET; ++i) 
    {
      if (pAET[i].yMax == y)
        memcpy(&(pAET[i--]), &(pAET[--nAET]), 
               sizeof(EDGE_ENTRY));
      else 
      {
        pAET[i].x += pAET[i].mInv ;
      }
    }
    /* keep doing this while there are any   edges left */
  } while ((nAET > 0) || (nGET > 0));

  /* all done, free the edge tables */
  free(pGET);
  free(pAET);
}
