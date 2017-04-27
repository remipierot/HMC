#ifndef __WORLD_H__
#define __WORLD_H__

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "OpenSimplexNoise.h"
#include "my_physics.h"

typedef uint8 NYCollision;
#define NY_COLLIDE_UP     0x01
#define NY_COLLIDE_BOTTOM 0x02
#define NY_COLLIDE_LEFT   0x04
#define NY_COLLIDE_RIGHT  0x08
#define NY_COLLIDE_FRONT  0x10
#define NY_COLLIDE_BACK   0x20
#define NY_COLLIDE_IN     0x40

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_X_SIZE 16 //en nombre de chunks
#define MAT_Y_SIZE 16 //en nombre de chunks
#define MAT_Z_SIZE 4 //en nombre de chunks
#define MAT_X_SIZE_CUBES (MAT_X_SIZE * NYChunk::X_CHUNK_SIZE)
#define MAT_Y_SIZE_CUBES (MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE)
#define MAT_Z_SIZE_CUBES (MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE)
#define SMOOTH_PASS 1

class NYWorld
{
public :
	NYChunk * _Chunks[MAT_X_SIZE][MAT_Y_SIZE][MAT_Z_SIZE];
	int _MatriceHeights[MAT_X_SIZE_CUBES][MAT_Y_SIZE_CUBES];
	int _MatriceHeightsTmp[MAT_X_SIZE_CUBES][MAT_Y_SIZE_CUBES];
	float _FacteurGeneration;
	std::vector<btRigidBody*> _ExplodedCubes;
	std::vector<NYCubeType> _ExplodedCubesTypes;

	NYWorld()
	{
		_FacteurGeneration = 300.0f;

		//On crée les chunks
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					_Chunks[x][y][z] = new NYChunk(x, y, z);

		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
				{
					NYChunk * cxPrev = NULL;
					if(x > 0)
						cxPrev = _Chunks[x-1][y][z];
					NYChunk * cxNext = NULL;
					if(x < MAT_X_SIZE-1)
						cxNext = _Chunks[x+1][y][z];

					NYChunk * cyPrev = NULL;
					if(y > 0)
						cyPrev = _Chunks[x][y-1][z];
					NYChunk * cyNext = NULL;
					if(y < MAT_Y_SIZE-1)
						cyNext = _Chunks[x][y+1][z];

					NYChunk * czPrev = NULL;
					if(z > 0)
						czPrev = _Chunks[x][y][z-1];
					NYChunk * czNext = NULL;
					if(z < MAT_Z_SIZE-1)
						czNext = _Chunks[x][y][z+1];

					_Chunks[x][y][z]->setVoisins(cxPrev,cxNext,cyPrev,cyNext,czPrev,czNext);
				}

					
	}

