//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>
#include <ctime>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

//Pour avoir le monde
#include "world.h"
#include "avatar.h"

#define ASKED_MODE true
#define ZQSD_MODE false
#define FACE_NORMALS true
#define VERTEX_NORMALS false
#define DAY_LENGTH_IN_MS 20000
#define HALF_DAY_LENGTH_IN_MS (DAY_LENGTH_IN_MS/2)
#define QUARTER_DAY_LENGTH_IN_MS (HALF_DAY_LENGTH_IN_MS/2)
#define SUN_DISTANCE 10000
#define SUN_SIZE (SUN_DISTANCE / 3)

enum KB_PRESSED
{
	Z_KEY,
	Q_KEY,
	S_KEY,
	D_KEY,
	CTRL_KEY
};

void setLights();

bool kb_inputs[5];

NYWorld * g_world;
NYAvatar * g_avatar;
NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;

bool movement_mode = ASKED_MODE;
bool capture_mouse = true;
float movement_speed = 500.0f;
float daytime = 0;
bool isDay = true;

NYColor dawn_sky = NYColor(200.0f / 255.0f, 236.0f / 255.0f, 255.0f / 255.0f, 1);
NYColor day_sky = NYColor(251.0f/255.0f, 255.0f/255.0f, 140/255.0f, 1);
NYColor dusk_sky = NYColor(107.0f / 255.0f, 79.0f / 255.0f, 40.0f / 255.0f, 1);
NYColor night_sky = NYColor(.0f / 255.0f, .0f / 255.0f, .0f / 255.0f, 1);

NYColor dawn_sun = NYColor(255.0f / 255.0f, 126.0f / 255.0f, 46.0f / 255.0f, 1);
NYColor day_sun = NYColor(255.0f / 255.0f, 252.0f / 255.0f, 46.0f / 255.0f, 1);
NYColor dusk_sun = NYColor(255.0f / 255.0f, 119.0f / 255.0f, 46.0f / 255.0f, 1);
NYColor night_sun = NYColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1);

NYColor sun_color;
NYVert3Df sun_pos;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCamPos = NULL;
GUILabel * LabelCamForward = NULL;
GUILabel * LabelCamRight = NULL;
GUILabel * LabelCamUp = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;


//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);

	static float g_eval_elapsed = 0;

	if (capture_mouse)
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2.0f, glutGet(GLUT_WINDOW_HEIGHT) / 2.0f);
	}

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);

		/*
		LabelCamPos->Text = "Position: " + g_renderer->_Camera->_Position.toStr();
		LabelCamForward->Text = "Forward : " + g_renderer->_Camera->_Direction.toStr();
		LabelCamRight->Text = "Right   : " + g_renderer->_Camera->_NormVec.toStr();
		LabelCamUp->Text = "Up      : " + g_renderer->_Camera->_UpVec.toStr();
		*/

		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	g_avatar->avance = kb_inputs[Z_KEY];
	g_avatar->recule = kb_inputs[S_KEY];
	g_avatar->droite = kb_inputs[D_KEY];
	g_avatar->gauche = kb_inputs[Q_KEY];
	g_avatar->update(elapsed);

	//XY movement if ZQSD_MODE
	if (movement_mode == ZQSD_MODE && (kb_inputs[Z_KEY] || kb_inputs[Q_KEY] || kb_inputs[S_KEY] || kb_inputs[D_KEY]))
	{
		g_renderer->_Camera->move(
			NYVert3Df(
				kb_inputs[Z_KEY] - kb_inputs[S_KEY],
				kb_inputs[D_KEY] - kb_inputs[Q_KEY],
				0
			) * elapsed * movement_speed
		);
	}

	//Time of day
	daytime += (NYRenderer::_DeltaTime * 1000); 
	daytime = (int)daytime % DAY_LENGTH_IN_MS;

	if (daytime < HALF_DAY_LENGTH_IN_MS && !isDay)
		cout << "SWITCH TO DAY - " << sun_pos.toStr() << endl;
	else if (daytime > HALF_DAY_LENGTH_IN_MS && isDay)
		cout << "SWITCH TO NIGHT - " << sun_pos.toStr() << endl;

	isDay = daytime < HALF_DAY_LENGTH_IN_MS;

	//Rendu
	g_renderer->render(elapsed);
}

