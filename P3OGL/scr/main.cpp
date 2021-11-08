#include "BOX.h"
#include "PYRAMID.h"

#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

const float ORBIT_RADIUS = 10.0f;
float bezier_t = 0.0f;

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model_box1 = glm::mat4(1.0f);
glm::mat4	model_box2 = glm::mat4(1.0f);
glm::mat4	model_box3 = glm::mat4(1.0f);
glm::mat4	model_pyramid = glm::mat4(1.0f);

//Variables de la cámara
float cameraX = 0.0f, cameraZ = -10.0f, cameraAlphaY = 0.0f, cameraAlphaX = 0.0f;
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 right = glm::vec3(-10.0f, 0.0f, -10.0f);

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//Variables manejadoras (Handlers de los shaders y el programa "linker" entre vert y frag)
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform 
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uModel;

//Texturas Uniform
int uColorTex;
int uEmiTex;

//Atributos 
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//Texturas
unsigned int colorTexId;
unsigned int emiTexId;

//Lights
unsigned int uLightPosition;
glm::vec3 lightPosition = glm::vec3(0.0f);
unsigned int uLightIntensity;
glm::vec3 lightIntensity = glm::vec3(1.0);	// == diffusa

//VAO punto medio entre atributos y datos del VBO
unsigned int vao;

//VBOs que forman parte del objeto 
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

//PYRAMID
/*
unsigned int posVBO2;
unsigned int colorVBO2;
unsigned int normalVBO2;
unsigned int texCoordVBO2;
unsigned int triangleIndexVBO2;
*/

//Ancho y alto de la ventana
int w, h;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Calcular punto de Bezier
glm::vec3 calcularPuntoBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float t);

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char* vname, const char* fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
GLuint loadShader(const char* fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
unsigned int loadTex(const char* fileName);


int main(int argc, char** argv) {
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);

}

void initOGL() {

	//Se habilita el test de profundidad "zBuffer" y se pinta el fondo de renderizado de un color gris, casi negro
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -10;

}

void destroy() {

	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);
	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	
	//PYRAMID
	/*
	glDeleteBuffers(1, &posVBO2);
	glDeleteBuffers(1, &colorVBO2);
	glDeleteBuffers(1, &normalVBO2);
	glDeleteBuffers(1, &texCoordVBO2);
	glDeleteBuffers(1, &triangleIndexVBO2);
	*/

	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

}

void initShader(const char* vname, const char* fname) {

	//Cargamos los shaders y almacenamos sus identificadores para luego enlazarlos a través del programa
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	//Inicialización del programa de enlazado
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	//Comprobamos errores de enlazado del vert shader y el frag shader a través del programa
	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		//Calculamos una cadena de error 
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	//Se pueden asignar valores a cada atributo de esta forma o mediante layout(location=idx) en el propio shader
	//glBindAttribLocation(program, 0, "inPos");
	//glBindAttribLocation(program, 1, "inColor");
	//glBindAttribLocation(program, 2, "inNormal"); 
	//glBindAttribLocation(program, 3, "inTexCoord");

	//Establecemos variables uniformes y atributos buscándolas en el servidor y en el cliente
	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
	uModel = glGetUniformLocation(program, "modelViewProj");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");

	//Identificadores de las variables uniformes de textura
	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	//Identificadores de las variables uniformes de iluminación
	uLightPosition = glGetUniformLocation(program, "lpos");
	uLightIntensity = glGetUniformLocation(program, "Id");



}

