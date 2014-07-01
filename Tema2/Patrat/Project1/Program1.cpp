#include "stb_image.h"

#include "Sprite.h"


string LoadFileToString(const char* filepath){
	string fileData;
	ifstream stream(filepath, ios::in);

	if (stream.is_open()){
		string line = "";
		while (getline(stream, line)){
			fileData += '\n' + line;
		}
		stream.close();
	}

	return fileData;
}
GLuint compileShaders(char* pixelShaders, char* vertexShaders){

	string vertexSourceStr = LoadFileToString(vertexShaders);
	string fragmentSourceStr = LoadFileToString(pixelShaders);
	const GLchar* vertexSource = vertexSourceStr.c_str();
	const GLchar* fragmentSource = fragmentSourceStr.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	return shaderProgram;

}
void FlipTexture(unsigned char* image_data, int x, int y, int n)
{
	//flip texture
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
}

int main() {

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);

	if (!window){
		fprintf(stderr, "Window couldn't be created!\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	//Initialize GLEW
	if (glewInit() != GLEW_OK){
		fprintf(stderr, "Glew is gone, man!\n");
		glfwTerminate();
		return -1;
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	float vertices[] = {
		//Position	TexCoords
		-0.5f, 0.5f,0.0f, 0.0f, 0.0f, // Top-left
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // Top-right
		0.5f, -0.5f, 0.0f, 1.0f, 1.0f,// Bottom-right
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f  // Bottom-left




	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	vector<GLuint> elements = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,elements.size()*sizeof(GLuint), &elements[0], GL_STATIC_DRAW);

	GLuint shaderProgram = compileShaders("pixelShader.glsl", "vertexShader.glsl");

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	GLuint textures;
	glGenTextures(1, &textures);

	int x, y,n;

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures);
	int force_channels = 4;
	unsigned char* image = stbi_load("bug.png", &x, &y, &n, force_channels);

	FlipTexture(image, x, y, n);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	while (!glfwWindowShouldClose(window)){

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, 0);
	}

	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

}