void coloredCube(bool normal_mode)
{
	GLfloat whiteSpecularMaterial[] = { 0.3, 0.3, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterial);
	GLfloat mShininess = 100;
	glMaterialf(GL_FRONT, GL_SHININESS, mShininess);

	glBegin(GL_QUADS);

	GLfloat redDiffuse[] = { 0.7f, 0, 0, 1};
	GLfloat greenDiffuse[] = { 0, 0.7f, 0, 1 };
	GLfloat blueDiffuse[] = { 0, 0, 0.7f, 1 };

	GLfloat redAmbient[] = { 0.4f, 0, 0, 1 };
	GLfloat greenAmbient[] = { 0, 0.4f, 0, 1 };
	GLfloat blueAmbient[] = { 0, 0, 0.4f, 1 };

	// Vertices coordinates
	NYVert3Df points[] = 
	{ 
		NYVert3Df(-1, -1, -1),
		NYVert3Df(-1, -1, 1),
		NYVert3Df(-1, 1, -1),
		NYVert3Df(-1, 1, 1),
		NYVert3Df(1, -1, -1),
		NYVert3Df(1, -1, 1),
		NYVert3Df(1, 1, -1),
		NYVert3Df(1, 1, 1)
	};

	// Index of vertices composing the faces
	NYVert4Df faces[] =
	{
		NYVert4Df(0, 4, 5, 1),
		NYVert4Df(2, 3, 7, 6),
		NYVert4Df(4, 6, 7, 5),
		NYVert4Df(0, 1, 3, 2),
		NYVert4Df(1, 5, 7, 3),
		NYVert4Df(0, 2, 6, 4)
	};

	// Normals of faces
	NYVert3Df facesNormals[] =
	{
		NYVert3Df(0, -1, 0),
		NYVert3Df(0, 1, 0),
		NYVert3Df(1, 0, 0),
		NYVert3Df(-1, 0, 0),
		NYVert3Df(0, 0, 1),
		NYVert3Df(0, 0, -1)
	};

	// Normals of vertices
	NYVert3Df pointsNormals[] =
	{
		points[0] / 3.0f,
		points[1] / 3.0f,
		points[2] / 3.0f,
		points[3] / 3.0f,
		points[4] / 3.0f,
		points[5] / 3.0f,
		points[6] / 3.0f,
		points[7] / 3.0f
	};

	int pointIndex = 0;
	for (int currFace = 0; currFace < 6; currFace++)
	{
		if (currFace < 2)
		{
			glMaterialfv(GL_FRONT, GL_DIFFUSE, greenDiffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT, greenAmbient);
		}
		else if (currFace < 4)
		{
			glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT, redAmbient);
		}
		else
		{
			glMaterialfv(GL_FRONT, GL_DIFFUSE, blueDiffuse);
			glMaterialfv(GL_FRONT, GL_AMBIENT, blueAmbient);
		}

		if (normal_mode == FACE_NORMALS)
		{
			glNormal3f(
				facesNormals[currFace].X,
				facesNormals[currFace].Y,
				facesNormals[currFace].Z
			);
		}

		for (int currPoint = 0; currPoint < 4; currPoint++)
		{
			switch (currPoint)
			{
				case 0 :
					pointIndex = faces[currFace].X;
					break;
				case 1:
					pointIndex = faces[currFace].Y;
					break;
				case 2:
					pointIndex = faces[currFace].Z;
					break;
				case 3:
					pointIndex = faces[currFace].T;
					break;
			}

			if (normal_mode == VERTEX_NORMALS)
			{
				glNormal3f(
					pointsNormals[pointIndex].X,
					pointsNormals[pointIndex].Y,
					pointsNormals[pointIndex].Z
				);
			}

			glVertex3f(
				points[pointIndex].X, 
				points[pointIndex].Y, 
				points[pointIndex].Z
			);
		}
	}

	glEnd();
}

