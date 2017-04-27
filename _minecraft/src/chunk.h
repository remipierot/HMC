#pragma once

#include "engine/render/renderer.h"
#include "cube.h"
#include "basic_physic_engine.h"

enum Color
{
	R,
	G,
	B
};

enum Coordinate
{
	X,
	Y,
	Z
};

enum Pointers
{
	VERTEX,
	COLOR,
	NORMAL
};

enum Faces
{
	FRONT,
	RIGHT,
	LEFT,
	TOP,
	BOTTOM,
	BACK
};

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class NYChunk
{
	public :
		static const int X_CHUNK_SIZE = 16;
		static const int Y_CHUNK_SIZE = 16;
		static const int Z_CHUNK_SIZE = 16;
		NYCube _Cubes[X_CHUNK_SIZE][Y_CHUNK_SIZE][Z_CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		GLuint _BufWorld; ///< Identifiant du VBO pour le monde

		static float _WorldVert[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2]; ///< Buffer pour les sommets
		static float _WorldCols[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2]; ///< Buffer pour les couleurs
		static float _WorldNorm[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2]; ///< Buffer pour les normales


		static const int SIZE_VERTICE = 3 * sizeof(float); ///< Taille en octets d'un vertex dans le VBO
		static const int SIZE_COLOR = 3 * sizeof(float);  ///< Taille d'une couleur dans le VBO
		static const int SIZE_NORMAL = 3 * sizeof(float);  ///< Taille d'une normale dans le VBO

		int _NbVertices; ///< Nombre de vertices dans le VBO (on ne met que les faces visibles)

		NYChunk * Voisins[6];
		btRigidBody * _Body;

		int _XPos, _YPos, _ZPos; ///< Position du chunk dans le monde
		bool _UpToDate;

		NYChunk(int x, int y, int z)
		{
			_NbVertices = 0;
			_BufWorld = 0;
			memset(Voisins, 0x00, sizeof(void*) * 6);
			_XPos = x;
			_YPos = y;
			_ZPos = z;
			_UpToDate = false;
			_Body = NULL;
		}

		void setVoisins(NYChunk * xprev, NYChunk * xnext, NYChunk * yprev, NYChunk * ynext, NYChunk * zprev, NYChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		/**
		* Raz de l'état des cubes (a draw = false)
		*/
		void reset(void)
		{
			for (int x = 0; x<X_CHUNK_SIZE; x++)
				for (int y = 0; y<Y_CHUNK_SIZE; y++)
					for (int z = 0; z<Z_CHUNK_SIZE; z++)
					{
						_Cubes[x][y][z]._Draw = true;
						_Cubes[x][y][z]._Type = CUBE_AIR;
					}
		}

		//On met le chunk ddans son VBO
		void toVbo(void)
		{
			//On utilise les buffers temporaires pour préparer nos datas
			float * ptVert = _WorldVert;
			float * ptCols = _WorldCols;
			float * ptNorm = _WorldNorm;
			_NbVertices = 0;

			//On parcourt tous nos cubes
			for (int x = 0; x<X_CHUNK_SIZE; x++)
			{
				for (int y = 0; y<Y_CHUNK_SIZE; y++)
				{
					for (int z = 0; z<Z_CHUNK_SIZE; z++)
					{
						if (_Cubes[x][y][z]._Draw && _Cubes[x][y][z]._Type != CUBE_AIR)
						{
							float color[3];

							switch (_Cubes[x][y][z]._Type)
							{
							case CUBE_TERRE:
								color[0] = 101.0f / 255.0f;
								color[1] = 74.0f / 255.0f;
								color[2] = 0.0f / 255.0f;
								break;
							case CUBE_HERBE:
								color[0] = 1.0f / 255.0f;
								color[1] = 112.0f / 255.0f;
								color[2] = 12.0f / 255.0f;
								break;
							case CUBE_EAU:
								color[0] = 0.0f / 255.0f;
								color[1] = 48.0f / 255.0f;
								color[2] = 255.0f / 255.0f;
								break;
							}

							//Position du cube (coin bas gauche face avant)
							float xPos = x*(float)NYCube::CUBE_SIZE;
							float yPos = y*(float)NYCube::CUBE_SIZE;
							float zPos = z*(float)NYCube::CUBE_SIZE;

							NYCube * cubeXPrev = NULL;
							NYCube * cubeXNext = NULL;
							NYCube * cubeYPrev = NULL;
							NYCube * cubeYNext = NULL;
							NYCube * cubeZPrev = NULL;
							NYCube * cubeZNext = NULL;

							get_surrounding_cubes(
								x, y, z,
								&cubeXPrev, &cubeXNext,
								&cubeYPrev, &cubeYNext,
								&cubeZPrev, &cubeZNext);


							//Premier QUAD (le z-)
							if (cubeZPrev == NULL || cubeZPrev->isSolid() == false)
							{
								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
							}



							//Second QUAD (x+)
							if (cubeXNext == NULL || cubeXNext->isSolid() == false)
							{
								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
							}

							//Troisieme QUAD (x-)
							if (cubeXPrev == NULL || cubeXPrev->isSolid() == false)
							{
								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
							}


							//Quatrieme QUAD (y+)
							if (cubeYNext == NULL || cubeYNext->isSolid() == false)
							{
								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
								*ptNorm = 0; ptNorm++;
							}

							//Cinquieme QUAD (y-)
							if (cubeYPrev == NULL || cubeYPrev->isSolid() == false)
							{
								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = -1; ptNorm++;
								*ptNorm = 0; ptNorm++;
							}

							//Sixieme QUAD (z+)
							if (cubeZNext == NULL || cubeZNext->isSolid() == false)
							{
								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;

								*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;

								*ptVert = xPos; ptVert++;
								*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
								*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

								*ptCols = color[0] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[1] + (rand() % 100) / 2000.0f; ptCols++;
								*ptCols = color[2] + (rand() % 100) / 2000.0f; ptCols++;

								*ptNorm = 0; ptNorm++;
								*ptNorm = 0; ptNorm++;
								*ptNorm = 1; ptNorm++;
							}
						}
					}
				}
			}

			if (_BufWorld != 0)
				glDeleteBuffers(1, &_BufWorld);

			glGenBuffers(1, &_BufWorld);

			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);

			glBufferData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR +
				_NbVertices * SIZE_NORMAL,
				NULL,
				GL_STREAM_DRAW);

			GLenum error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				0,
				_NbVertices * SIZE_VERTICE,
				_WorldVert);

			error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE,
				_NbVertices * SIZE_COLOR,
				_WorldCols);

			error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR,
				_NbVertices * SIZE_NORMAL,
				_WorldNorm);

			error = glGetError();

			glBindBuffer(GL_ARRAY_BUFFER, 0);

		}

		void render(void)
		{

			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			//On bind le buuffer
			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);
			NYRenderer::checkGlError("glBindBuffer");

			//On active les datas que contiennent le VBO
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);

			//On place les pointeurs sur les datas, aux bons offsets
			glVertexPointer(3, GL_FLOAT, 0, (void*)(0));
			glColorPointer(3, GL_FLOAT, 0, (void*)(_NbVertices*SIZE_VERTICE));
			glNormalPointer(GL_FLOAT, 0, (void*)(_NbVertices*SIZE_VERTICE + _NbVertices*SIZE_COLOR));

			//On demande le dessin
			glDrawArrays(GL_TRIANGLES, 0, _NbVertices);

			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);

			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHTING);
		}
		/**
		* Met a jour la structure, à la fois pour le rendu et pour la physique
		*/
		void updateStructure(void)
		{
			disableHiddenCubes();
			toVbo();
			addToPhysic();
			_UpToDate = true;
		}

		/**
		* Permet d'ajouter un chunk au moteur physique. utilise pour cela _WorldVert (qui est statique)
		* et qui doit donc être généré avant. Cette fonction est donc appelée par updatestructure et
		* reste privée
		*/
		void addToPhysic()
		{
			if (_Body != NULL)
			{
				NYBasicPhysicEngine::getInstance()->removeObject(_Body);
				SAFEDELETE(_Body);
			}

			//Mise a jour du worldVert
			if (_NbVertices > 0)
			{
				btGImpactMeshShape * mesh = NYBasicPhysicEngine::getInstance()->
					calcTriMeshShape(_WorldVert, _NbVertices, 3 * sizeof(float));
				_Body = NYBasicPhysicEngine::getInstance()->addTriMeshObject(false, mesh,
					NYVert3Df(_XPos*NYChunk::X_CHUNK_SIZE*NYCube::CUBE_SIZE,
						_YPos*NYChunk::Y_CHUNK_SIZE*NYCube::CUBE_SIZE,
						_ZPos*NYChunk::Z_CHUNK_SIZE*NYCube::CUBE_SIZE), 0.0f);
				NYBasicPhysicEngine::getInstance()->setCollisionReportOnBody(_Body);
			}
		}

		/**
		* On verifie si le cube peut être vu
		*/
		bool test_hidden(int x, int y, int z)
		{
			NYCube * cubeXPrev = NULL;
			NYCube * cubeXNext = NULL;
			NYCube * cubeYPrev = NULL;
			NYCube * cubeYNext = NULL;
			NYCube * cubeZPrev = NULL;
			NYCube * cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[X_CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == X_CHUNK_SIZE - 1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < X_CHUNK_SIZE - 1)
				cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][Y_CHUNK_SIZE - 1][z]);
			else if (y > 0)
				cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == Y_CHUNK_SIZE - 1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < Y_CHUNK_SIZE - 1)
				cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][Z_CHUNK_SIZE - 1]);
			else if (z > 0)
				cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == Z_CHUNK_SIZE - 1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < Z_CHUNK_SIZE - 1)
				cubeZNext = &(_Cubes[x][y][z + 1]);

			if (cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL)
				return false;

			if (cubeXPrev->isSolid() == true && //droite
				cubeXNext->isSolid() == true && //gauche
				cubeYPrev->isSolid() == true && //haut
				cubeYNext->isSolid() == true && //bas
				cubeZPrev->isSolid() == true && //devant
				cubeZNext->isSolid() == true)  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for (int x = 0; x<X_CHUNK_SIZE; x++)
				for (int y = 0; y<Y_CHUNK_SIZE; y++)
					for (int z = 0; z<Z_CHUNK_SIZE; z++)
					{
						_Cubes[x][y][z]._Draw = true;
						if (test_hidden(x, y, z))
							_Cubes[x][y][z]._Draw = false;
					}
		}

		void get_surrounding_cubes(int x, int y, int z, NYCube ** cubeXPrev, NYCube ** cubeXNext,
			NYCube ** cubeYPrev, NYCube ** cubeYNext,
			NYCube ** cubeZPrev, NYCube ** cubeZNext)
		{

			*cubeXPrev = NULL;
			*cubeXNext = NULL;
			*cubeYPrev = NULL;
			*cubeYNext = NULL;
			*cubeZPrev = NULL;
			*cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				*cubeXPrev = &(Voisins[0]->_Cubes[X_CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				*cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == X_CHUNK_SIZE - 1 && Voisins[1] != NULL)
				*cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < X_CHUNK_SIZE - 1)
				*cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				*cubeYPrev = &(Voisins[2]->_Cubes[x][Y_CHUNK_SIZE - 1][z]);
			else if (y > 0)
				*cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == Y_CHUNK_SIZE - 1 && Voisins[3] != NULL)
				*cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < Y_CHUNK_SIZE - 1)
				*cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				*cubeZPrev = &(Voisins[4]->_Cubes[x][y][Z_CHUNK_SIZE - 1]);
			else if (z > 0)
				*cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == Z_CHUNK_SIZE - 1 && Voisins[5] != NULL)
				*cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < Z_CHUNK_SIZE - 1)
				*cubeZNext = &(_Cubes[x][y][z + 1]);
		}

};