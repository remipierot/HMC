#pragma once

#include "engine/render/renderer.h"
#include "cube.h"

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

		static const int CHUNK_SIZE = 16; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		NYCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		GLuint _BufWorld; ///< Identifiant du VBO pour le monde
		
		static float _WorldVert[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les sommets
		static float _WorldCols[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les couleurs
		static float _WorldNorm[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les normales

		static const int SIZE_VERTICE = 3 * sizeof(float); ///< Taille en octets d'un vertex dans le VBO
		static const int SIZE_COLOR = 3 * sizeof(float);  ///< Taille d'une couleur dans le VBO
		static const int SIZE_NORMAL = 3 * sizeof(float);  ///< Taille d'une normale dans le VBO
		static const int DELTA_COLOR_FACTOR = 10;
		
		int _NbVertices; ///< Nombre de vertices dans le VBO (on ne met que les faces visibles)

		NYChunk * Voisins[6];
		
		NYChunk()
		{
			_NbVertices = 0;
			_BufWorld = 0;
			memset(Voisins,0x00,sizeof(void*) * 6);
		}

		void setVoisins(NYChunk * xprev, NYChunk * xnext,NYChunk * yprev,NYChunk * ynext,NYChunk * zprev,NYChunk * znext)
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
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						_Cubes[x][y][z]._Type = CUBE_AIR;
					}
		}

		//On met le chunk ddans son VBO
		void toVbo(void)
		{
			NYCube c;
			float color[3];
			float tmp_color[3];
			float pos[3];
			float* pointers[] = {_WorldVert, _WorldCols, _WorldNorm};
			_NbVertices = 0;

			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				for (int y = 0; y < CHUNK_SIZE; y++)
				{
					for (int z = 0; z < CHUNK_SIZE; z++)
					{
						c = _Cubes[x][y][z];
						if (c._Draw && c.isSolid())
						{
							switch (c._Type)
							{
								case CUBE_HERBE:
									color[R] = 81.0f / 255.0f;
									color[G] = 124.0f / 255.0f;
									color[B] = 30.0f / 255.0f;
									break;
								case CUBE_EAU:
									color[R] = 40.0f / 255.0f;
									color[G] = 56.0f / 255.0f;
									color[B] = 219.0f / 255.0f;
									break;
								case CUBE_TERRE:
									color[R] = 118.0f / 255.0f;
									color[G] = 77.0f / 255.0f;
									color[B] = 43.0f / 255.0f;
									break;
							}

							pos[X] = x * (float)NYCube::CUBE_SIZE;
							pos[Y] = y * (float)NYCube::CUBE_SIZE;
							pos[Z] = z * (float)NYCube::CUBE_SIZE;

							for (int face = 0; face <= BACK; face++)
							{
								tmp_color[R] = color[R] + (randf() / DELTA_COLOR_FACTOR);
								tmp_color[G] = color[G] + (randf() / DELTA_COLOR_FACTOR);
								tmp_color[B] = color[B] + (randf() / DELTA_COLOR_FACTOR);

								for (int triangle = 0; triangle < 2; triangle++)
								{
									for (int vertex = 0; vertex < 3; vertex++)
									{
										//X pos
										*(pointers[VERTEX]) = pos[X];
										if (
											(face == RIGHT) ||
											(triangle == 0 && vertex == 1 && (face == BOTTOM || face == BACK)) || 
											(((triangle == 0 && vertex == 2) || (triangle == 1 && vertex == 0)) && face != RIGHT && face != LEFT) ||
											(triangle == 1 && vertex == 1 && (face == FRONT || face == TOP))
										)
											*(pointers[VERTEX]) += NYCube::CUBE_SIZE;
										pointers[VERTEX]++;

										//Y pos
										*(pointers[VERTEX]) = pos[Y];
										if (
											(face == TOP) ||
											(triangle == 0 && vertex == 1 && (face == FRONT || face == RIGHT)) ||
											(((triangle == 0 && vertex == 2) || (triangle == 1 && vertex == 0)) && face != TOP && face != BOTTOM) ||
											(triangle == 1 && vertex == 1 && (face == LEFT || face == BACK))
										)
											*(pointers[VERTEX]) += NYCube::CUBE_SIZE;
										pointers[VERTEX]++;

										//Z pos
										*(pointers[VERTEX]) = pos[Z];
										if (
											(face == BACK) ||
											(triangle == 0 && vertex == 1 && (face == LEFT || face == TOP)) ||
											(((triangle == 0 && vertex == 2) || (triangle == 1 && vertex == 0)) && face != FRONT && face != BACK) ||
											(triangle == 1 && vertex == 1 && (face == RIGHT || face == BOTTOM))
										)
											*(pointers[VERTEX]) += NYCube::CUBE_SIZE;
										pointers[VERTEX]++;
										_NbVertices++;

										*(pointers[COLOR]) = tmp_color[R];
										pointers[COLOR]++;
										*(pointers[COLOR]) = tmp_color[G];
										pointers[COLOR]++;
										*(pointers[COLOR]) = tmp_color[B];
										pointers[COLOR]++;

										*(pointers[NORMAL]) = (face == RIGHT) - (face == LEFT);
										pointers[NORMAL]++;
										*(pointers[NORMAL]) = (face == TOP) - (face == BOTTOM);
										pointers[NORMAL]++;
										*(pointers[NORMAL]) = (face == BACK) - (face == FRONT);
										pointers[NORMAL]++;
									}
								}
							}
						}
					}
				}
			}

			//On le detruit si il existe deja
			if (_BufWorld != 0)
				glDeleteBuffers(1, &_BufWorld);

			//Genere un identifiant
			glGenBuffers(1, &_BufWorld);

			//On attache le VBO pour pouvoir le modifier
			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);

			//On reserve la quantite totale de datas (creation de la zone memoire, mais sans passer les données)
			//Les tailles g_size* sont en octets, à vous de les calculer
			glBufferData(
				GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR +
				_NbVertices * SIZE_NORMAL,
				NULL,
				GL_STREAM_DRAW
			);

			GLenum error = glGetError();

			//On copie les vertices
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0, //Offset 0, on se place au début
				_NbVertices * SIZE_VERTICE, //Taille en octets du buffer vertices
				_WorldVert	//Datas
			);

			//Check error (la tester ensuite...)
			error = glGetError();

			//On copie les couleurs
			glBufferSubData(
				GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE, //Offset, on se place après les vertices
				_NbVertices * SIZE_COLOR, //Taille en octets du buffer colors
				_WorldCols	//Datas
			);

			//Check error (la tester ensuite...)
			error = glGetError();

			//On copie les normales (a vous de déduire les params)
			glBufferSubData(
				GL_ARRAY_BUFFER,
				_NbVertices * (SIZE_VERTICE + SIZE_COLOR),	//Offset, on se place après les colors
				_NbVertices * SIZE_NORMAL,	//Taille en octets du buffer normals
				_WorldNorm	//Datas
			);

			//On debind le buffer pour eviter une modif accidentelle par le reste du code
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

			if(x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE-1][y][z]);
			else if(x > 0)
				cubeXPrev = &(_Cubes[x-1][y][z]);

			if(x == CHUNK_SIZE-1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if(x < CHUNK_SIZE-1)
				cubeXNext = &(_Cubes[x+1][y][z]);

			if(y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE-1][z]);
			else if(y > 0)
				cubeYPrev = &(_Cubes[x][y-1][z]);

			if(y == CHUNK_SIZE-1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if(y < CHUNK_SIZE-1)
				cubeYNext = &(_Cubes[x][y+1][z]);

			if(z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE-1]);
			else if(z > 0)
				cubeZPrev = &(_Cubes[x][y][z-1]);

			if(z == CHUNK_SIZE-1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if(z < CHUNK_SIZE-1)
				cubeZNext = &(_Cubes[x][y][z+1]);

			if( cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL )
				return false;

			if( cubeXPrev->isSolid() == true && //droite
				cubeXNext->isSolid() == true && //gauche
				cubeYPrev->isSolid() == true && //haut
				cubeYNext->isSolid() == true && //bas
				cubeZPrev->isSolid() == true && //devant
				cubeZNext->isSolid() == true )  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						if(test_hidden(x,y,z))
							_Cubes[x][y][z]._Draw = false;
					}
		}
};