class vec : public POINT
{
public:
	vec(long x, long y) : POINT{x,y}
	{}

	vec(POINT A, POINT B) : POINT{B.x - A.x, B.y - A.y}
	{}

	vec() : POINT {0,0}
	{}

	friend LONG cross(vec A, vec B)
	{
		return A.x*B.y - B.x*A.y;
	}
};