#include"shaderClass.h"

// Reads a text file and outputs a string with everything in the text file
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	// Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// Convert the shader source strings into character arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();


	//crear el objeto sombreador de vertices de referencia
	GLuint /*version opengl de entero sin signo*/ vertexShader = glCreateShader(GL_VERTEX_SHADER/*Tipo de shader*/); //referencia para el objeto

	//enlaza la fuente del sombreador de vertices al objeto de sombreador de vertices
	glShaderSource(vertexShader, 1, &vertexSource, NULL);

	//compila el sombreador de vertices en el codigo 
	glCompileShader(vertexShader);
	// todo es el shader de vertices

	//crear sombreador de fragmentos de referencia
	GLuint  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //referencia para el objeto

	//enlaza la fuente del sombreador de fragmentos al objeto de sombreador de fragmentos
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

	//compila el sombreador de vertices en el codigo 
	glCompileShader(fragmentShader);
	// todo es el shader de fragmentos

	//crear el objeto de programa de sombreado de referencia (donde se encapsularan ambos shaders)
	ID = glCreateProgram();

	//enlaza los shader de vertices y fragmentos al programa de shaders
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	//enlaza todos los shaders juntos en el programa de shaders
	glLinkProgram(ID);

	//elimina los shaders de vertices y fragmentos sin usar
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader); //preguntar al profe porque aqui se eliminan los shaders creados
}

// Activates the Shader Program
void Shader::Activate()
{
	glUseProgram(ID);
}

// Deletes the Shader Program
void Shader::Delete()
{
	glDeleteProgram(ID);
}