void coloredSun()
{
	GLfloat sunDiffuse[] = { sun_color.R, sun_color.V, sun_color.B, 1 };
	GLfloat sunAmbient[] = { 0.2f * sun_color.R, 0.2f * sun_color.V, 0.2f * sun_color.B, 1 };

	//Material du soleil : de l'emissive
	glMaterialfv(GL_FRONT, GL_DIFFUSE, sunDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, sunAmbient);

	GLfloat sunEmissionMaterial[] = { sun_color.R, sun_color.V, sun_color.B, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);

	//On dessine un cube pour le soleil
	glutSolidCube(SUN_SIZE);

	//On reset le material emissive pour la suite
	sunEmissionMaterial[0] = 0.0f;
	sunEmissionMaterial[1] = 0.0f;
	sunEmissionMaterial[2] = 0.0f;
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);
}

void render2d(void)
{
	g_screen_manager->render();
}

void renderObjects(void)
{
	//Rendu des axes
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	glColor3d(1,0,0);
	glVertex3d(0,0,0);
	glVertex3d(10000,0,0);
	glColor3d(0,1,0);
	glVertex3d(0,0,0);
	glVertex3d(0,10000,0);
	glColor3d(0,0,1);
	glVertex3d(0,0,0);
	glVertex3d(0,0,10000);
	glEnd();

	setLights();
	glPushMatrix();
	glTranslatef(sun_pos.X, sun_pos.Y, sun_pos.Z);
	coloredSun();
	glPopMatrix();

	glPushMatrix();
	g_world->render_world_vbo();
	//g_world->render_world_old_school();
	glPopMatrix();
}

