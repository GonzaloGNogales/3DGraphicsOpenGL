#ifndef __PYRAMIDILE__
#define __PYRAMIDILE__

//Describimos una pirámide
//*******************
//Se replican vértices para que cada cara 
//tenga una normal distinta.

//Número de vértices
const int pyramidNVertex = 12; // 3 vértices * 4 caras
const int pyramidNTriangleIndex = 4; // 1 triángulo por cara x  4 cara;


const unsigned int pyramidTriangleIndex[] = {
	
	//Cara z = 1
	0,1,2,
	
	//Cara x = 1
	3,4,5,

	//Cara z = -1
	6,7,8,
	
	//Cara y = -1
	9,10,11,


};

//Posicíon de los vertices
const float pyramidVertexPos[] = { 

	//Cara z = 1
	-1.0f,	-1.0f,	  1.0f, //0 --> B
	 1.0f,	-1.0f,	  1.0f, //1 --> C
	 0.0f,	 1.0f,	  0.0f, //2 --> A

	//Cara x = 1
	 1.0f,	-1.0f,	  1.0f, //3 --> C
	 1.0f,	-1.0f,	 -1.0f, //4 --> D
	 0.0f,	 1.0f,	  0.0f, //5 --> A

	//Cara z = -1
	1.0f,	-1.0f,	 -1.0f, //6 --> D
	-1.0f,	-1.0f,	  1.0f, //7 --> B
	 0.0f,	 1.0f,	  0.0f, //8 --> A

	//Cara y = -1
	-1.0f,	-1.0f,	  1.0f, //9 --> B
	 1.0f,	-1.0f,	 -1.0f, //11 --> D
	1.0f,	-1.0f,	  1.0f, //10 --> C


 };


//Normal de los vertices
const float pyramidVertexNormal[] = { 

	//Cara z = 1
	0.0f,	0.0f,	1.0f, //0 --> B
	0.0f,	0.0f,	1.0f, //1 --> C
	0.0f,	0.0f,	1.0f, //2 --> A

	//Cara x = 1
	1.0f,	0.0f,	0.0f, //3 --> C
	1.0f,	0.0f,	0.0f, //4 --> D
	1.0f,	0.0f,	0.0f, //5 --> A
	
	//Cara z = -1
	-0.5f,	0.0f,	-0.5f, //9 --> B
	-0.5f,	0.0f,	-0.5f, //10 --> C
	-0.5f,	0.0f,	-0.5f, //11 --> D

	//Cara y = -1
	0.0f,	-1.0f,	0.0f, //6 --> D
	0.0f,	-1.0f,	0.0f, //7 --> B
	0.0f,	-1.0f,	0.0f, //8 --> A

 };


//Color de los vertices
const float pyramidVertexColor[] = { 
	//Cara z = 1
	0.5f,	0.0f,	0.5f, //0 --> B
	0.5f,	0.0f,	0.5f, //1 --> C
	0.5f,	0.0f,	0.5f, //2 --> A

	//Cara x = 1
	1.0f,	0.0f,	0.0f, //3 --> C
	1.0f,	0.0f,	0.0f, //4 --> D
	1.0f,	0.0f,	0.0f, //5 --> A

	//Cara z = -1
	0.5f,	0.3f,	0.2f, //9 --> B
	0.5f,	0.3f,	0.2f, //10 --> C
	0.5f,	0.3f,	0.2f, //11 --> D
						  
	//Cara y = -1
	0.0f,	1.0f,	0.0f, //6 --> D
	0.0f,	1.0f,	0.0f, //7 --> B
	0.0f,	1.0f,	0.0f, //8 --> A

};

//Cood. de textura de los vertices
const float pyramidVertexTexCoord[] = { 
	//Cara z = 1
	 0.0f, 0.0f,
	 1.0f, 0.0f,
	 0.0f, 1.0f,

	//Cara x = 1	
	0.0f,	1.0f,
	1.0f,	1.0f,
	0.0f,	0.0f,

	//Cara z = -1
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,

	//Cara y = -1
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,

 };


const float pyramidVertexTangent[] = { 
	//Cara z = 1
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
		

	//Cara x = 1				
	0.0f,	0.0f,	-1.0f,		
	0.0f,	0.0f,	-1.0f,		
	0.0f,	0.0f,	-1.0f,		

	//Cara z = -1				
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
					
	//Cara y = -1					   
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,

 };

#endif


