#include "BOX.h"
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

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);


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
glm::vec3 lightPosition;
unsigned int uLightIntensity;
glm::vec3 lightIntensity;	// == difusa

//VAO punto medio entre atributos y datos del VBO
unsigned int vao; 

//VBOs que forman parte del objeto 
unsigned int posVBO; 
unsigned int colorVBO; 
unsigned int normalVBO; 
unsigned int texCoordVBO; 
unsigned int triangleIndexVBO;

//Ancho y alto de la ventana
int w, h;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
unsigned int loadTex(const char *fileName);


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
	view[3].z = -6;

	lightPosition = glm::vec3(0.0f);
	lightIntensity = glm::vec3(1.0);

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
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

}

void initShader(const char *vname, const char *fname) {

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
		char *logString = new char[logLen]; 
		glGetProgramInfoLog(program, logLen, NULL, logString); 
		std::cout << "Error: " << logString << std::endl; 
		delete[] logString;
		glDeleteProgram(program);
		program = 0; 
		exit (-1); 
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
	uLightIntensity= glGetUniformLocation(program, "Id");



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
	model = glm::mat4(1.0f);

	//Creación de texturas
	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");

}

GLuint loadShader(const char *fileName, GLenum type) { 
	
	unsigned int fileLen; 
	char* source = loadStringFromFile(fileName, fileLen); 

	////////////////////////////////////////////// 
	//Creación y compilación del Shader 
	GLuint shader; 
	shader = glCreateShader(type); 
	glShaderSource(shader, 1, 
		(const GLchar **)&source, (const GLint *)&fileLen); 
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

unsigned int loadTex(const char *fileName){ 

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

	glViewport(0, 0, w/2, h);

	//Se crean las matrices modelView, modelViewProjection y normal
	glm::mat4 modelView = view * model; 
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView)); 

	//Si se usan y se encuentran en los shaders las variables uniformes (sus idx son distintos a -1) se suben al cauce
	if (uModel != -1)
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &(model[0][0]));
	if (uModelViewMat != -1) 
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0])); 
	if (uModelViewProjMat != -1) 
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0])); 
	if (uNormalMat != -1) 
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao); 
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	glViewport(w / 2, 0, w / 2, h);  //Cada vez que se llama a renderizar se ajusta el viewport

	glm::mat4 v = view;
	v[3].x += 0.1f;
	//Se crean las matrices modelView, modelViewProjection y normal
	modelView = v * model;
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

	w = width;
	h = height;
	
	glutPostRedisplay();

}

void idleFunc() {

	model = glm::mat4(1.0f); 
	static float angle = 0.0f; 
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.001f; 
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

	glutPostRedisplay();

}

void keyboardFunc(unsigned char key, int x, int y){

	//i, j, k ,l --> posicion de la luz y/Y--> intensidad de la luz
	switch (key) {

	case 'i': lightPosition.z += 1.0f;
		break;

	case 'j': lightPosition.x += 1.0f;
		break;

	case 'k': lightPosition.z -= 1.0f;
		break;

	case 'l': lightPosition.x -= 1.0f;
		break;

	case 'y': 
		if (lightIntensity[0] < 1.25) {
			lightIntensity = glm::vec3(lightIntensity[0] - 0.1f);
		}
		break;

	case 'Y':
		if (lightIntensity[0] > 0.0 ) {
			lightIntensity = glm::vec3(lightIntensity[0] + 0.1f);
		}
		break;

	}

	std::cout << "posicion de la luz puntual= "<< lightPosition.x <<"		"<< lightPosition.y <<"	" << lightPosition.z << std::endl;
}
void mouseFunc(int button, int state, int x, int y){}









