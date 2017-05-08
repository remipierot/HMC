#ifndef __SIMPLE_CAM_H__
#define __SIMPLE_CAM_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"

class NYCamera
{
	public:
		NYVert3Df _Position; ///< Position de la camera
		NYVert3Df _LookAt; ///< Point regarde par la camera
		NYVert3Df _Direction; ///< Direction de la camera
		NYVert3Df _UpVec; ///< Vecteur up de la camera
		NYVert3Df _NormVec; ///< Si on se place dans la camera, indique la droite	
		NYVert3Df _UpRef; ///< Ce qu'on considère comme le "haut" dans notre monde (et pas le up de la cam)

		NYCamera()
		{
			_Position = NYVert3Df(0,-1,0);
			_LookAt = NYVert3Df(0,0,0);
			_UpRef = NYVert3Df(0,0,1);
			_UpVec = _UpRef;
			updateVecs();
		}

		/**
		  * Mise a jour de la camera                          
		  */
		virtual void update(float elapsed)
		{
		
		}

		/**
		  * Definition du point regarde
		  */
		void setLookAt(NYVert3Df lookat)
		{
			_LookAt = lookat;
			updateVecs();
		}

		/**
		  * Definition de la position de la camera
		  */
		void setPosition(NYVert3Df pos)
		{
			_Position = pos;
			updateVecs();
		}

		/**
		  * Definition du haut de notre monde
		  */
		void setUpRef(NYVert3Df upRef)
		{
			_UpRef = upRef;
			updateVecs();
		}

		/**
		  * Deplacement de la camera d'un delta donné
		  */
		void move(NYVert3Df delta)
		{
			delta = (_Direction * delta.X) + (_NormVec * delta.Y) + (_UpVec * delta.Z);
			_Position += delta;
			_LookAt += delta;
			updateVecs();
		}

		/**
		  * Deplacement de la camera d'un delta donné
		  */
		void moveTo(NYVert3Df & target)
		{
			NYVert3Df delta = target - _Position;
			_Position += delta;
			_LookAt += delta;
			updateVecs();
		}

		/**
		  * On recalcule les vecteurs utiles au déplacement de la camera (_Direction, _NormVec, _UpVec)
		  * on part du principe que sont connus :
		  * - la position de la camera
		  * - le point regarde par la camera
		  * - la vecteur up de notre monde
		  */
		void updateVecs(void)
		{	
			_Direction = (_LookAt - _Position).normalize();
			_NormVec = _Direction.vecProd(_UpRef).normalize();
			_UpVec = _NormVec.vecProd(_Direction).normalize();
		}

		/**
		  * Rotation droite gauche en subjectif
		  */
		void rotate(float angle)
		{
			_LookAt = (_LookAt - _Position).rotate(_UpVec, angle) + _Position;
			updateVecs();
		}

		/**
		  * Rotation haut bas en subjectif
		  */
		void rotateUp(float angle)
		{
			NYVert3Df tmp = (_LookAt - _Position).rotate(_NormVec, angle) + _Position;
			if (abs((tmp - _Position).normalize().scalProd(_UpRef)) < 0.99f)
			{
				_LookAt = tmp;
				updateVecs();
			}
		}

		/**
		  * Rotation droite gauche en troisième personne
		  */
		void rotateAround(float angle)
		{

		}

		/**
		  * Rotation haut bas en troisième personne
		  */
		void rotateUpAround(float angle)
		{		

		}
	
		/**
		  * Calcul du bon repère de départ pour la matrice monde 
		  */
		void look(void)
		{
			gluLookAt(_Position.X, _Position.Y,_Position.Z,_LookAt.X,_LookAt.Y,_LookAt.Z,_UpVec.X,_UpVec.Y,_UpVec.Z);
		}
};




#endif