void setLights(void)
{
	float radAngle = (360.0f * (daytime / (float)DAY_LENGTH_IN_MS)) * 3.14159265f / 180.0f;
	sun_pos = NYVert3Df(0, SUN_DISTANCE, 0).rotate(NYVert3Df(1, 0, 0), radAngle) + g_renderer->_Camera->_Position;

	//On active l'illumination 
	glEnable(GL_LIGHTING);

	//On active la light 0
	glEnable(GL_LIGHT0);

	//On d�finit une lumi�re 
	float position[4] = { sun_pos.X, sun_pos.Y, sun_pos.Z, 0 }; // w = 1 donc c'est une point light (w=0 -> directionelle, point � l'infini)
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	float specular[4] = { 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	float alpha;

	NYColor origin_color, dest_color;
	alpha = (float)((int)daytime%QUARTER_DAY_LENGTH_IN_MS) / (float)QUARTER_DAY_LENGTH_IN_MS;

	if (isDay)
	{
		dest_color = (daytime < QUARTER_DAY_LENGTH_IN_MS) ? day_sun : dusk_sun;
		origin_color = (daytime < QUARTER_DAY_LENGTH_IN_MS) ? dawn_sun : day_sun;
		sun_color = origin_color.interpolate(dest_color, alpha);

		dest_color = (daytime < QUARTER_DAY_LENGTH_IN_MS) ? day_sky : dusk_sky;
		origin_color = (daytime < QUARTER_DAY_LENGTH_IN_MS) ? dawn_sky : day_sky;
		g_renderer->setBackgroundColor(origin_color.interpolate(dest_color, alpha));
	}
	else
	{
		dest_color = (daytime < (HALF_DAY_LENGTH_IN_MS + QUARTER_DAY_LENGTH_IN_MS)) ? night_sun : dawn_sun;
		origin_color = (daytime < (HALF_DAY_LENGTH_IN_MS + QUARTER_DAY_LENGTH_IN_MS)) ? dusk_sun : night_sun;
		sun_color = origin_color.interpolate(dest_color, alpha);

		dest_color = (daytime < (HALF_DAY_LENGTH_IN_MS + QUARTER_DAY_LENGTH_IN_MS)) ? night_sky : dawn_sky;
		origin_color = (daytime < (HALF_DAY_LENGTH_IN_MS + QUARTER_DAY_LENGTH_IN_MS)) ? dusk_sky : night_sky;
		g_renderer->setBackgroundColor(origin_color.interpolate(dest_color, alpha));
	}

	float color[4] = { g_renderer->_BackGroundColor.R, g_renderer->_BackGroundColor.V, g_renderer->_BackGroundColor.B, 1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	float color2[4] = { g_renderer->_BackGroundColor.R, g_renderer->_BackGroundColor.V, g_renderer->_BackGroundColor.B, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
}

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width,height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2)
{
	//On change de mode de camera
	if(key == GLUT_KEY_LEFT)
	{
	}

	if (key == GLUT_KEY_CTRL_L || key == GLUT_KEY_CTRL_R)
	{
		kb_inputs[CTRL_KEY] = true;
	}
}

void specialUpFunction(int key, int p1, int p2)
{
	if (key == GLUT_KEY_CTRL_L || key == GLUT_KEY_CTRL_R)
	{
		kb_inputs[CTRL_KEY] = false;
	}
}

void keyboardDownFunction(unsigned char key, int p1, int p2)
{
	if(key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);	
		exit(0);
	}

	if(key == 'f')
	{
		int screen_width = 800;
		int screen_height = 600;

		if(!g_fullscreen){
			glutFullScreen();
			g_fullscreen = true;
		} else if(g_fullscreen){
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(screen_width, screen_height);
			glutPositionWindow(
				(GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2.0f, 
				(GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2.0f
			);
			g_fullscreen = false;
		}
	}

	if (key == 't')
	{
		g_world->init_world();
	}

	switch (key)
	{
		case 'z':
			kb_inputs[Z_KEY] = true;
			break;
		case 'q':
			kb_inputs[Q_KEY] = true;
			break;
		case 's':
			kb_inputs[S_KEY] = true;
			break;
		case 'd':
			kb_inputs[D_KEY] = true;
			break;
		case 'c':
			capture_mouse = !capture_mouse;
			glutSetCursor((GLUT_CURSOR_NONE * capture_mouse) + (GLUT_CURSOR_LEFT_ARROW * !capture_mouse));
			break;
	}
}

void keyboardUpFunction(unsigned char key, int p1, int p2)
{
	switch (key)
	{
		case 'z': 
			kb_inputs[Z_KEY] = false;
			break;
		case 'q':
			kb_inputs[Q_KEY] = false;
			break;
		case 's':
			kb_inputs[S_KEY] = false;
			break;
		case 'd':
			kb_inputs[D_KEY] = false;
			break;
	}
}

void mouseWheelFunction(int wheel, int dir, int x, int y)
{
	//Z movement when in ASKED_MODE
	if (kb_inputs[CTRL_KEY] && movement_mode == ASKED_MODE)
	{
		g_renderer->_Camera->move(NYVert3Df(0, 0, dir));
	}
}

void mouseFunction(int button, int state, int x, int y)
{
	//Gestion de la roulette de la souris
	if((button & 0x07) == 3 && state)
		mouseWheelFunction(button,1,x,y);
	if((button & 0x07) == 4 && state)
		mouseWheelFunction(button,-1,x,y);

	//GUI
	g_mouse_btn_gui_state = 0;
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;
	
	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
}

void mouseMoveFunction(int x, int y, bool pressed)
{
	bool mouseTraite = false;

	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);
	if (pressed && mouseTraite)
	{
		//Mise a jour des variables li�es aux sliders
	}

	x = -x + glutGet(GLUT_WINDOW_WIDTH) / 2.0f;
	y = -y + glutGet(GLUT_WINDOW_HEIGHT) / 2.0f;

	//Head orientation (no matter the movement_mode)
	if (capture_mouse)
	{
		g_avatar->rotate(x / 500.0f);
		g_avatar->rotateUp(y / 500.0f);
		/*
		g_renderer->_Camera->rotate(x / 500.0f);
		g_renderer->_Camera->rotateUp(y / 500.0f);
		*/
		
	}

	//XY movement when in ASKED_MODE
	if (kb_inputs[CTRL_KEY] && movement_mode == ASKED_MODE)
	{
		g_renderer->_Camera->move(NYVert3Df(y / 10.0f, -x / 10.0f, 0));
	}
}

void mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,true);
}

void mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,false);
}

void clickBtnParams (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{ 
	LogConsole::createInstance();

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv); 
	glutInitContextVersion(3,0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(
		(GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2.0f, 
		(GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2.0f
	);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE );

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());	
	bool gameMode = true;
	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if(gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);
		
		char gameModeStr[200];
		sprintf(gameModeStr,"%dx%d:32@60",width,height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width,screen_height);
	}

	if(g_main_window_id < 1) 
	{
		Log::log(Log::ENGINE_ERROR,"Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}
	
	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR,("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s",glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacit�s du syst�me
	Log::log(Log::ENGINE_INFO,("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

	glutDisplayFunc(update);
	glutReshapeFunc(resizeFunction);
	glutKeyboardFunc(keyboardDownFunction);
	glutKeyboardUpFunc(keyboardUpFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(mouseMoveActiveFunction);
	glutPassiveMotionFunc(mouseMovePassiveFunction);
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRender2DFun(render2d);
	g_renderer->setLightsFun(setLights);

	//Changement de la couleur de fond
	NYColor skyColor(0, 0, 0, 1);
	g_renderer->setBackgroundColor(skyColor);

	g_renderer->initialise();

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);
	
	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen(); 

	g_screen_manager = new GUIScreenManager();
		
	//Bouton pour afficher les params
	/*
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick(clickBtnParams);
	g_screen_jeu->addElement(BtnParams);

	y += BtnParams->Height + 1;
	*/

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	//Ecran de parametrage
	/*
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y+=10;
	x+=10;

	GUILabel * label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider = new GUISlider();
	g_slider->setPos(x,y);
	g_slider->setMaxMin(1,0);
	g_slider->Visible = true;
	g_screen_params->addElement(g_slider);

	y += g_slider->Height + 1;
	y+=10;

	x = 10;
	y += 15;
	LabelCamPos = new GUILabel();
	LabelCamPos->X = x;
	LabelCamPos->Y = y;
	LabelCamPos->Text = "Position: " + g_renderer->_Camera->_Position.toStr();
	g_screen_jeu->addElement(LabelCamPos);

	y += 10;
	LabelCamForward = new GUILabel();
	LabelCamForward->X = x;
	LabelCamForward->Y = y;
	LabelCamForward->Text = "Forward : " + g_renderer->_Camera->_Direction.toStr();
	g_screen_jeu->addElement(LabelCamForward);

	y += 10;
	LabelCamRight = new GUILabel();
	LabelCamRight->X = x;
	LabelCamRight->Y = y;
	LabelCamRight->Text = "Right   : " + g_renderer->_Camera->_NormVec.toStr();
	g_screen_jeu->addElement(LabelCamRight);

	y += 10;
	LabelCamUp = new GUILabel();
	LabelCamUp->X = x;
	LabelCamUp->Y = y;
	LabelCamUp->Text = "Up      : " + g_renderer->_Camera->_UpVec.toStr();
	g_screen_jeu->addElement(LabelCamUp);
	*/

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);
	
	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(10,10,10));
	g_renderer->_Camera->setLookAt(NYVert3Df(0,0,0));

	//Fin init moteur

	//Init application
	//Hide cursor
	glutSetCursor(GLUT_CURSOR_NONE);

	//Init Timer
	g_timer = new NYTimer();
	
	//On start
	g_timer->start();

	//A la fin du main, on genere un monde
	g_world = new NYWorld();
	g_world->_FacteurGeneration = 5;
	g_world->init_world();

	g_avatar = new NYAvatar(g_renderer->_Camera, g_world);
	g_avatar->fly = true;

	glutMainLoop(); 

	return 0;
}