void initObj() {

	//Configuración de los Vertex Buffer Objects
	glGenBuffers(1, &posVBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &normalVBO);
	glGenBuffers(1, &texCoordVBO);

	//Configuración del Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//////////////////////////////////////////////////////	BOX	//////////////////////////////////////////////////////
	glBindBuffer(GL_ARRAY_BUFFER, posVBO); //Se habilita el Vertex Buffer Object que vamos a configurar a continuación
	//Se reserva espacio en memoria y se suben los datos
	//glBufferData reserva memoria y la carga, para solo reservar memoria en vez de cubeVertexPos 
	//se pasa un NULL y se utiliza glBufferSubData() para subir los datos
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);  //Se especifica la configuración del VBO
	if (inPos != -1) glEnableVertexAttribArray(0); //Si no se necesita se desactiva

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if (inColor != -1) glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if (inNormal != -1) glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	if (inTexCoord != -1) glEnableVertexAttribArray(3);

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex, GL_STATIC_DRAW);

	//Inicialización de la matriz model
	model_box1 = glm::mat4(1.0f);
	model_box2 = glm::mat4(1.0f);
	model_box3 = glm::mat4(1.0f);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	//////////////////////////////////////////////////////	PYRAMID	//////////////////////////////////////////////////////	
	/*//Configuración de los Vertex Buffer Objects
	glGenBuffers(1, &posVBO2);
	glGenBuffers(1, &colorVBO2);
	glGenBuffers(1, &normalVBO2);
	glGenBuffers(1, &texCoordVBO);

	glBindBuffer(GL_ARRAY_BUFFER, posVBO2);
	glBufferData(GL_ARRAY_BUFFER, pyramidNVertex * sizeof(float) * 3, pyramidVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);  //Se especifica la configuración del VBO
	if (inPos != -1) glEnableVertexAttribArray(0); //Si no se necesita se desactiva

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO2);
	glBufferData(GL_ARRAY_BUFFER, pyramidNVertex * sizeof(float) * 3, pyramidVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if (inColor != -1) glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO2);
	glBufferData(GL_ARRAY_BUFFER, pyramidNVertex * sizeof(float) * 3, pyramidVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if (inNormal != -1) glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO2);
	glBufferData(GL_ARRAY_BUFFER, pyramidNVertex * sizeof(float) * 2, pyramidVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	if (inTexCoord != -1) glEnableVertexAttribArray(3);

	glGenBuffers(1, &triangleIndexVBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, pyramidNTriangleIndex * sizeof(unsigned int) * 3, pyramidTriangleIndex, GL_STATIC_DRAW);

	//Inicialización de la matriz model
	model_pyramid = glm::mat4(1.0f);
	*/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	//Creación de texturas
	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");

}

GLuint loadShader(const char* fileName, GLenum type) {

	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	////////////////////////////////////////////// 
	//Creación y compilación del Shader 
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien 
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		//Calculamos una cadena de error 
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader;

}

