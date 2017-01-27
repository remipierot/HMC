#ifndef __WORLD_H__
#define __WORLD_H__

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "OpenSimplexNoise.h"

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_X_SIZE 20 //en nombre de chunks
#define MAT_Y_SIZE 20 //en nombre de chunks
#define MAT_Z_SIZE 1 //en nombre de chunks
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

	NYWorld()
	{
		_FacteurGeneration = 300.0f;

		//On crée les chunks
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					_Chunks[x][y][z] = new NYChunk();

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
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if (x >= MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) x = (MAT_X_SIZE * NYChunk::X_CHUNK_SIZE) - 1;
		if (y >= MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) y = (MAT_Y_SIZE * NYChunk::Y_CHUNK_SIZE) - 1;
		if (z >= MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) z = (MAT_Z_SIZE * NYChunk::Z_CHUNK_SIZE) - 1;
		_Chunks[x / NYChunk::X_CHUNK_SIZE][y / NYChunk::Y_CHUNK_SIZE][z / NYChunk::Z_CHUNK_SIZE]->toVbo();
	}

	void deleteCube(int x, int y, int z)
	{
		NYCube * cube = getCube(x,y,z);
		cube->_Draw = false;
		cube = getCube(x-1,y,z);
		updateCube(x,y,z);	
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
			if (reset == true || z >= (height-1))
			{
				c->_Type = (z == 0)
							? CUBE_EAU
							: (z < height - 1)
								? CUBE_TERRE
								: (z == height - 1)
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

		//diamond_square_generation(profmax);
		perlin_generation();

		/*
		for (int i = 0; i < SMOOTH_PASS; i++)
			lisse();
			*/

		disableHiddenCubes();

		add_world_to_vbo();
	}

	void disableHiddenCubes() 
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
					_Chunks[x][y][z]->disableHiddenCubes();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point)
	{
		return NULL;
	}

	//Boites de collisions plus petites que deux cubes
	NYAxis getMinCol(NYVert3Df pos, float width, float height, float & valueColMin, int i)
	{
		NYAxis axis = 0x00;
		return axis;
	}

	void render_world_vbo(void)
	{
		for (int x = 0; x<MAT_X_SIZE; x++)
			for (int y = 0; y<MAT_Y_SIZE; y++)
				for (int z = 0; z<MAT_Z_SIZE; z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::X_CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(y*NYChunk::Y_CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(z*NYChunk::Z_CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}
	}

	int get_pile_height(float x, float y)
	{
		x = (int)x / NYCube::CUBE_SIZE;
		y = (int)y / NYCube::CUBE_SIZE;

		return (x >= 0 && x < MAT_X_SIZE_CUBES && y >= 0 && y < MAT_Y_SIZE_CUBES) ? _MatriceHeights[(int)x][(int)y] * NYCube::CUBE_SIZE : 0;
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
					tmp_noise = OpenSimplexNoise::eval(x / 16.0f, y / 16.0f, z / 16.0f);
					
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

		Log::log(Log::ENGINE_INFO,(toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{
		NYCube* c;

		GLfloat whiteSpecularMaterial[] = { 0.3, 0.3, 0.3, 1.0 };
		GLfloat mShininess = 100;

		GLfloat earthDiffuse[] = { 255.0f / 255.0f, 98.0f / 255.0f, 46.0f / 255.0f, 1 };
		GLfloat grassDiffuse[] = { 99.0f / 255.0f, 255.0f / 255.0f, 46.0f / 255.0f, 1 };
		GLfloat waterDiffuse[] = { 46.0f / 255.0f, 203.0f / 255.0f, 255.0f / 255.0f, 1 };

		GLfloat earthAmbient[] = { 0.2f * earthDiffuse[0], 0.2f * earthDiffuse[1], 0.2f * earthDiffuse[2], 1 };
		GLfloat grassAmbient[] = { 0.2f * grassDiffuse[0], 0.2f * grassDiffuse[1], 0.2f * grassDiffuse[2], 1 };
		GLfloat waterAmbient[] = { 0.2f * waterDiffuse[0], 0.2f * waterDiffuse[1], 0.2f * waterDiffuse[2], 1 };

		glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterial);
		glMaterialf(GL_FRONT, GL_SHININESS, mShininess);

		for (int x = 0; x < MAT_X_SIZE_CUBES; x++)
		{
			glPushMatrix();
			for (int y = 0; y < MAT_Y_SIZE_CUBES; y++)
			{
				glPushMatrix();
				for (int z = 0; z < _MatriceHeights[x][y]; z++)
				{
					c = getCube(x, y, z);

					if (c->_Draw && c->isSolid())
					{
						glPushMatrix();
						glTranslatef(x * c->CUBE_SIZE, y * c->CUBE_SIZE, z * c->CUBE_SIZE);

						switch (c->_Type)
						{
							case CUBE_HERBE:
								glMaterialfv(GL_FRONT, GL_DIFFUSE, grassDiffuse);
								glMaterialfv(GL_FRONT, GL_AMBIENT, grassAmbient);
								break;
							case CUBE_EAU:
								glMaterialfv(GL_FRONT, GL_DIFFUSE, waterDiffuse);
								glMaterialfv(GL_FRONT, GL_AMBIENT, waterAmbient);
								break;
							case CUBE_TERRE:
								glMaterialfv(GL_FRONT, GL_DIFFUSE, earthDiffuse);
								glMaterialfv(GL_FRONT, GL_AMBIENT, earthAmbient);
								break;
						}

						glutSolidCube(c->CUBE_SIZE);
						glPopMatrix();
					}
					//glTranslatef(0, 0, 5);
				}
				glPopMatrix();
				//glTranslatef(0, 5, 0);
			}
			glPopMatrix();
			//glTranslatef(5, 0, 0);
		}
	}
};

#endif