#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"

class NYAvatar
{
	public :
		NYVert3Df Position;
		NYVert3Df Speed;

		NYVert3Df Forward;
		NYVert3Df Right;
		
		bool Move;
		bool Jump;
		float Height;
		float Width;
		bool avance;
		bool recule;
		bool gauche;
		bool droite;
		bool Standing;
		bool fly;

		NYCamera * Cam;
		NYWorld * World;

		NYAvatar(NYCamera * cam, NYWorld * world)
		{
			Position = NYVert3Df(0, 0, 0);
			
			Height = 10;
			Width = 3;
			Cam = cam;
			Cam->moveTo(Position);

			Forward = Cam->_Direction;
			Right = Cam->_NormVec;
			avance = false;
			recule = false;
			gauche = false;
			droite = false;
			Standing = false;
			Jump = false;
			fly = false;
			World = world;
		}

		void render(void)
		{
			glutSolidCube(Width/2);
		}

		void update(float elapsed)
		{
			Move = avance || recule || droite || gauche;

			if (Move)
				move(elapsed);
		}

		void move(float elapsed)
		{
			Speed = (Forward * (avance - recule)) + (Right * (droite - gauche));
			Speed *= elapsed;
			Speed *= 500.0f;

			Position += Speed;
			//Position.Z = (World->get_pile_height(Position.X, Position.Y) + NYCube::CUBE_SIZE * Height / 2.0f);

			Cam->moveTo(Position);
		}

		void rotate(float angle)
		{
			Cam->rotate(angle);
			Forward = Cam->_Direction;
			Right = Cam->_NormVec;

			if (!fly)
			{
				Forward.Z = 0;
				Right.Z = 0;
				Forward = Forward.normalize();
				Right = Right.normalize();
			}
		}

		void rotateUp(float angle)
		{
			Cam->rotateUp(angle);

			if (fly)
			{
				Forward = Cam->_Direction;
				Right = Cam->_NormVec;
			}
		}
};

#endif