unsigned int loadTex(const char* fileName) {

	unsigned char* map;
	unsigned int width, height;
	map = loadTexture(fileName, width, height);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc() {

	//Limpiar escena cada iteración del bucle de renderizado
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Se habilita el programa creado para enlazar el vert shader y el fragment shader
	glUseProgram(program);

	glViewport(0, 0, w / 2, h);

	///////////////////////////////////////////////	BOX 1 //////////////////////////////////////////////
	//Se crean las matrices modelView, modelViewProjection y normal
	glm::mat4 modelView = view * model_box1;
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModel != -1)
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &(model_box1[0][0]));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	///////////////////////////////////////////////	BOX 2 //////////////////////////////////////////////
	//Se crean las matrices modelView, modelViewProjection y normal
	modelView = view * model_box2;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModel != -1)
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &(model_box2[0][0]));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
		///////////////////////////////////////////////	BOX 3 //////////////////////////////////////////////
	//Se crean las matrices modelView, modelViewProjection y normal
	modelView = view * model_box3;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModel != -1)
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &(model_box3[0][0]));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	///////////////////////////////////////////////	PYRAMID //////////////////////////////////////////////
	/*//Se crean las matrices modelView, modelViewProjection y normal
	modelView = view * model_pyramid;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModel != -1)
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &(model_pyramid[0][0]));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, pyramidNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////
	

	glViewport(w / 2, 0, w / 2, h);  //Cada vez que se llama a renderizar se ajusta el viewport

	glm::mat4 v = view;
	v[3].x += 0.1f;
	//Se crean las matrices modelView, modelViewProjection y normal
	modelView = v * model_box1;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	if (uLightPosition != -1)
		glUniform3fv(uLightPosition, 1, &(lightPosition[0]));
	if (uLightIntensity != -1)
		glUniform3fv(uLightIntensity, 1, &(lightIntensity[0]));


	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}



	glutSwapBuffers();

}

void resizeFunc(int width, int height) {

	float a = float(width) / float(height);
	w = width; h = height;

	float n = 1.0f;
	float f = 60.0f;
	float x = 1.0f / (glm::tan(30.0f * 3.1419f / 180.0f));

	proj[0].x = x * 1.0 / a;
	proj[1].y = x;
	proj[2].z = (f + n) / (n - f);
	proj[2].w = -1.0f;
	proj[3].z = 2.0f * n * f / (n - f);
	
	glutPostRedisplay();

}
	

void idleFunc() {

	model_box1 = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.001f;
	model_box1 = glm::rotate(model_box1, angle, glm::vec3(1.0f, 1.0f, 0.0f));

	model_box2 = glm::mat4(1.0f);
	glm::mat4 model_box2_translate = glm::translate(model_box2, glm::vec3(4.0f, 0.0f, 0.0f));
	glm::mat4 model_box2_rotate = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	model_box2 = model_box2_rotate * model_box2_translate * model_box2_rotate;
	
	glm::vec3 punto_bezier_curva = glm::vec3(0.0f);
	if ((0.0f <= bezier_t) && (bezier_t <= 1.0f)) {  //Cuadrante I: [p0 = 0, p2 = pi/2]
		glm::vec3 p0 = glm::vec3(8.0f, 0.0f, 0.0f);
		glm::vec3 p1 = glm::vec3(9.0f, 0.0f, 10.0f);
		glm::vec3 p2 = glm::vec3(0.0f, 0.0f, 8.0f);
		punto_bezier_curva = calcularPuntoBezier(p0, p1, p2, bezier_t);
		bezier_t += 0.001f;
	}
	else if ((1.0f <= bezier_t) && (bezier_t <= 2.0f)) {  //Cuadrante II [p0 = pi/2, p2 = pi] restamos 1 a t
		glm::vec3 p0 = glm::vec3(0.0f, 0.0f, 8.0f);
		glm::vec3 p1 = glm::vec3(-11.0, 0.0f, 10.0f);
		glm::vec3 p2 = glm::vec3(-8.0f, 0.0f, 0.0f);
		float bezier_T_cuadrante2 = (bezier_t - 1);
		punto_bezier_curva = calcularPuntoBezier(p0, p1, p2, bezier_T_cuadrante2);
		bezier_t += 0.001f;
	}
	else if ((2.0f <= bezier_t) && (bezier_t <= 3.0f)) {  //Cuadrante III [p0 = pi, p2  = 3pi/2] restamos 2 a t
		glm::vec3 p0 = glm::vec3(-8.0f, 0.0f, 0.0f);
		glm::vec3 p1 = glm::vec3(-15.0f, 0.0f, -12.5f);
		glm::vec3 p2 = glm::vec3(0.0f, 0.0f, -8.0f);
		float bezier_T_cuadrante3 = (bezier_t - 2);
		punto_bezier_curva = calcularPuntoBezier(p0, p1, p2, bezier_T_cuadrante3);
		bezier_t += 0.001f;
	}
	else if ((3.0f <= bezier_t) && (bezier_t <= 4.0f)) {  //Cuadrante IV [p0 = 3pi/2, p2 = 2pi] restamos 3 a t
		glm::vec3 p0 = glm::vec3(0.0f, 0.0f, -8.0f);
		glm::vec3 p1 = glm::vec3(13.0f, 0.0f, -8.3f);
		glm::vec3 p2 = glm::vec3(8.0f, 0.0f, 0.0f);
		float bezier_T_cuadrante4 = (bezier_t - 3);
		punto_bezier_curva = calcularPuntoBezier(p0, p1, p2, bezier_T_cuadrante4);
		bezier_t += 0.001f;
	}
	if (bezier_t >= 4.0f) bezier_t = 0.0f;  //Reseteo: t pertenece al intervalo[0, 1]

	model_box3 = glm::mat4(1.0f);
	glm::mat4 model_box3_translate = glm::translate(model_box3, punto_bezier_curva);
	model_box3 = model_box3_translate;

	glutPostRedisplay();

}

glm::vec3 calcularPuntoBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float t)
{
	float coeficiente_punto0 = pow((1 - t), 2);
	float coeficiente_punto1 = 2 * t * (1 - t);
	float coeficiente_punto2 = pow(t, 2);
	return (coeficiente_punto0 * p0) + (coeficiente_punto1 * p1) + (coeficiente_punto2 * p2);
}

void keyboardFunc(unsigned char key, int x, int y) {

	const float SPEED = 1.0f;
	const float ALPHA = 5.0f;

	//i, j, k ,l --> posicion de la luz y/Y--> intensidad de la luz
	switch (key) {

		///////////////////////////////////////	Modificaciones en la luz
	case 'i': lightPosition.z += 1.0f;
		break;

	case 'j': lightPosition.x += 1.0f;
		break;

	case 'k': lightPosition.z -= 1.0f;
		break;

	case 'l': lightPosition.x -= 1.0f;
		break;

	case 'Y':
		if (lightIntensity[0] < 1.25) {
			lightIntensity = glm::vec3(lightIntensity[0] + 0.1f);
		}
		break;

	case 'y':
		if (lightIntensity[0] > 0.0) {
			lightIntensity = glm::vec3(lightIntensity[0] - 0.1f);
		}
		break;

		///////////////////////////////////////	Modificaciones en la matriz view (cámara)
	case 'w':  //Alante
		cameraX += SPEED * glm::sin(glm::radians(-cameraAlphaY));
		cameraZ += SPEED * glm::cos(glm::radians(-cameraAlphaY));
		break;

	case 'W':
		cameraX += SPEED * glm::sin(glm::radians(-cameraAlphaY));
		cameraZ += SPEED * glm::cos(glm::radians(-cameraAlphaY));
		break;

	case 's':  //Atrás
		cameraX -= SPEED * glm::sin(glm::radians(-cameraAlphaY));
		cameraZ -= SPEED * glm::cos(glm::radians(-cameraAlphaY));
		break;

	case 'S':
		cameraX -= SPEED * glm::sin(glm::radians(-cameraAlphaY));
		cameraZ -= SPEED * glm::cos(glm::radians(-cameraAlphaY));
		break;

	case 'a':  //Izq
		cameraX += SPEED * glm::cos(glm::radians(cameraAlphaY));
		cameraZ += SPEED * glm::sin(glm::radians(cameraAlphaY));
		break;

	case 'A':
		cameraX += SPEED * glm::cos(glm::radians(cameraAlphaY));
		cameraZ += SPEED * glm::sin(glm::radians(cameraAlphaY));
		break;
	case 'd':  //Der
		cameraX -= SPEED * glm::cos(glm::radians(cameraAlphaY));
		cameraZ -= SPEED * glm::sin(glm::radians(cameraAlphaY));
		break;

	case 'D':
		cameraX -= SPEED * glm::cos(glm::radians(cameraAlphaY));
		cameraZ -= SPEED * glm::sin(glm::radians(cameraAlphaY));
		break;

	case 'q':  //RotIzq
		cameraAlphaY -= ALPHA;
		break;

	case 'Q':
		cameraAlphaY -= ALPHA;
		break;

	case 'e':  //RotDer
		cameraAlphaY += ALPHA;
		break;

	case 'E':
		cameraAlphaY += ALPHA;
		break;
	}

	glm::mat4 camera_movement = glm::mat4(1.0f);
	//Inicializar estado actual de cámara (traslación)
	camera_movement[3].x = cameraX;
	camera_movement[3].z = cameraZ;
	//Rotación
	glm::mat4 center_camera = glm::translate(camera_movement, glm::vec3(-cameraX, 0.0f, -cameraZ));  // Matriz para trasladar al centro
	glm::mat4 rotate_camera = glm::rotate(center_camera, glm::radians(cameraAlphaY), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::translate(rotate_camera, glm::vec3(cameraX, 0.0f, cameraZ));

	lookAt.x = cameraX;
	lookAt.z = cameraZ;
	right.x = cameraX;
	right.z = cameraZ;
	lookAt = glm::vec3(lookAt.x + ORBIT_RADIUS * glm::sin(glm::radians(-cameraAlphaY)), 0.0f, lookAt.z + ORBIT_RADIUS * glm::cos(glm::radians(cameraAlphaY)));
	right = glm::vec3(right.x + ORBIT_RADIUS * -glm::cos(glm::radians(-cameraAlphaY)), 0.0f, right.z + ORBIT_RADIUS * -glm::cos(glm::radians(cameraAlphaY)));
	std::cout << "Lookat x: " << lookAt.x << " - lookAt z: " << lookAt.z << std::endl;
	std::cout << "Right x: " << right.x << " - Right z: " << right.z << std::endl;
	std::cout << "posicion de la luz puntual= " << lightPosition.x << "		" << lightPosition.y << "	" << lightPosition.z << std::endl;
}

void mouseFunc(int button, int state, int x, int y) {

	glm::mat4 camera_orbit = glm::mat4(1.0f);
	//Inicializar estado actual de cámara (traslación)
	camera_orbit[3].x = cameraX;
	camera_orbit[3].z = cameraZ;
	//Rotación
	glm::mat4 center_camera = glm::translate(camera_orbit, glm::vec3(-cameraX, 0.0f, -cameraZ));  // Matriz para trasladar al centro
	glm::mat4 rotate_camera = glm::rotate(center_camera, glm::radians(cameraAlphaY), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 actual_camera_state = glm::translate(rotate_camera, glm::vec3(cameraX, 0.0f, cameraZ));

	float movX = 0.0f - lookAt.x;
	float movZ = 0.0f - lookAt.z;
	//Calcular angle con respecto al cambio en la x del mouse click en el viewport
	float angleX = x;
	float angleZ = y;

	glm::vec3 normalized_lookAt = glm::normalize(lookAt);
	glm::vec3 normalized_right = glm::normalize(right);

	/*std::cout << "Cross product X: " << glm::cross(normalized_lookAt, normalized_right).x << " Cross product y: " <<
		glm::cross(normalized_lookAt, normalized_right).y <<
		" Cross product z: " << glm::cross(normalized_lookAt, normalized_right).z << std::endl;*/

	glm::mat4 translate_to_rot_center = glm::translate(actual_camera_state, glm::vec3(movX, 0.0f, movZ));
	glm::mat4 rotation_from_rot_centerX = glm::rotate(translate_to_rot_center, glm::radians(angleX), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation_from_rot_centerZ = glm::rotate(rotation_from_rot_centerX, glm::radians(angleZ), glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::mat4 rotation_from_rot_centerZ = glm::rotate(rotation_from_rot_centerX, glm::radians(angleZ), glm::cross(normalized_lookAt, normalized_right));
	glm::mat4 final_view = glm::translate(rotation_from_rot_centerZ, glm::vec3(-movX, 0.0f, -movZ));

	view = final_view;

	glutPostRedisplay();

}









