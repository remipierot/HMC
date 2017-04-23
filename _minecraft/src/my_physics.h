#pragma once

#include "engine/utils/types_3d.h"

inline bool planeIntersect(NYVert3Df& start, NYVert3Df& end, NYVert3Df& planePoint1, NYVert3Df& planePoint2, NYVert3Df& planePoint3, NYVert3Df& intersect)
{
	NYVert3Df planeNormal = (planePoint3 - planePoint1).vecProd(planePoint2 - planePoint1).normalize();
	float d = -planeNormal.scalProd(planePoint3);

	float div = planeNormal.X*(start.X - end.X) + planeNormal.Y*(start.Y - end.Y) + planeNormal.Z*(start.Z - end.Z);
	float t = -1;

	if (div != 0)
	{
		t = (planeNormal.X*start.X + planeNormal.Y*start.Y + planeNormal.Z*start.Z + d) / div;

		if (t >= 0 && t <= 1)
		{
			intersect = start + (end - start) * t;
		}
	}

	return (t >= 0 && t <= 1);
}

inline bool faceIntersect(NYVert3Df& A, NYVert3Df& B, NYVert3Df& C, NYVert3Df& D, NYVert3Df& P)
{
	NYVert3Df vect1 = (B - A).vecProd(P - A);
	NYVert3Df vect2 = (C - B).vecProd(P - B);
	NYVert3Df vect3 = (D - C).vecProd(P - C);
	NYVert3Df vect4 = (A - D).vecProd(P - D);
	
	return vect1.scalProd(vect2) > 0 &&
		vect2.scalProd(vect3) > 0 &&
		vect3.scalProd(vect4) > 0;
}

/*
bool cubeIntersect(NYVert3Df start, NYVert3Df end, NYVert3Df* intersect)
{
	NYVert3Df cubePos;
	int cubeResolution = (int)((end - start).getMagnitude() / NYCube::CUBE_SIZE);
	
	for (int x = -(cubeResolution / 2); x < (cubeResolution / 2); x++)
	{
		for (int y = -(cubeResolution / 2); y < (cubeResolution / 2); y++)
		{
			for (int z = -(cubeResolution / 2); z < (cubeResolution / 2); z++)
			{
				cubePos = NYVert3Df(
					(((int)start.X / NYCube::CUBE_SIZE) * NYCube::CUBE_SIZE) + x * NYCube::CUBE_SIZE,
					(((int)start.Y / NYCube::CUBE_SIZE) * NYCube::CUBE_SIZE) + y * NYCube::CUBE_SIZE,
					(((int)start.Z / NYCube::CUBE_SIZE) * NYCube::CUBE_SIZE) + z * NYCube::CUBE_SIZE
				);

				if (getCube(cubePos.X, cubePos.Y, cubePos.Z)->isSolid())
				{
					NYVert3Df A = cubePos;
					NYVert3Df B = cubePos + NYVert3Df(NYCube::CUBE_SIZE, 0, 0);
					NYVert3Df C = cubePos + NYVert3Df(NYCube::CUBE_SIZE, 0, NYCube::CUBE_SIZE);
					NYVert3Df D = cubePos + NYVert3Df(0, 0, NYCube::CUBE_SIZE);
					NYVert3Df E = cubePos + NYVert3Df(0, NYCube::CUBE_SIZE, NYCube::CUBE_SIZE);
					NYVert3Df F = cubePos + NYVert3Df(0, NYCube::CUBE_SIZE, 0);
					NYVert3Df G = cubePos + NYVert3Df(NYCube::CUBE_SIZE, NYCube::CUBE_SIZE, 0);
					NYVert3Df H = cubePos + NYVert3Df(NYCube::CUBE_SIZE, NYCube::CUBE_SIZE, NYCube::CUBE_SIZE);

					NYVert3Df* tmp = new NYVert3Df(0, 0, 0);
					*intersect = *tmp;

					//ABCD
					if (planeIntersect(start, end, A, B, C, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}

					//BGHC
					if (planeIntersect(start, end, B, G, H, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}

					//GFEH
					if (planeIntersect(start, end, G, F, E, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}

					//FADE
					if (planeIntersect(start, end, F, A, D, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}

					//DCHE
					if (planeIntersect(start, end, D, C, H, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}

					//ABGF
					if (planeIntersect(start, end, A, B, G, tmp))
					{
						if ((*tmp - start).getMagnitude() < (*intersect - start).getMagnitude())
						{
							*intersect = *tmp;
						}
					}
				}
			}
		}
	}
	
	return (intersect->X != 0 || intersect->Y != 0 || intersect->Z != 0);
}
*/

/**
* Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
* versions optimisées de ce calcul.
*/
inline bool intersecDroitePlan(NYVert3Df & debSegment, NYVert3Df & finSegment,
	NYVert3Df & p1Plan, NYVert3Df & p2Plan, NYVert3Df & p3Plan,
	NYVert3Df & inter)
{
	//Equation du plan :
	NYVert3Df nrmlAuPlan = (p1Plan - p2Plan).vecProd(p3Plan - p2Plan); //On a les a,b,c du ax+by+cz+d = 0
	float d = -(nrmlAuPlan.X * p2Plan.X + nrmlAuPlan.Y * p2Plan.Y + nrmlAuPlan.Z* p2Plan.Z); //On remarque que c'est un produit scalaire...

																							 //Equation de droite :
	NYVert3Df dirDroite = finSegment - debSegment;

	//On resout l'équation de plan
	float nominateur = -d - nrmlAuPlan.X * debSegment.X - nrmlAuPlan.Y * debSegment.Y - nrmlAuPlan.Z * debSegment.Z;
	float denominateur = nrmlAuPlan.X * dirDroite.X + nrmlAuPlan.Y * dirDroite.Y + nrmlAuPlan.Z * dirDroite.Z;

	if (denominateur == 0)
		return false;

	//Calcul de l'intersection
	float t = nominateur / denominateur;
	inter = debSegment + (dirDroite*t);

	//Si point avant le debut du segment
	if (t < 0 || t > 1)
		return false;

	return true;
}

/**
* Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
* versions optimisées de ce calcul. Il faut donner les points dans l'ordre (CW ou CCW)
*/
inline bool intersecDroiteCubeFace(NYVert3Df & debSegment, NYVert3Df & finSegment,
	NYVert3Df & p1, NYVert3Df & p2, NYVert3Df & p3, NYVert3Df & p4,
	NYVert3Df & inter)
{
	//On calcule l'intersection
	bool res = intersecDroitePlan(debSegment, finSegment, p1, p2, p4, inter);

	if (!res)
		return false;

	//On fait les produits vectoriels
	NYVert3Df v1 = p2 - p1;
	NYVert3Df v2 = p3 - p2;
	NYVert3Df v3 = p4 - p3;
	NYVert3Df v4 = p1 - p4;

	NYVert3Df n1 = v1.vecProd(inter - p1);
	NYVert3Df n2 = v2.vecProd(inter - p2);
	NYVert3Df n3 = v3.vecProd(inter - p3);
	NYVert3Df n4 = v4.vecProd(inter - p4);

	//on compare le signe des produits scalaires
	float ps1 = n1.scalProd(n2);
	float ps2 = n2.scalProd(n3);
	float ps3 = n3.scalProd(n4);

	if (ps1 >= 0 && ps2 >= 0 && ps3 >= 0)
		return true;

	return false;
}
