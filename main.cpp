#include <iostream>
#include <vector>

#include <GL/glew.h>
//#include <GL/gl.h> // OpenGL header not necessary, included by GLEW
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "GLSLProgram.h"
#include "GLTools.h"
#include <cmath>
#include <math.h>
#include <time.h>
#include <ctime>
// Definiert PI
#define M_PI acos(-1.0)

// Standard window width
const int WINDOW_WIDTH  = 640;
// Standard window height
const int WINDOW_HEIGHT = 480;
// GLUT window id/handle
int glutID = 0;

cg::GLSLProgram program;

glm::mat4x4 view;
glm::mat4x4 projection;
std::clock_t start;
float zNear = 0.1f;
float zFar  = 100.0f;
float p2axis = 2.f*(float)M_PI / 8.f;
// Dieser Wert steuert die Höhe des gesamten Planetensystemes. Es wird entlang der Y_Achse verschoben.
float height;
// Dieser Wert steuert die Höhe von Planet 1 und seinen drei Monden. Es wird entlang der Y_Achse verschoben.
float planetHeight;
// Dieser Wert regelt die Geschwindigkeit, in der die Planeten die Sonne umkreisen
float planetSpeed=0.01f;


/*
Struct to hold data for object rendering.
*/
struct Object
{
	/* IDs for several buffers. */
	GLuint vao;

	GLuint positionBuffer;
	GLuint colorBuffer;

	GLuint indexBuffer;

	/* Model matrix */
	glm::mat4x4 model;
};

//variable camera-position
struct viewpoint
{
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
};
glm::mat4x4 xmat;
glm::mat4x4 ymat;
glm::mat4x4 zmat;

// Sonne
Object sun;
// Planet 1 + Monde
Object planet1;
Object moon1p1;
Object moon2p1;
Object moon3p1;
Object moon4p1;
// Planet 2 + Monde
Object planet2;
Object moon1p2;
Object moon2p2;
Object moon3p2;
Object moon4p2;
Object moon5p2;
Object moon6p2;
Object moon7p2;
Object moon8p2;
Object moon9p2;
Object moon10p2;

float win;
// Linien durch die Planetenachsen
Object lines;
// Viewpoint für die Kamera
viewpoint viewp;

// Neuberechnung der Rotationsmatrixen
// Es wird um die jeweilige Achse um den Winkel Radiant rotiert
void refreshMatrix(float winkel)
{
	
	ymat = { 
		cos(winkel),0,sin(winkel),0,
		0,1,0,0,
		-sin(winkel),0,cos(winkel),0,
		0,0,0,1 
	};
	zmat = {
		cos(winkel),-sin(winkel),0,0,
		sin(winkel),cos(winkel),0,0,
		0,0,1,0,
		0,0,0,1
	};
	win = winkel;
}

void renderWire(Object model)
{
	// Create mvp.
	glm::mat4x4 mvp = projection * view * model.model;

	// Bind the shader program and set uniform(s).
	program.use();
	program.setUniform("mvp", mvp);

	// GLUT: bind vertex-array-object
	// this vertex-array-object must be bound before the glutWireSphere call
	glBindVertexArray(model.vao);

	//glLineWidth(1.0f);
	glutWireSphere(1.0, 100, 100);

	// GLUT: unbind vertex-array-object
	glBindVertexArray(0);
}

void renderLines()
{
	// Create mvp.
	glm::mat4x4 mvp = projection * view * lines.model;

	// Bind the shader program and set uniform(s).
	program.use();
	program.setUniform("mvp", mvp);

	// Bind vertex array object so we can render the 2 triangles.
	glBindVertexArray(lines.vao);
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}


