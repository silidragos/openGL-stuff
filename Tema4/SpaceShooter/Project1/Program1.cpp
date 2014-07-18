#include "SpriteManager.h"

#include "Entity.h"

#include "PlayerPhysics.h"
#include "EnemyPhysics.h"
#include "ProjectilePhysics.h"

#include"XML.h"
#include"Anim.h"
#include"AnimManager.h"
#include"CameraTransform.h"

#include<iostream>
#include<fstream>
#include"glm\gtc\matrix_transform.hpp"
#include"glm\gtc\type_ptr.hpp"
#include<time.h>


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

	GLint Result = GL_FALSE;
	int InfoLogLength;

	printf("Compiling shader : %s\n", vertexShaders);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Check Vertex Shader
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	printf("Compiling shader : %s\n", pixelShaders);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Check Fragment Shader
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(fragmentShader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	printf("Linking program\n");
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Check the program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Result);
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage(InfoLogLength+1);
	glGetProgramInfoLog(shaderProgram, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	return shaderProgram;

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
		fprintf(stderr, "Glew is gone!\n");
		glfwTerminate();
		return -1;
	}

	//Enable alpha channel
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glEnable(GL_BLEND);  
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	vector<float> vertices;
	

	GLuint ebo[NMAX];
	vector<vector<GLuint>> elements;

	//Test -- Create new sprites
	SpriteManager* spriteMan = new SpriteManager();
	AnimManager* animManager = new AnimManager();

	vector<Entity*> EnEnt;
	vector<Sprite*> spr2;
	spr2.resize(5);
	EnEnt.resize(5);
	
	//Add background
	Sprite* BG = new Sprite(-100.0f, 100.0f, -100.0f, 100.0f, vertices, elements);

	Entity* BGEntity = new Entity(BG, NULL);
	spriteMan->addSprite(BG);
	spriteMan->addEntity(BGEntity);

	//Creates Enemy ships Sprites
	for (int i = 0; i < 5; ++i)
	{
		spr2[i] = new Sprite(-1.0f+i*0.2f, -1.0f+(i + 1)*0.2f, 0.6, 0.8f, vertices, elements);
		spriteMan->addSprite(spr2[i]);
	}

	//EndTest

	GLuint shaderProgram = compileShaders("pixelShader.glsl", "vertexShader.glsl");

	spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);

	//Translations
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	GLint uniTrans = glGetUniformLocation(shaderProgram, "trans");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

	// Set up projection
		glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
		);
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	
	
	//glm::mat4 proj = glm::perspective(45.0f, 800.0f / 600.0f, 0.0f, 100.0f);
	glm::mat4 proj = glm::ortho(-1.0f,1.0f,-1.0f,1.0f,0.0f,100.0f);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	//Enemy Entities
	srand(time(NULL));

	for (int i = 0; i < 5; ++i){
		EnEnt[i] = new Entity(spr2[i], new EnemyPhysics(uniTrans, rand() % 2));
		spriteMan->addEntity(EnEnt[i]);

		EnEnt[i]->physics->setAABB(EnEnt[i]->sprite->getLowX() + 0.04f, EnEnt[i]->sprite->getHighX() - 0.04f, EnEnt[i]->sprite->getLowY() + 0.06f, EnEnt[i]->sprite->getHighY() - 0.06f);
	}

	//Add player
	Sprite* player = new Sprite(-0.1f, 0.1f, -1.0f, -0.8f, vertices, elements);
	spriteMan->addSprite(player);
	Entity* playerEnt = new Entity(player, new PlayerPhysics(uniTrans,animManager));
	spriteMan->addEntity(playerEnt);
	playerEnt->physics->setAABB(playerEnt->sprite->getLowX(), playerEnt->sprite->getHighX(), playerEnt->sprite->getLowY(), playerEnt->sprite->getHighY());

	

	//Add textures
	TextureManager* t = new TextureManager();
	GLuint enTex = t->addTexture("bug.png",'c');
	GLuint blastTex = t->addTexture("blast.png",'c');
	GLuint bgTex = t->addTexture("stars.png",'r');
	//Texturile trebuie afisate in ordinea in care sunt in elements!!!

	for (int i = 0; i < 5; ++i){
		spr2[i]->addTexture(enTex);
	}

	player->addTexture(t->getTexture("player.png"));

	BG->addTexture(t->getTexture("stars.png"));

	BG->setUV(vertices, 0.0f, 100.0f, 0.0f, 100.0f);
	BG->setInBackground(vertices);

	
	spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

	//Loading Animations
	animManager->setTimePerFrame(0.2f);

	vector<Dict_Entry*> loadedSprites;
	getXml(t,loadedSprites);
	
	Animatie* anim = new Animatie();
	anim->setSprite(player);
	for (int i = 0; i < 13; i++)
		anim->addDictEntry(loadedSprites[i]);

	Animatie* animLeft = new Animatie();
	animLeft->setSprite(player);
	for (int i = 16; i < 18; ++i){
		animLeft->addDictEntry(loadedSprites[i]);
	}
	animLeft->setLoop(false);

	Animatie* animRight = new Animatie();
	animRight->setSprite(player);
	for (int i = 18; i < 20; ++i){
		animRight->addDictEntry(loadedSprites[i]);
	}
	animRight->setLoop(false);

	spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);

	animManager->addAnim("Idle", anim);
	animManager->startAnim("Idle");

	animManager->addAnim("Left", animLeft);
	animManager->addAnim("Right", animRight);


	//Variables used in while
	vector<Sprite*> blast;
	vector<Entity*> projEnt;

	GLfloat lastShoot = 0.0f;
	float lastAnim = 0.0f;
	float lastFrame = 0.0f;

	//Check How much time a Blast should last
	vector<float> blastLifeTime;
	float blastLife = 1.3f;
	float dt;

	float cameraPosX = 0.0f;
	float cameraPosY = 0.0f;
	float lastPosX=0.0f, lastPosY=0.0f;
	bool shouldMove = false;

	while (!glfwWindowShouldClose(window) && !glfwGetKey(window,GLFW_KEY_ESCAPE)){

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		GLfloat time = (GLfloat)clock() / (GLfloat)CLOCKS_PER_SEC;


		//Create Projectile
		if (glfwGetKey(window, GLFW_KEY_SPACE) && time-lastShoot>0.2f){
			lastShoot = time;
			Sprite* sBlast = new Sprite(playerEnt->physics->getPozX(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY())-0.2f, 
				playerEnt->physics->getPozX(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY()),
				playerEnt->physics->getPozY(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY()) -0.1f,
				playerEnt->physics->getPozY(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY()) + 0.1f, 
				vertices, elements);
			sBlast->addTexture(blastTex);
			spriteMan->addSprite(sBlast);
			blast.push_back(sBlast);
			blastLifeTime.push_back(0.0f);


			Entity* sProjEnt = new Entity(sBlast, new ProjectilePhysics(uniTrans));
			projEnt.push_back(sProjEnt);
			spriteMan->addEntity(sProjEnt);
			
			sProjEnt->physics->setAABB(sProjEnt->sprite->getLowX() + 0.06f, sProjEnt->sprite->getHighX() - 0.06f, sProjEnt->sprite->getLowY() + 0.06f, sProjEnt->sprite->getHighY() - 0.06f);

			spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);
			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
		}
		
		//Check collisions - Blast - Enemy
		for (int j = 0; j < projEnt.size();j++)
			for (int i = 0; i < spr2.size(); ++i)
			if (projEnt[j]->physics->AABBvsAABB(EnEnt[i]->physics->getAABB())){
				spriteMan->removeSprite(spr2[i], vertices, elements);
				spr2.erase(spr2.begin() + i);
				EnEnt.erase(EnEnt.begin() + i);

				spriteMan->removeSprite(projEnt[j]->sprite, vertices, elements);
				projEnt.erase(projEnt.begin() + j);
				blastLifeTime.erase(blastLifeTime.begin() + j);

				spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);
				glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
				break;
			}

		

		animManager->playActiveAnim(time, vertices);
		spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);

		dt = time - lastFrame;
		lastFrame = time;

		bool blastDestroyed = false;
		for (int i = 0; i < blastLifeTime.size(); i++){
			blastLifeTime[i] += dt;
			if (blastLifeTime[i] >= blastLife){
				spriteMan->removeSprite(projEnt[i]->sprite, vertices, elements);
				projEnt.erase(projEnt.begin() + i);
				blastLifeTime.erase(blastLifeTime.begin() + i);
				blastDestroyed = true;
			}
		}
		if (blastDestroyed){
			spriteMan->reGenBuffers(vbo, ebo, elements, vertices, shaderProgram);
			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
		}


		//Camera Projection/View

	//LERP(should use constant timing)		
		float pozX = playerEnt->physics->getPozX(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY());
		float pozY = playerEnt->physics->getPozY(playerEnt->sprite->getHighX(), playerEnt->sprite->getHighY());
		pozY += 0.75f;
		if ((abs(pozX - lastPosX) > 0.3f || abs(pozY - lastPosY) > 0.3f)||shouldMove){
			shouldMove = moveCameraTo(cameraPosX, cameraPosY, pozX, pozY);
			lastPosX = pozX;
			lastPosY = pozY;
		}

		glm::mat4 view = glm::lookAt(
			glm::vec3(cameraPosX,cameraPosY, 2.0f),
			glm::vec3(cameraPosX, cameraPosY, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
			);
		
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


		//Draw
		spriteMan->drawAll(elements, ebo,shaderProgram,window,vertices,uniTrans,dt);
	}

	//Free memory
	delete playerEnt;
	for (int i = 0; i < EnEnt.size(); ++i) delete EnEnt[i];
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(elements.size(), ebo);
	glDeleteVertexArrays(1, &vao);

}