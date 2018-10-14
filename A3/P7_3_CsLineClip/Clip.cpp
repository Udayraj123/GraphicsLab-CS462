#include <windows.h>
#include "Line.h"
#include "ClipUtil.h"

int code(double x, double y)
{
  return ( ((x < gDrawData.clipMin.x) << 3) +
           ((x > gDrawData.clipMax.x) << 2) +
           ((y < gDrawData.clipMin.y) << 1) +
            (y > gDrawData.clipMax.y));
}

void scLineClip(POINT start, POINT end)
{
  bool bVerticalLine;
  float m;
  double x1Clip, y1Clip, x2Clip, y2Clip;
  int c1,c2;  
  POINT clipStart, clipEnd;

  // groundwork before drawing contiguous line segments for clipping
  setupLineSegmentDrawing(gDrawData.hdcMem, start, end);
  calculateSlope(start, end, bVerticalLine, m);

  x1Clip = start.x;
  y1Clip = start.y;
  x2Clip = end.x;
  y2Clip = end.y;

  c1 = code(x1Clip, y1Clip); /* region code of start point*/ 
  c2 = code(x2Clip, y2Clip); /* region code of end point*/

  while (c1 | c2)  
  {
    /* two points are not inside the rectangle */
    if(c1 & c2) 
    {
      // line is completely outside, erase entire line
      drawNextLineSegment(end, CLR_BG);
      return;
    }
    if(c1) 
    {
      if(c1&8)
      {
        y1Clip += m * (gDrawData.clipMin.x - x1Clip);
        x1Clip = gDrawData.clipMin.x;                 
      }
      else if(c1&4)
      {
        y1Clip += m * (gDrawData.clipMax.x - x1Clip);
        x1Clip = gDrawData.clipMax.x;           
      }
      else if(c1&2)
      {
        if (!bVerticalLine)
          x1Clip += (gDrawData.clipMin.y - y1Clip)/m;
        y1Clip = gDrawData.clipMin.y;           
      }
      else if(c1&1)
      {
        if (!bVerticalLine)
          x1Clip += (gDrawData.clipMax.y - y1Clip)/m;
        y1Clip = gDrawData.clipMax.y;           
      }
      c1 = code(x1Clip, y1Clip);
    }//End OF if(c1)
    else // c1 is false, so c2 must be true
    {
      if(c2&8)
      {
        y2Clip += m * (gDrawData.clipMin.x - x2Clip);
        x2Clip = gDrawData.clipMin.x;                
      }
      else if(c2&4)
      {
        y2Clip += m * (gDrawData.clipMax.x - x2Clip);
        x2Clip = gDrawData.clipMax.x;
      }
      else if(c2&2)
      {
        if (!bVerticalLine)
          x2Clip += (gDrawData.clipMin.y - y2Clip)/m;
        y2Clip = gDrawData.clipMin.y;           
      }
      else if(c2&1) 
      {
        if (!bVerticalLine)
          x2Clip += (gDrawData.clipMax.y - y2Clip)/m;
        y2Clip = gDrawData.clipMax.y;           
      }
      c2 = code(x2Clip, y2Clip);
    }//End OF else
  }//End of while
  clipStart.x = (long) (x1Clip);
  clipStart.y = (long) (y1Clip);
  clipEnd.x = (long) (x2Clip);
  clipEnd.y = (long) (y2Clip);

  drawNextLineSegment(clipStart, CLR_BG);
  drawNextLineSegment(clipEnd, CLR_LINE);
  drawNextLineSegment(end, CLR_BG);
  performCorrectionAtClipPts(gDrawData.hdcMem, clipStart,
                             CLR_LINE, CLR_BG);
  performCorrectionAtClipPts(gDrawData.hdcMem, clipEnd,
                             CLR_LINE, CLR_BG); 
}

void clip(HWND hwnd)
{
  scLineClip(gDrawData.lineEndPts[0], gDrawData.lineEndPts[1]);
  reDraw(hwnd);
  setDrawMode(CLIPPED_MODE, hwnd);
}