	inline NYCube * getCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) x = (MAT_X_SIZE * NYChunk::X_CHUNK_SIZE)-1;
		if(y >= MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) y = (MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE)-1;
		if(z >= MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) z = (MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE)-1;

		return &(_Chunks[x / NYChunk::X_CHUNK_SIZE][y / NYChunk::Y_CHUNK_SIZE][z / NYChunk::Z_CHUNK_SIZE]->_Cubes[x % NYChunk::X_CHUNK_SIZE][y % NYChunk::Y_CHUNK_SIZE][z % NYChunk::Z_CHUNK_SIZE]);
	}

	void updateCube(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_X_SIZE * NYChunk::X_CHUNK_SIZE)x = (MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) - 1;
		if (y >= MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE)y = (MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) - 1;
		if (z >= MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE)z = (MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) - 1;

		int xChnk = x / NYChunk::X_CHUNK_SIZE;
		int yChnk = y / NYChunk::Y_CHUNK_SIZE;
		int zChnk = z / NYChunk::Z_CHUNK_SIZE;

		NYChunk * chk = _Chunks[xChnk][yChnk][zChnk];

		chk->updateStructure();

		for (int i = 0; i<6; i++)
			if (chk->Voisins[i])
			{
				chk->Voisins[i]->updateStructure();
			}
	}

	void updateStructure(void)
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					if (!_Chunks[x][y][z]->_UpToDate)
					{
						_Chunks[x][y][z]->updateStructure();
					}
	}

	void deleteCube(int x, int y, int z, bool autoUpdate = true)
	{
		NYCube * cube = getCube(x, y, z);
		cube->_Draw = false;
		cube->_Type = CUBE_AIR;
		NYChunk * chk = getChunkFromCube(x, y, z);
		chk->_UpToDate = false;
		for (int i = 0; i<6; i++)
			if (chk->Voisins[i])
				chk->Voisins[i]->_UpToDate = false;

		if (autoUpdate)
			updateStructure();
	}

	inline NYChunk * getChunkFromCube(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) x = (MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) - 1;
		if (y >= MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) y = (MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) - 1;
		if (z >= MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) z = (MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) - 1;

		return _Chunks[x / NYChunk::X_CHUNK_SIZE][y / NYChunk::Y_CHUNK_SIZE][z / NYChunk::Z_CHUNK_SIZE];
	}

	/**
	* Ajoute tous les chunks au moteur physique
	*/
	void addToPhysic(void)
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
				{
					_Chunks[x][y][z]->updateStructure();
				}
	}

	//Création d'une pile de cubes
	//only if zero permet de ne générer la  pile que si sa hauteur actuelle est de 0 (et ainsi de ne pas regénérer de piles existantes)
	void load_pile(int x, int y, int height, bool reset = true, bool onlyIfZero = true)
	{
		if (onlyIfZero && _MatriceHeights[x][y] != 0)
		{
			return;
		}

		NYCube* c;
		_MatriceHeights[x][y] = height;

		for (int z = 0; z < MAT_Z_SIZE_CUBES; z++)
		{
			c = getCube(x, y, z);
			if (reset == true)
			{
				c->_Type = (z == 0)
							? CUBE_EAU
							: (z < height - 1)
								? CUBE_TERRE
								: (z == height - 1)
									? CUBE_HERBE
									: CUBE_AIR;
			}
			else
			{
				if (c->_Type == CUBE_TERRE)
				{
					_MatriceHeights[x][y] = z;
				}
			}
		}

		if (reset == false)
		{
			for (int z = _MatriceHeights[x][y]; z < MAT_Z_SIZE_CUBES; z++)
			{
				c = getCube(x, y, z);
				c->_Type = (z == _MatriceHeights[x][y])
							? CUBE_HERBE
							: CUBE_AIR;
			}
		}
	}

	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	void generate_piles(
		int x1, int y1,
		int x2, int y2, 
		int x3, int y3,
		int x4, int y4, 
		int prof, int profMax = -1)
	{
		if (abs(x3 - x1) <= 1 && abs(y3 - y1) <= 1)
			return;

		float factor = _FacteurGeneration / (float)prof;

		NYVert3Df center = NYVert3Df(
			(x1 + x2 + x3 + x4) / 4,
			(y1 + y2 + y3 + y4) / 4,
			((_MatriceHeights[x1][y1] + _MatriceHeights[x2][y2] + _MatriceHeights[x3][y3] + _MatriceHeights[x4][y4]) / 4) + (factor / 2.0f) - (randf() * factor)
		);
		NYVert3Df north = NYVert3Df(
			(x1 + x2) / 2, 
			(y1 + y2) / 2, 
			((_MatriceHeights[x1][y1] + _MatriceHeights[x2][y2]) / 3) + (factor * center.Z / 2.0f) - (randf() * factor * center.Z)
		);
		NYVert3Df east = NYVert3Df(
			(x2 + x3) / 2, 
			(y2 + y3) / 2, 
			((_MatriceHeights[x2][y2] + _MatriceHeights[x3][y3]) / 3) + (factor * center.Z / 2.0f) - (randf() * factor * center.Z)
		);
		NYVert3Df south = NYVert3Df(
			(x3 + x4) / 2, 
			(y3 + y4) / 2, 
			((_MatriceHeights[x3][y3] + _MatriceHeights[x4][y4]) / 3) + (factor * center.Z / 2.0f) - (randf() * factor * center.Z)
		);
		NYVert3Df west = NYVert3Df(
			(x4 + x1) / 2, 
			(y4 + y1) / 2, 
			((_MatriceHeights[x4][y4] + _MatriceHeights[x1][y1]) / 3) + (factor * center.Z / 2.0f) - (randf() * factor * center.Z)
		);

		center.Z = max(1, center.Z);
		north.Z = max(1, north.Z);
		east.Z = max(1, east.Z);
		south.Z = max(1, south.Z);
		west.Z = max(1, west.Z);

		load_pile(center.X, center.Y, center.Z);
		load_pile(north.X, north.Y, north.Z);
		load_pile(east.X, east.Y, east.Z);
		load_pile(south.X, south.Y, south.Z);
		load_pile(west.X, west.Y, west.Z);

		//NORTH-WEST block
		generate_piles(
			x1, y1, 
			north.X, north.Y, 
			center.X, center.Y, 
			west.X, west.Y, 
			prof+1, profMax);

		//NORTH-EAST block
		generate_piles(
			north.X, north.Y, 
			x2, y2, 
			east.X, east.Y, 
			center.X, center.Y, 
			prof+1, profMax);

		//SOUTH-EAST block
		generate_piles(
			center.X, center.Y, 
			east.X, east.Y, 
			x3, y3, 
			south.X, south.Y, 
			prof+1, profMax);

		//SOUTH-WEST block
		generate_piles(
			west.X, west.Y, 
			center.X, center.Y, 
			south.X, south.Y, 
			x4, y4, 
			prof+1, profMax);
	}


	void lisse(bool reset = true)
	{
		int window = 4;
		memset(_MatriceHeightsTmp, 0x00, sizeof(int)*MAT_X_SIZE_CUBES*MAT_Y_SIZE_CUBES);

		for (int x = 0; x<MAT_X_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_Y_SIZE_CUBES; y++)
			{
				int count = 0;
				int win_x_start = max(0, x - window);
				int win_x_end = min(x + window, MAT_X_SIZE_CUBES - 1);
				int win_y_start = max(0, y - window);
				int win_y_end = min(y + window, MAT_Y_SIZE_CUBES - 1);
				
				for (int i = win_x_start; i < win_x_end; i++)
				{
					for (int j = win_y_start; j < win_y_end; j++)
					{
						_MatriceHeightsTmp[x][y] += _MatriceHeights[i][j];
						count++;
					}
				}

				_MatriceHeightsTmp[x][y] /= count;
			}
		}

		//On reset les piles
		for (int x = 0; x<MAT_X_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_Y_SIZE_CUBES; y++)
			{
				load_pile(x, y, _MatriceHeightsTmp[x][y], reset, false);
			}
		}
	}

	void perlin_generation()
	{
		NYCube* c;
		float tmp_noise;

		OpenSimplexNoise::init(NYRenderer::_DeltaTimeCumul);

		for (int z = 0; z < MAT_Z_SIZE_CUBES; z++)
		{
			for (int y = 0; y < MAT_Y_SIZE_CUBES; y++)
			{
				for (int x = 0; x < MAT_X_SIZE_CUBES; x++)
				{
					c = getCube(x, y, z);
					tmp_noise = OpenSimplexNoise::eval(x / (float)NYChunk::X_CHUNK_SIZE, y / (float)NYChunk::Y_CHUNK_SIZE, z / (float)NYChunk::Z_CHUNK_SIZE);

					c->_Type = (tmp_noise < 0) ? CUBE_AIR : CUBE_TERRE;

					if (c->_Type == CUBE_TERRE)
					{
						_MatriceHeights[x][y] = z;
					}
					if (z == 0 && c->_Type == CUBE_AIR)
					{
						c->_Type = CUBE_EAU;
					}
				}
			}
		}

		for (int x = 0; x < MAT_X_SIZE_CUBES; x++)
		{
			for (int y = 0; y < MAT_Y_SIZE_CUBES; y++)
			{
				int z = _MatriceHeights[x][y];
				c = getCube(x, y, z);

				c->_Type = (z == 0) ? CUBE_EAU : CUBE_HERBE;
			}
		}
	}

	void diamond_square_generation(int profmax = -1)
	{
		int max_height = rand() % 4;

		//On charge les 4 coins
		load_pile(
			0,
			0,
			((max_height == 0) + (randf() * (max_height != 0))) * MAT_Z_SIZE_CUBES
			);
		load_pile(
			MAT_X_SIZE_CUBES - 1,
			0,
			((max_height == 1) + (randf() * (max_height != 1))) * MAT_Z_SIZE_CUBES
			);
		load_pile(
			MAT_X_SIZE_CUBES - 1,
			MAT_Y_SIZE_CUBES - 1,
			((max_height == 2) + (randf() * (max_height != 2))) * MAT_Z_SIZE_CUBES
			);
		load_pile(
			0,
			MAT_Y_SIZE_CUBES - 1,
			((max_height == 3) + (randf() * (max_height != 3))) * MAT_Z_SIZE_CUBES
			);

		//On génère a partir des 4 coins
		generate_piles(
			0, 0,
			MAT_X_SIZE_CUBES - 1, 0,
			MAT_X_SIZE_CUBES - 1, MAT_Y_SIZE_CUBES - 1,
			0, MAT_Y_SIZE_CUBES - 1,
			1, profmax
			);
	}

	void init_world(int profmax = -1)
	{
		NYCube* c;
		_cprintf("Creation du monde %f \n",_FacteurGeneration);

		srand(NYRenderer::_DeltaTimeCumul);
		randf(); // Needed so that the first randf does not give 0 as an output

		//Reset du monde
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					_Chunks[x][y][z]->reset();
		memset(_MatriceHeights,0x00,MAT_X_SIZE_CUBES*MAT_Y_SIZE_CUBES*sizeof(int));

		diamond_square_generation(profmax);
		//perlin_generation();

		for (int i = 0; i < SMOOTH_PASS; i++)
			lisse();

		disableHiddenCubes();

		add_world_to_vbo();
		updateStructure();
		//addToPhysic();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point)
	{
		return NULL;
	}

	void disableHiddenCubes() 
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					_Chunks[x][y][z]->disableHiddenCubes();
	}

	//Boites de collisions plus petites que deux cubes
	NYAxis getMinCol(NYVert3Df pos, NYVert3Df dir, float width, float height, float & valueColMin, bool oneShot)
	{
		int x = (int)(pos.X / NYCube::CUBE_SIZE);
		int y = (int)(pos.Y / NYCube::CUBE_SIZE);
		int z = (int)(pos.Z / NYCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / NYCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / NYCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / NYCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / NYCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / NYCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / NYCube::CUBE_SIZE);

		if (x < 0)	x = 0;
		if (y < 0)	y = 0;
		if (z < 0)	z = 0;

		if (xPrev < 0)	xPrev = 0;
		if (yPrev < 0)	yPrev = 0;
		if (zPrev < 0)	zPrev = 0;

		if (xNext < 0)	xNext = 0;
		if (yNext < 0)	yNext = 0;
		if (zNext < 0)	zNext = 0;

		if (x >= MAT_X_SIZE_CUBES)	x = MAT_X_SIZE_CUBES - 1;
		if (y >= MAT_Y_SIZE_CUBES)	y = MAT_Y_SIZE_CUBES - 1;
		if (z >= MAT_Z_SIZE_CUBES)	z = MAT_Z_SIZE_CUBES - 1;

		if (xPrev >= MAT_X_SIZE_CUBES)	xPrev = MAT_X_SIZE_CUBES - 1;
		if (yPrev >= MAT_Y_SIZE_CUBES)	yPrev = MAT_Y_SIZE_CUBES - 1;
		if (zPrev >= MAT_Z_SIZE_CUBES)	zPrev = MAT_Z_SIZE_CUBES - 1;

		if (xNext >= MAT_X_SIZE_CUBES)	xNext = MAT_X_SIZE_CUBES - 1;
		if (yNext >= MAT_Y_SIZE_CUBES)	yNext = MAT_Y_SIZE_CUBES - 1;
		if (zNext >= MAT_Z_SIZE_CUBES)	zNext = MAT_Z_SIZE_CUBES - 1;

		//On fait chaque axe
		NYAxis axis = 0x00;
		valueColMin = oneShot ? 0.5 : 10000.0f;
		float seuil = 0.00001;
		float prodScalMin = 1.0f;
		if (dir.getMagnitude() > 1)
			dir.normalize();

		//On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				getCube(xPrev + 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((xPrev + 1) * NYCube::CUBE_SIZE) - (pos.X - width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		//On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				getCube(xNext - 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		//On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				getCube(xNext, yNext - 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		//On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				getCube(xNext, yPrev + 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				getCube(xNext, yNext, zNext - 1)->isSolid()) || !oneShot)
			{
				float depassement = (zNext * NYCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}

		//On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				getCube(xNext, yNext, zPrev + 1)->isSolid()) || !oneShot)
			{
				float depassement = ((zPrev + 1) * NYCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}
		return axis;
	}

	//Boites de collisions plus petites que deux cubes
	NYCollision collide_with_world(NYVert3Df pos, float width, float height, NYCollision & collisionPrincipale)
	{
		NYCollision collision = 0x00;

		int x = (int)(pos.X / NYCube::CUBE_SIZE);
		int y = (int)(pos.Y / NYCube::CUBE_SIZE);
		int z = (int)(pos.Z / NYCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / NYCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / NYCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / NYCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / NYCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / NYCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / NYCube::CUBE_SIZE);

		//De combien je dépasse dans les autres blocs
		float xDepNext = 0;
		if (xNext != x)
			xDepNext = abs((xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f));
		float xDepPrev = 0;
		if (xPrev != x)
			xDepPrev = abs((xPrev * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f));


		float yDepNext = 0;
		if (yNext != y)
			yDepNext = abs((yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f));
		float yDepPrev = 0;
		if (yPrev != y)
			yDepPrev = abs((yPrev * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f));


		float zDepNext = 0;
		if (zNext != z)
			zDepNext = abs((zNext * NYCube::CUBE_SIZE) - (pos.Z + height / 2.0f));
		float zDepPrev = 0;
		if (zPrev != z)
			zDepPrev = abs((zPrev * NYCube::CUBE_SIZE) - (pos.Z + height / 2.0f));

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;

		if (xPrev < 0)
			xPrev = 0;
		if (yPrev < 0)
			yPrev = 0;
		if (zPrev < 0)
			zPrev = 0;

		if (xNext < 0)
			xNext = 0;
		if (yNext < 0)
			yNext = 0;
		if (zNext < 0)
			zNext = 0;

		if (x >= MAT_X_SIZE_CUBES)
			x = MAT_X_SIZE_CUBES - 1;
		if (y >= MAT_Y_SIZE_CUBES)
			y = MAT_Y_SIZE_CUBES - 1;
		if (z >= MAT_Z_SIZE_CUBES)
			z = MAT_Z_SIZE_CUBES - 1;

		if (xPrev >= MAT_X_SIZE_CUBES)
			xPrev = MAT_X_SIZE_CUBES - 1;
		if (yPrev >= MAT_Y_SIZE_CUBES)
			yPrev = MAT_Y_SIZE_CUBES - 1;
		if (zPrev >= MAT_Z_SIZE_CUBES)
			zPrev = MAT_Z_SIZE_CUBES - 1;

		if (xNext >= MAT_X_SIZE_CUBES)
			xNext = MAT_X_SIZE_CUBES - 1;
		if (yNext >= MAT_Y_SIZE_CUBES)
			yNext = MAT_Y_SIZE_CUBES - 1;
		if (zNext >= MAT_Z_SIZE_CUBES)
			zNext = MAT_Z_SIZE_CUBES - 1;

		//Est on dans un cube plein ?
		if (getCube(x, y, z)->_Draw)
			collision |= NY_COLLIDE_IN;

		//Collisions droite et gauche
		//On checke ou se trouvent les sommets de la box
		//On checke les coins top et bottom en meme temps

		//Sommets arrières gauches
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid())
		{
			collision |= NY_COLLIDE_LEFT;
			collision |= NY_COLLIDE_BACK;
			if (xDepPrev > yDepPrev)
				collisionPrincipale |= NY_COLLIDE_LEFT;
			else
				collisionPrincipale |= NY_COLLIDE_BACK;
		}

		//Sommets avants gauches
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			collision |= NY_COLLIDE_LEFT;
			collision |= NY_COLLIDE_FRONT;
			if (xDepPrev > yDepNext)
				collisionPrincipale |= NY_COLLIDE_LEFT;
			else
				collisionPrincipale |= NY_COLLIDE_FRONT;
		}

		//Sommets arrière droits
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			collision |= NY_COLLIDE_RIGHT;
			collision |= NY_COLLIDE_BACK;
			if (xDepNext > yDepPrev)
				collisionPrincipale |= NY_COLLIDE_RIGHT;
			else
				collisionPrincipale |= NY_COLLIDE_BACK;
		}

		//Sommets avant droits
		if (getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			collision |= NY_COLLIDE_RIGHT;
			collision |= NY_COLLIDE_FRONT;
			if (xDepNext > yDepNext)
				collisionPrincipale |= NY_COLLIDE_RIGHT;
			else
				collisionPrincipale |= NY_COLLIDE_FRONT;
		}

		//Pour le bottom on checke tout, meme le milieu 
		if (getCube(xPrev, y, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, y, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(x, y, zPrev)->isSolid() ||
			getCube(x, yPrev, zPrev)->isSolid() ||
			getCube(x, yNext, zPrev)->isSolid())
			collision |= NY_COLLIDE_BOTTOM;

		//Pour le up on checke tout, meme le milieu 
		if (getCube(xPrev, y, zNext)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, y, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid() ||
			getCube(x, y, zNext)->isSolid() ||
			getCube(x, yPrev, zNext)->isSolid() ||
			getCube(x, yNext, zNext)->isSolid())
			collision |= NY_COLLIDE_UP;

		return collision;
	}

	void add_world_to_vbo(void)
	{
		int totalNbVertices = 0;

		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
				{
					_Chunks[x][y][z]->toVbo();
					totalNbVertices += _Chunks[x][y][z]->_NbVertices;
				}

		Log::log(Log::ENGINE_INFO, (toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{
		for (int x = 0; x < MAT_X_SIZE_CUBES; x++)
		{
			for (int y = 0; y < MAT_Y_SIZE_CUBES; y++)
			{
				for (int z = 0; z < MAT_Z_SIZE_CUBES; z++)
				{
					NYCube *cube = getCube(x, y, z);
					if (cube->_Draw)
					{
						glPushMatrix(); //à chaque fois qu'il faut dessiner plusieurs figures
										//Material
						GLfloat materialDiffuse[] = { 0, 0.7, 0,1.0 };
						GLfloat materialAmbient[] = { 1, 1, 1, 0.2 };

						switch (cube->_Type)
						{
						case CUBE_TERRE:
							materialDiffuse[0] = 164 / 255.f;
							materialDiffuse[1] = 120 / 255.f;
							materialDiffuse[2] = 70 / 255.f;
							break;
						case CUBE_AIR:
							materialDiffuse[0] = 226 / 255.f;
							materialDiffuse[1] = 236 / 255.f;
							materialDiffuse[2] = 238 / 255.f;
							materialDiffuse[3] = 0.f;
							break;
						case CUBE_EAU:
							materialDiffuse[0] = 161 / 255.f;
							materialDiffuse[1] = 228 / 255.f;
							materialDiffuse[2] = 239 / 255.f;
							materialDiffuse[3] = 0.5f;
							break;
						case CUBE_HERBE:
							materialDiffuse[0] = 142 / 255.f;
							materialDiffuse[1] = 196 / 255.f;
							materialDiffuse[2] = 147 / 255.f;
							break;
						default:
							break;
						}
						glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

						glTranslated(x, y, z);
						glutSolidCube(1.f);
						glPopMatrix();
					}
				}
			}
		}
	}

	bool getRayCollision(NYVert3Df & debSegment, NYVert3Df & finSegment,
		NYVert3Df & inter,
		int &xCube, int&yCube, int&zCube)
	{
		float len = (finSegment - debSegment).getSize();

		int x = (int)(debSegment.X / NYCube::CUBE_SIZE);
		int y = (int)(debSegment.Y / NYCube::CUBE_SIZE);
		int z = (int)(debSegment.Z / NYCube::CUBE_SIZE);

		int l = (int)(len / NYCube::CUBE_SIZE) + 1;

		int xDeb = x - l;
		int yDeb = y - l;
		int zDeb = z - l;

		int xFin = x + l;
		int yFin = y + l;
		int zFin = z + l;

		if (xDeb < 0)
			xDeb = 0;
		if (yDeb < 0)
			yDeb = 0;
		if (zDeb < 0)
			zDeb = 0;

		if (xFin >= MAT_X_SIZE_CUBES)
			xFin = MAT_X_SIZE_CUBES - 1;
		if (yFin >= MAT_Y_SIZE_CUBES)
			yFin = MAT_Y_SIZE_CUBES - 1;
		if (zFin >= MAT_Z_SIZE_CUBES)
			zFin = MAT_Z_SIZE_CUBES - 1;

		float minDist = -1;
		NYVert3Df interTmp;
		for (x = xDeb; x <= xFin; x++)
			for (y = yDeb; y <= yFin; y++)
				for (z = zDeb; z <= zFin; z++)
				{
					if (getCube(x, y, z)->isSolid())
					{
						if (getRayCollisionWithCube(debSegment, finSegment, x, y, z, interTmp))
						{
							if ((debSegment - interTmp).getMagnitude() < minDist || minDist == -1)
							{
								minDist = (debSegment - interTmp).getMagnitude();
								inter = interTmp;
								xCube = x;
								yCube = y;
								zCube = z;

							}
						}
					}
				}

		if (minDist != -1)
			return true;

		return false;

	}

	/**
	* De meme cette fonction peut être grandement opitimisée, on a priviligié la clarté
	*/
	bool getRayCollisionWithCube(NYVert3Df & debSegment, NYVert3Df & finSegment,
		int x, int y, int z,
		NYVert3Df & inter)
	{


		float minDist = -1;
		NYVert3Df interTemp;

		//Face1
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}

		//Face2
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}

		//Face3
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}

		//Face4
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}

		//Face5
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 0)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}

		//Face6
		if (intersecDroiteCubeFace(debSegment, finSegment,
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 0)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 1)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			NYVert3Df((x + 1)*NYCube::CUBE_SIZE, (y + 0)*NYCube::CUBE_SIZE, (z + 1)*NYCube::CUBE_SIZE),
			interTemp))
		{
			if ((interTemp - debSegment).getMagnitude() < minDist || minDist == -1)
			{
				minDist = (interTemp - debSegment).getMagnitude();
				inter = interTemp;
			}
		}


		if (minDist < 0)
			return false;

		return true;
	}

	/**
	* Crée une explosiont : suppression de tous les cubes de la matrice dans un rayon donné, pour les ajouter
	* sous forme de box au moteur physique. Conserver les body dans un vector pour pouvoir effectuer leur rendu.
	*/
	void createExplosion(NYVert3Df & position, float size)
	{
		int x = (int)(position.X / NYCube::CUBE_SIZE);
		int y = (int)(position.Y / NYCube::CUBE_SIZE);
		int z = (int)(position.Z / NYCube::CUBE_SIZE);

		int l = (int)(size / NYCube::CUBE_SIZE) + 1;

		int xDeb = x - l;
		int yDeb = y - l;
		int zDeb = z - l;

		int xFin = x + l;
		int yFin = y + l;
		int zFin = z + l;

		if (xDeb < 0)
			xDeb = 0;
		if (yDeb < 0)
			yDeb = 0;
		if (zDeb < 0)
			zDeb = 0;

		if (xFin >= MAT_X_SIZE_CUBES)
			xFin = MAT_X_SIZE_CUBES - 1;
		if (yFin >= MAT_Y_SIZE_CUBES)
			yFin = MAT_Y_SIZE_CUBES - 1;
		if (zFin >= MAT_Z_SIZE_CUBES)
			zFin = MAT_Z_SIZE_CUBES - 1;

		for (x = xDeb; x <= xFin; x++)
			for (y = yDeb; y <= yFin; y++)
				for (z = zDeb; z <= zFin; z++)
				{
					float dist = (position - NYVert3Df(x*NYCube::CUBE_SIZE,
						y*NYCube::CUBE_SIZE,
						z*NYCube::CUBE_SIZE)).getMagnitude();
					if (dist < size*size)
					{
						if (getCube(x, y, z)->isSolid())
						{
							_ExplodedCubesTypes.push_back(getCube(x, y, z)->_Type);
							deleteCube(x, y, z, false);
							btRigidBody * cube = NYBasicPhysicEngine::getInstance()->addBoxObject(true,
								NYVert3Df(NYCube::CUBE_SIZE / 2,
									NYCube::CUBE_SIZE / 2,
									NYCube::CUBE_SIZE / 2),
								NYVert3Df(x*NYCube::CUBE_SIZE + NYCube::CUBE_SIZE / 2,
									y*NYCube::CUBE_SIZE + NYCube::CUBE_SIZE / 2,
									z*NYCube::CUBE_SIZE + NYCube::CUBE_SIZE / 2),
								10.0f);
							cube->applyImpulse(btVector3(0, 0, 500), btVector3(0, 0, 0));
							_ExplodedCubes.push_back(cube);
						}
					}

				}

		updateStructure();
	}

	void render_world_vbo(void)
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::X_CHUNK_SIZE*NYCube::CUBE_SIZE),
						(float)(y*NYChunk::Y_CHUNK_SIZE*NYCube::CUBE_SIZE),
						(float)(z*NYChunk::Z_CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}

		for (int i = 0; i<_ExplodedCubes.size(); i++)
		{
			btMotionState * state = _ExplodedCubes[i]->getMotionState();
			btTransform t;
			state->getWorldTransform(t);
			btScalar m[16];
			t.getOpenGLMatrix(m);

			glEnable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);
			switch (_ExplodedCubesTypes[i])
			{
			case CUBE_TERRE:
				glColor3f(101.0f / 255.0f, 74.0f / 255.0f, 0.0f / 255.0f);
				break;
			case CUBE_HERBE:
				glColor3f(1.0f / 255.0f, 112.0f / 255.0f, 12.0f / 255.0f);
				break;
			case CUBE_EAU:
				glColor3f(0.0f / 255.0f, 48.0f / 255.0f, 255.0f / 255.0f);
				break;
			}

			glPushMatrix();
			glMultMatrixf(m);
			glutSolidCube(NYCube::CUBE_SIZE);
			glPopMatrix();
		}
	}
};

#endif