void initLines()
{
	// Construct triangle. These vectors can go out of scope after we have send all data to the graphics card.
	const std::vector<glm::vec3> vertices = { planet2.model * glm::vec4(0,-10.0f,0,1), planet2.model * glm::vec4(0,10.0f,0,1),glm::vec3(0.0f, -1000.0f, 0.0f), glm::vec3(0.0f, 1000.0f, 0.0f), planet1.model *glm::vec4(0.0f, -10.0f, 0,1), planet1.model* glm::vec4(0.0f, 10.0f, 0.f,1) };
	const std::vector<glm::vec3> colors = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) };
	const std::vector<GLushort> indices = { 0, 1,2,3,4,5};
	GLuint programId = program.getHandle();
	GLuint pos;

	// Step 0: Create vertex array object.
	glGenVertexArrays(1, &lines.vao);
	glBindVertexArray(lines.vao);

	// Step 1: Create vertex buffer object for position attribute and bind it to the associated "shader attribute".
	glGenBuffers(1, &lines.positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lines.positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	// Bind it to position.
	pos = glGetAttribLocation(programId, "position");
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Step 2: Create vertex buffer object for color attribute and bind it to...
	glGenBuffers(1, &lines.colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lines.colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	// Bind it to color.
	pos = glGetAttribLocation(programId, "color");
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Step 3: Create vertex buffer object for indices. No binding needed here.
	glGenBuffers(1, &lines.indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lines.indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

	// Unbind vertex array object (back to default).
	glBindVertexArray(0);
}

void initWire(Object &model)
{

	// set attribute locations (of shader) for GLUT
	GLuint programId = program.getHandle();
	// position attribute to variable "position"
	glutSetVertexAttribCoord3(glGetAttribLocation(programId, "position"));
	// normal attribute to variable "color"
	// this creates a colorful sphere :-)
	glutSetVertexAttribNormal(glGetAttribLocation(programId, "color"));
	// create a vertex-array-object for GLUT geometry
	glGenVertexArrays(1, &model.vao);

	// Modify model matrix.
	model.model = glm::mat4(1.0f);
}

void initAll()
{
	// Sonne
	initWire(sun);
	sun.model = glm::scale(sun.model, glm::vec3(2, 2, 2));
	// Planet 1 + Monde
	initWire(planet1);
	planet1.model = glm::translate(planet1.model, glm::vec3(-10, 0, 0));
	// Der Mond wird initialisiert
	initWire(moon1p1);
	// Der Mond bekommt die Modelmatrix von seinem Mutterplaneten --> Mond liegt jetzt sozusagen auf dem Mutterplaneten
	moon1p1.model = planet1.model;
	// Der Mond wird durch den Vektor skaliert ( Matrix wird also mit dem Faktor 0.5 multipliziert) --> Mond ist jetzt kleiner geworden
	moon1p1.model = glm::scale(moon1p1.model, glm::vec3(0.5, 0.5, 0.5));
	// Der Mond wird ausgehend vom Mutterplaneten wegtransferiert
	// x =  cos (2* PI / anzSegmente) * radius
	// y = 0 --> Kreis wird durch x,y - Ebene beschrieben
	// z = sin ( 2* PI / anzSegmente) * radius
	moon1p1.model = glm::translate(moon1p1.model, glm::vec3(cos(2 * M_PI / 3) * 10, 0, sin(2 * M_PI / 3) * 10));
	initWire(moon2p1);
	moon2p1.model = planet1.model;
	moon2p1.model = glm::scale(moon2p1.model, glm::vec3(0.5, 0.5, 0.5));
	moon2p1.model = glm::translate(moon2p1.model, glm::vec3(cos(2 * M_PI / 3 * 2) * 10, 0, sin(2 * M_PI / 3 * 2) * 10));
	initWire(moon3p1);
	moon3p1.model = planet1.model;
	moon3p1.model = glm::scale(moon3p1.model, glm::vec3(0.5, 0.5, 0.5));
	moon3p1.model = glm::translate(moon3p1.model, glm::vec3(cos(2 * M_PI) * 10, 0, sin(2 * M_PI) * 10));
	// Planet 2 + Monde
	initWire(planet2);
	planet2.model = glm::translate(planet2.model, glm::vec3(20, 0, 0));
	refreshMatrix(2 * M_PI / 8);
	planet2.model = planet2.model * zmat;
	initWire(moon1p2);
	moon1p2.model = planet2.model;
	moon1p2.model = glm::scale(moon1p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon1p2.model = glm::translate(moon1p2.model, glm::vec3(10,0,0));
	initWire(moon2p2);
	moon2p2.model = planet2.model;
	moon2p2.model = glm::scale(moon2p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon2p2.model = glm::translate(moon2p2.model, glm::vec3(-10, 0, 0));
	initWire(moon3p2);
	moon3p2.model = planet2.model;
	moon3p2.model = glm::scale(moon3p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon3p2.model = glm::translate(moon3p2.model, glm::vec3(-10, 5, 0));
	initWire(moon4p2);
	moon4p2.model = planet2.model;
	moon4p2.model = glm::scale(moon4p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon4p2.model = glm::translate(moon4p2.model, glm::vec3(10, 5, 0));
	initWire(moon5p2);
	moon5p2.model = planet2.model;
	moon5p2.model = glm::scale(moon5p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon5p2.model = glm::translate(moon5p2.model, glm::vec3(0, 5, -10));
	initWire(moon6p2);
	moon6p2.model = planet2.model;
	moon6p2.model = glm::scale(moon6p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon6p2.model = glm::translate(moon6p2.model, glm::vec3(0, 5, 10));
	initWire(moon7p2);
	moon7p2.model = planet2.model;
	moon7p2.model = glm::scale(moon7p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon7p2.model = glm::translate(moon7p2.model, glm::vec3(-10, -5, 0));
	initWire(moon8p2);
	moon8p2.model = planet2.model;
	moon8p2.model = glm::scale(moon8p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon8p2.model = glm::translate(moon8p2.model, glm::vec3(10, -5, 0));
	initWire(moon9p2);
	moon9p2.model = planet2.model;
	moon9p2.model = glm::scale(moon9p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon9p2.model = glm::translate(moon9p2.model, glm::vec3(0, -5, -10));
	initWire(moon10p2);
	moon10p2.model = planet2.model;
	moon10p2.model = glm::scale(moon10p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon10p2.model = glm::translate(moon10p2.model, glm::vec3(0, -5, 10));
}

/*
 Initialization. Should return true if everything is ok and false if something went wrong.
 */
bool init()
{
	// OpenGL: Set "background" color and enable depth testing.
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	viewp.eye = { 0.0f,15.0f,40.0f };
	viewp.center = { 0.0f,0.0f,0.f };
	viewp.up = { 0.0f,1.0f,0.0f };
	view = glm::lookAt(viewp.eye, viewp.center, viewp.up);
	refreshMatrix(0.1);
	// Create a shader program and set light direction.
	if (!program.compileShaderFromFile("shader/simple.vert", cg::GLSLShader::VERTEX))
	{
		std::cerr << program.log();
		return false;
	}

	if (!program.compileShaderFromFile("shader/simple.frag", cg::GLSLShader::FRAGMENT))
	{
		std::cerr << program.log();
		return false;
	}
	
	if (!program.link())
	{
		std::cerr << program.log();
		return false;
	}

	// Create objects.
	initAll();
	initLines();
	return true;
}

void animate()
{
	static float p2rad = { win };
	// Ist der Radius größer 360, wird er wieder auf 0 gesetzt
	if (p2rad >= 2 * M_PI) { p2rad = p2rad - 2 * M_PI; }
	//Sun
	sun.model[3][1] = height;
	//Planet1
	// Wir bilden die Einheitsmatrix, rotieren und skalieren zurück 
	planet1.model = planet1.model * glm::inverse(planet1.model) * ymat * planet1.model;
	planet1.model[3][1] = height + planetHeight;
	//Moon1
	moon1p1.model = glm::translate(glm::translate(moon1p1.model, glm::vec3(-cos(2 * M_PI / 3) * 10, 0, -sin(2 * M_PI / 3) * 10))*glm::inverse(ymat)*glm::inverse(ymat), glm::vec3(cos(2 * M_PI / 3) * 10, 0, sin(2 * M_PI / 3) * 10));
	moon1p1.model = moon1p1.model * glm::inverse(moon1p1.model) * ymat * moon1p1.model;
	moon1p1.model[3][1] = height + planetHeight;
	//Moon2
	moon2p1.model = glm::translate(glm::translate(moon2p1.model, glm::vec3(-cos(2 * M_PI / 3 * 2) * 10, 0, -sin(2 * M_PI / 3 * 2) * 10))*glm::inverse(ymat)*glm::inverse(ymat), glm::vec3(cos(2 * M_PI / 3 * 2) * 10, 0, sin(2 * M_PI / 3 * 2) * 10));
	moon2p1.model = moon2p1.model * glm::inverse(moon2p1.model) * ymat * moon2p1.model;
	moon2p1.model[3][1] = height + planetHeight;
	//Moon3
	moon3p1.model = glm::translate(glm::translate(moon3p1.model, glm::vec3(-cos(2 * M_PI) * 10, 0, -sin(2 * M_PI) * 10))*glm::inverse(ymat)*glm::inverse(ymat), glm::vec3(cos(2 * M_PI) * 10, 0, sin(2 * M_PI) * 10));
	moon3p1.model = moon3p1.model * glm::inverse(moon3p1.model) * ymat * moon3p1.model;
	moon3p1.model[3][1] = height + planetHeight;
	// Linien neu konfigurieren
	initLines();
	//Planet2
	p2rad += win;
	glm::mat4x4 backup = planet2.model;
	planet2.model = planet2.model * glm::inverse(planet2.model);
	planet2.model = glm::rotate(planet2.model, p2axis, glm::vec3(0, 0, 1));
	planet2.model[3][0] = cos(p2rad) * 20;
	planet2.model[3][2] = sin(p2rad) * 20;
	planet2.model = glm::rotate(planet2.model, p2rad, glm::vec3(0, 1, 0));
	planet2.model[3][1] = height;
	//Moon1
	moon1p2.model = planet2.model;
	moon1p2.model = glm::scale(moon1p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon1p2.model = glm::rotate(moon1p2.model, p2rad, glm::vec3(0, 1, 0));
	moon1p2.model = glm::translate(moon1p2.model, glm::vec3(10, 0, 0));
	//Moon2 
	moon2p2.model = planet2.model;
	moon2p2.model = glm::scale(moon2p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon2p2.model = glm::rotate(moon2p2.model, p2rad, glm::vec3(0, 1, 0));
	moon2p2.model = glm::translate(moon2p2.model, glm::vec3(-10, 0, 0));
	//Moon3 
	moon3p2.model = planet2.model;
	moon3p2.model = glm::scale(moon3p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon3p2.model = glm::rotate(moon3p2.model, p2rad, glm::vec3(0, 1, 0));
	moon3p2.model = glm::translate(moon3p2.model, glm::vec3(-10, 5, 0));
	//Moon4
	moon4p2.model = planet2.model;
	moon4p2.model = glm::scale(moon4p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon4p2.model = glm::rotate(moon4p2.model, p2rad, glm::vec3(0, 1, 0));
	moon4p2.model = glm::translate(moon4p2.model, glm::vec3(10, 5, 0));
	//Moon5
	moon5p2.model = planet2.model;
	moon5p2.model = glm::scale(moon5p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon5p2.model = glm::rotate(moon5p2.model, p2rad, glm::vec3(0, 1, 0));
	moon5p2.model = glm::translate(moon5p2.model, glm::vec3(0, 5, -10));
	//Moon6 
	moon6p2.model = planet2.model;
	moon6p2.model = glm::scale(moon6p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon6p2.model = glm::rotate(moon6p2.model, p2rad, glm::vec3(0, 1, 0));
	moon6p2.model = glm::translate(moon6p2.model, glm::vec3(0, 5, 10));
	//Moon7 
	moon7p2.model = planet2.model;
	moon7p2.model = glm::scale(moon7p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon7p2.model = glm::rotate(moon7p2.model, p2rad, glm::vec3(0, 1, 0));
	moon7p2.model = glm::translate(moon7p2.model, glm::vec3(-10, -5, 0));
	//Moon8
	moon8p2.model = planet2.model;
	moon8p2.model = glm::scale(moon8p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon8p2.model = glm::rotate(moon8p2.model, p2rad, glm::vec3(0, 1, 0));
	moon8p2.model = glm::translate(moon8p2.model, glm::vec3(10, -5, 0));
	//Moon9
	moon9p2.model = planet2.model;
	moon9p2.model = glm::scale(moon9p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon9p2.model = glm::rotate(moon9p2.model, p2rad, glm::vec3(0, 1, 0));
	moon9p2.model = glm::translate(moon9p2.model, glm::vec3(0, -5, -10));
	//Moon10 
	moon10p2.model = planet2.model;
	moon10p2.model = glm::scale(moon10p2.model, glm::vec3(0.5, 0.5, 0.5));
	moon10p2.model = glm::rotate(moon10p2.model, p2rad, glm::vec3(0, 1, 0));
	moon10p2.model = glm::translate(moon10p2.model, glm::vec3(0, -5, 10));
}


/*
 Release object resources.
*/
void releaseObject(Object& obj)
{
	glDeleteVertexArrays(1, &obj.vao);
	glDeleteBuffers(1, &obj.indexBuffer);
	glDeleteBuffers(1, &obj.colorBuffer);
	glDeleteBuffers(1, &obj.positionBuffer);
}

/*
 Release resources on termination.
 */
void release()
{
	// Shader program will be released upon program termination.
	//releaseObject(quad);
	releaseObject(sun);
	releaseObject(planet1);
	releaseObject(moon1p1);
	releaseObject(moon2p1);
	releaseObject(moon3p1);
	releaseObject(planet2);
	releaseObject(moon1p2);
	releaseObject(moon2p2);
	releaseObject(moon3p2);
	releaseObject(moon4p2);
	releaseObject(moon5p2);
	releaseObject(moon6p2);
	releaseObject(moon7p2);
	releaseObject(moon8p2);
	releaseObject(moon9p2);
	releaseObject(moon10p2);

}


void renderall()
{
	renderWire(sun);
	renderWire(planet1);
	renderWire(moon1p1);
	renderWire(moon2p1);
	renderWire(moon3p1);
	renderWire(planet2);
	renderWire(moon1p2);
	renderWire(moon2p2);
	renderWire(moon3p2);
	renderWire(moon4p2);
	renderWire(moon5p2);
	renderWire(moon6p2);
	renderWire(moon7p2);
	renderWire(moon8p2);
	renderWire(moon9p2);
	renderWire(moon10p2);

}
/*
 Rendering.
 */
void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	renderall();
	renderLines();
}
void refresh()
{
	animate();
	renderall();
}

void glutDisplay ()
{
   GLCODE(render());
   glutSwapBuffers();
   // Neuberechnung der Rotationsmatrixen.
   refreshMatrix(planetSpeed);
   start = std::clock();
   refresh();
}


/*
 Resize callback.
 */
void glutResize (int width, int height)
{
	// Division by zero is bad...
	height = height < 1 ? 1 : height;
	glViewport(0, 0, width, height);

	// Construct projection matrix.
	projection = glm::perspective(45.0f, (float) width / height, zNear, zFar);
}

/*
 Callback for char input.
 */
void glutKeyboard (unsigned char keycode, int x, int y)
{
	const float radiant = 0.1;

	switch (keycode)
	{
	case 27: // ESC
	  glutDestroyWindow ( glutID );
	  return;
	case 'p':
		p2axis += 0.1;
		if (p2axis >= 2 * M_PI) { p2axis = p2axis - 2 * M_PI; }
		break;
	case 'P':
		p2axis -= 0.1;
		if (p2axis <= 0) { p2axis = 2*M_PI - p2axis; }
		break;
	case 't':
		height += 0.1f;
		break;
	case 'T':
		height -= 0.1f;
		break;
	case 'l':
		planetHeight += 0.1f;
		break;
	case 'L':
		planetHeight -= 0.1f;
		break;
	case 'a':
		viewp.eye.z += 0.1;
		view = glm::lookAt(viewp.eye, viewp.center, viewp.up);
		break;
	case 's':
		viewp.eye.z -= 0.1;
		view = glm::lookAt(viewp.eye, viewp.center, viewp.up);
		break;
	case 'w':
		if (planetSpeed > 0) {
			planetSpeed = planetSpeed - 0.01f;
		}
		break;
	case 'W':
		planetSpeed = planetSpeed + 0.01f;
		break;

	}
	
	glutPostRedisplay();
}



int main(int argc, char** argv)
{
	// GLUT: Initialize freeglut library (window toolkit).
        glutInitWindowSize    (WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(40,40);
	glutInit(&argc, argv);

	// GLUT: Create a window and opengl context (version 4.3 core profile).
	glutInitContextVersion(4, 3);
	glutInitContextFlags  (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitDisplayMode   (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glutCreateWindow("Aufgabenblatt 4");
	glutID = glutGetWindow();
	  
	// GLEW: Load opengl extensions
	glewExperimental = GL_TRUE;
	GLenum result = glewInit();

	if (result != GLEW_OK) {
	   return -1;
	}

	// GLUT: Set callbacks for events.
	glutReshapeFunc(glutResize);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc   (glutDisplay); // redisplay when idle
	glutKeyboardFunc(glutKeyboard);
	start = std::clock();
	// Init VAO.
	{
		GLCODE(bool result = init());
		if (!result) {
			release();
			return -2;
		}
	}

	// GLUT: Loop until the user closes the window
	// rendering & event handling
	glutMainLoop ();
	// Clean up everything on termination.
	release();
	

	return 0;
}