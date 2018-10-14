#include <windows.h>
#include <utility>
#include "Clip.h"
#include "ClipUtil.h"

std::pair<double, double> findPoint(POINT P, POINT Q, double t)
{
	return std::make_pair(t*(Q.x - P.x) + P.x, t*(Q.y - P.y) + P.y);
}

void drawClippedLine()
{
	POINT A, B;
	if (gDrawData.overlap < 0)
	{
		A = gDrawData.clippedLineEndPts[0];
		B = gDrawData.clippedLineEndPts[1];
	}
	else
	{
		A = gDrawData.cornerPts[gDrawData.overlap];
		B = gDrawData.cornerPts[(gDrawData.overlap + 1) % gDrawData.nCornerPts];
	}
	if (gDrawData.nIntrscn < 1)
		return;
	SelectObject(gDrawData.hdcMem, CreatePen(PS_SOLID, 1, CLR_CLIP));
	MoveToEx(gDrawData.hdcMem, A.x, A.y, NULL);
	LineTo(gDrawData.hdcMem, B.x, B.y);
}

void DRAWING_DATA::intersection()
{
	//POINT P = { 0,0 }, POINT Q = { 0,0 };
	POINT &P = clippedLineEndPts[0];
	POINT &Q = clippedLineEndPts[1];
	vec PQ(P, Q), S;
	long numerator, denominator;
	//double t, l;
	overlap = -1;
	nIntrscn = 0;
	for (int i = 0; i < nCornerPts; i++)
	{
		S = vec(cornerPts[i], cornerPts[(i + 1) % nCornerPts]);
		numerator = cross(vec(cornerPts[i], P), PQ);
		denominator = cross(S, PQ);
		if (denominator == 0) //parallel
		{
			if (numerator == 0) //overlapping
			{
				overlap = i;
				return;
			}
			//non intersecting
			continue;
		}
		else if (abs(numerator) > abs(denominator) || (denominator*numerator <= 0))
			continue;	//outside the side
		//t = (double)numerator / denominator;
		numerator = cross(S, vec(P, cornerPts[i]));
		lambda[nIntrscn] = (double)numerator / denominator;
		nIntrscn++;
		if (nIntrscn > 1)
			break;
	}

	switch (nIntrscn)
	{
	case 2:
		if (lambda[0] > lambda[1])
			std::swap(lambda[0], lambda[1]);
		if (lambda[0] > 1 || lambda[1] < 0)
		{
			nIntrscn = 0;
			break;
		}
		else
		{
			if (lambda[0] > 0)
			{
				//std::pair<double, double> dP = findPoint(beginPt, endPt, lambda[0]);
				//P = POINT{ (long)dP.first, (long)dP.second };
				double pxterm = lambda[0] * (endPt.x - beginPt.x),
				       pyterm = lambda[0] * (endPt.y - beginPt.y);
				if (pxterm > 0)
					pxterm = ceil(pxterm);
				else
					pxterm = floor(pxterm);
				if (pyterm > 0)
					pyterm = ceil(pyterm);
				else
					pyterm = floor(pyterm);
				P = { (long)pxterm + beginPt.x, (long)pyterm + beginPt.y };
			}
			if (lambda[1] < 1)
			{
				//std::pair<double, double> dQ = findPoint(beginPt, endPt, lambda[1]);
				//Q = POINT{ (long)dQ.first, (long)dQ.second };
				double pxterm = lambda[1] * (endPt.x - beginPt.x),
				       pyterm = lambda[1] * (endPt.y - beginPt.y);
				if (pxterm > 0)
					pxterm = ceil(pxterm);
				else
					pxterm = floor(pxterm);
				if (pyterm > 0)
					pyterm = ceil(pyterm);
				else
					pyterm = floor(pyterm);
				Q = { (long)pxterm + beginPt.x, (long)pyterm + beginPt.y };
			}
		}
		break;
	case 1:
	{
		std::pair<double, double> dP = findPoint(P, Q, lambda[0]);
		P = Q = POINT{ (long)round(dP.first), (long)round(dP.second) };
		break;
	}
	default:
		break;
	}
}