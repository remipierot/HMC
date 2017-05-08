#ifndef __BASIC_PHYSIC_ENGINE__
#define __BASIC_PHYSIC_ENGINE__

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "LinearMath/btConvexHull.h"

#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log.h"

class NYBasicPhysicEngine
{
	public:
		btDefaultCollisionConfiguration * CollisionConfiguration;
		btCollisionDispatcher * Dispatcher;
		btDbvtBroadphase * Broadphase;
		btSequentialImpulseConstraintSolver * Solver;
		btDiscreteDynamicsWorld * DynamicsWorld;
		
	private:

		static NYBasicPhysicEngine * _Instance;

		NYBasicPhysicEngine()
		{
			CollisionConfiguration = NULL;
			Dispatcher = NULL;
			Broadphase = NULL;
			Solver = NULL;
			DynamicsWorld = NULL;
		}

	public:

		static NYBasicPhysicEngine * getInstance()
		{
			if(!_Instance)
				_Instance = new NYBasicPhysicEngine();
			return _Instance;
		}

		///Initialisation du moteur physique
		void initialisation(NYVert3Df gravity)
		{
			//Gestion des collisions
			CollisionConfiguration = new btDefaultCollisionConfiguration();
			Dispatcher = new btCollisionDispatcher(CollisionConfiguration);

			//Solveur de contraintes
			Solver = new btSequentialImpulseConstraintSolver();

			//Le monde simulé
			Broadphase = new btDbvtBroadphase();
			DynamicsWorld = new btDiscreteDynamicsWorld(Dispatcher,Broadphase,Solver,CollisionConfiguration);

			//Algo de calcul de collisions
			btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(DynamicsWorld->getDispatcher());
			btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

			//On init la gravité
			DynamicsWorld->setGravity(btVector3(gravity.X,gravity.Y,gravity.Z));
			
		}

		btRigidBody * addSphereObject(bool dynamic, float radius, const NYVert3Df & position,float fMass)
		{
			//On crée une sphère shape
			btCollisionShape* colShape = new btSphereShape(btScalar(radius));
			return addObject(dynamic,colShape,position,fMass);
		}

		btRigidBody * addBoxObject(bool dynamic, const NYVert3Df & size, const NYVert3Df & position, float fMass)
		{
			//On crée une box shape
			btCollisionShape* colShape = new btBoxShape(btVector3(size.X,size.Y,size.Z));
			return addObject(dynamic,colShape,position,fMass);
		}

		///Stride : taille en octets d'un point
		btGImpactMeshShape * calcTriMeshShape(float * vertices, int nbVertices,int stride)
		{
			btTriangleMesh * trimesh = new btTriangleMesh();
			Log::log(Log::ENGINE_INFO,("Building trimesh shape of " + toString(nbVertices) + " vertices").c_str());
			
			for (int i = 0; i < nbVertices; i+=3)
			{
				btVector3 vertex0(vertices[3*i], vertices[(3*i)+1],  vertices[(3*i)+2]);
				btVector3 vertex1(vertices[3*(i+1)], vertices[(3*(i+1))+1],  vertices[(3*(i+1))+2]);
				btVector3 vertex2(vertices[3*(i+2)], vertices[(3*(i+2))+1],  vertices[(3*(i+2))+2]);
				
				//Attention au sens de la normale !
				trimesh->addTriangle(vertex2,vertex1,vertex0);
			}

			btGImpactMeshShape * shape = new btGImpactMeshShape(trimesh);
			shape->setMargin(0.01f);
			shape->updateBound();

			//si modif call btGImpactMeshShape.postUpdate()

			return shape;

		}

		
		btRigidBody * addTriMeshObject(bool dynamic, btGImpactMeshShape * colShape, const NYVert3Df & position, float fMass)
		{
			return addObject(dynamic,colShape,position,fMass);
		}

		void removeObject(btRigidBody * body)
		{
			Log::log(Log::ENGINE_INFO,("remove body 0x" + toString(body)).c_str());
			DynamicsWorld->removeRigidBody(body);
		}

		void update (float elapsed)
		{
			if(!DynamicsWorld)
				Log::log(Log::ENGINE_ERROR, "Update d'un moteur physique non initialise !!");
			else
				DynamicsWorld->stepSimulation(elapsed);
		}

		/**
		  * Malheureusement cette fonction n'a aucune impact en ce qui concerne les concave shapes, surement
		  * a cause du fix nécessaire pour avoir le report de collision sur les mesh concaves
		  */
		void setCollisionReportOnBody(btRigidBody * body,bool report = true)
		{
			if(report)
				body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
			else
				body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}


		/**
		  * Besoin d'un fix pour fonctionner avec les concave shapes : 
		  * rajouter 
		  * if(m_manifoldPtr != NULL)
		  *    m_manifoldPtr->refreshContactPoints(body0->getWorldTransform(), body1->getWorldTransform());
		  * a la fin de btGImpactCollisionAlgorithm::processCollision(); (from bullet wiki)
		  */
		void setCollisionReportFunction( bool (*ContactProcessedCallback)(btManifoldPoint& cp, void* body0,void* body1))
		{
			gContactProcessedCallback = ContactProcessedCallback;
		}


	private:

		btRigidBody * addObject(bool dynamic, btCollisionShape* colShape, const NYVert3Df & position, float fMass)
		{
			//On lui donne sa position initiale
			btTransform startTransform;
			startTransform.setIdentity();
			startTransform.setOrigin(btVector3(btScalar(position.X),btScalar(position.Y),btScalar(position.Z)));
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			
			//Inertie initiale
			btVector3 localInertia(0,0,0);
			float mass = 0; ///< Objet inanimé : masse nulle
			if (dynamic) 
			{
				mass = fMass;
				colShape->calculateLocalInertia(mass,localInertia);
			}

			//On construit le rigid body a partir de ces infos			
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody * body = new btRigidBody(rbInfo);

			//On initialise le rigid body
			body->setUserPointer(body); ///< permet de recup un pointeur lors d'un event (collision)
			body->setActivationState(ISLAND_SLEEPING);
			body->setSleepingThresholds(0.01f,0.01f);

			if(dynamic)
				body->setActivationState(ACTIVE_TAG);

			//On l'ajoute au monde
			DynamicsWorld->addRigidBody(body);

			Log::log(Log::ENGINE_INFO,("Adding body 0x" + toString(body)).c_str());

			return body;
		}
};

#endif