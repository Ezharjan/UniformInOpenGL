#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lesson of error handling ability in OpenGL.

//Optimization
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))
	
/////////////////////////////////////////////////////
static void GLClearError(){
	while (glGetError() != GL_NO_ERROR){	}; // Just clear the error history while there is no error.

}

//// Before seting an Assert() function and witout optimization.
//static void GLCheckError(){
//	while (GLenum error = glGetError()) // Log error info  while there is the errors.
//	{
//		cout << "[OpenGL Error] (" << error << ")" << endl;
//	}
//}

//// 1st and 2nd stage of Optimization through the C++ Assert() method
//static bool GLLogCall(){
//	while (GLenum error = glGetError()) // Log error info  while there is the errors.
//	{
//		cout << "[OpenGL Error] (" << error << ")" << endl;
//		return false;
//	}
//	return true;
//}

//// 3rd stage of Optimization
static bool GLLogCall(const char* function, const char* file, int line){
	while (GLenum error = glGetError()) // Log error info  while there is the errors.
	{
		cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << endl;
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ShaderProgramSource
{
	string VertexSource;
	string FragmentSource;
};

static ShaderProgramSource ParseShader(const string filePath){
	
	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	ifstream stream(filePath);
	ShaderType type = ShaderType::NONE;
	string line;
	stringstream ss[2]; // One for vertex shader while the other is for the fragment shader
	while (getline(stream, line))
	{
		if (line.find("#shader") != string :: npos )
		{
			if (line.find("vertex") != string::npos)
			{
				//set vertex mode
				type = ShaderType::VERTEX;
			}
			else{
				//set fragment mode
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n'; // Add line to the stream string
		}
	}
	return {ss[0].str(), ss[1].str()};
}

static int CompilerShader(unsigned int type, const string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str(); 
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) 
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length *sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex " : "fragment ") << "shader" <<   endl;
		cout << message << endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader) { 
	unsigned int programe = glCreateProgram();  // "unsigned int" equals to "GLuint"
	unsigned int vShder = CompilerShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fShder = CompilerShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(programe, vShder);
	glAttachShader(programe, fShder);
	glLinkProgram(programe);
	glValidateProgram(programe);

	glDeleteShader(vShder); 
	glDeleteShader(fShder);

	return programe;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error(Don't worry for this is just a test!)" << std::endl; 
	}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(480, 480, "Shaders in OpenGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // Set the interval time among each rendering time

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error Again!!!" << std::endl;  // Will not an Error printing 
	}
	std::cout << "Your current GL version is : " << glGetString(GL_VERSION) << std::endl;

	float position[] = {
		-0.5f,-0.5f,//0
		0.5f, -0.5f,//1
		0.5f,  0.5f,//2
	   -0.5f,  0.5f//3
	};

	unsigned int indecies[] = {
		0,1,2,
		0,2,3	
	}; 

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), position, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indecies, GL_STATIC_DRAW);

	ShaderProgramSource source = ParseShader("Basics.shader");

	cout << "Vertex Shader Code Below" << endl;
	cout << source.VertexSource << endl;
	cout << "Fragment Shader Code Below" << endl;
	cout << source.FragmentSource << endl;

	unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
	glUseProgram(shader);

	int location = glGetUniformLocation(shader, "u_Color");
	ASSERT(location != -1); // Assert the location in case it was ruined or fragiled！*********Very Important*********
	
	//glUniform4f(location, 0.9f, 0.3f, 0.5f, 1.0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	float red = 0.0f;
	//float green = 0.0f;
	//float blue = 0.0f;
	float increment = 0.05f;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform4f(location, red, 0.9f, 0.0f, 1.0f); // Only the Grayscale will change when RGB is equally distributed

		/*Draw here*/
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (red > 1.0f)
		{
			increment = -0.05f; // Make the increment go back to initial state
		}
		else if (red < 0.0f)
		{
			 increment = 0.05f; // Make the increment go back to initial state
		}
		red += increment; // Add the increment in each loop
		//green += increment + 0.25f;
		 //Only the grayscale will change when RGB is equally distributed, so the increment shouldn't be the same.
		 //Only the lightness will change when the increment is the same, so the increment should change in random type. 
		//blue += increment - 0.35f;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glDeleteProgram(shader);
	glfwTerminate();
	return 0;
}