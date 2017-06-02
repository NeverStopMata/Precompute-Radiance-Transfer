#include "Ray.h"



Ray::Ray(vec3 vertexPos, vec3 direction, vec3 normal = vec3(0,0,0))
{
	this->direction = normalize(direction);
	this->source = vertexPos + normal * EPSILON;
}


Ray::~Ray()
{
}

int Ray::IsBlocked(vector<Triangle> trianleList, int lastBlockNum)
{
	int numTriangle = trianleList.size();
	int i = lastBlockNum;
	do
	{
		//if (dot(trianleList[i].faceNormal, this->direction) > 0)//去掉背面对着该光线的三角面
		//{
		//	i = (i + 1) % numTriangle;
		//	continue;
		//}
		float distance = dot(trianleList[i].vertices[0] - this->source, trianleList[i].faceNormal) /
			dot(trianleList[i].faceNormal, this->direction);
		if (distance < 0.5 * EPSILON)//去掉在射线反方向的三角面
		{
			i = (i + 1) % numTriangle;
			continue;
		}
		vec3 crossPos = this->source + this->direction * distance;
		vec3 PA = trianleList[i].vertices[0] - crossPos;
		vec3 PB = trianleList[i].vertices[1] - crossPos;
		vec3 PC = trianleList[i].vertices[2] - crossPos;
		float angle1 = acos(dot(normalize(PA), normalize(PB)));
		float angle2 = acos(dot(normalize(PB), normalize(PC)));
		float angle3 = acos(dot(normalize(PC), normalize(PA)));
		if (angle1 + angle2 + angle3 >= 2*M_PI - 0.5 * EPSILON)
			return i;
		else
		{
			i = (i + 1) % numTriangle;
			continue;
		}
		
	} while (i != lastBlockNum);
	return -1;
}