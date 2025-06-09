#include "IM472.h"
#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Parâmetros da malha
const int N = 50; // número de divisões em x [0,2]
const int M = 25; // número de divisões em y [0,1]
const float h = 2.0f / float(N); // número de passos em x
const float k = 1.0f / float(M); // número de passos em y

// Variáveis globais
float *solution = nullptr;

// rotação
float rotationX = 20.0f;
float rotationY = 180.0f;

// visão e escala
float zoom = 10.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = .2f;

// visualização do wireframe
bool wireframe = false;

// Função para converter índices da malha (i,j) para coordenadas (x,y)
void getCoordinates(int i, int j, float &x, float &y) {
	x = (float)i * h;
	y = (float)j * k;
}

// Função para obter o valor da solução no ponto (i,j)
float getSolutionValue(int i, int j) {
	// Pontos interiores da malha
	if (i > 0 && i < N && j > 0 && j < M) {
		return solution[(i - 1) + (j - 1) * (N - 1)];
	}

	float x, y;
	getCoordinates(i, j, x, y);

	// Condições de contorno
	if (i == 0)
		return 0.0f; // u(0,y) = 0
	if (i == N)
		return 2.0f * expf(y); // u(2,y) = 2e^y
	if (j == 0)
		return x; // u(x,0) = x
	if (j == M)
		return expf(x); // u(x,1) = e^x

	return 0.0f; // Nunca deve chegar aqui
}

// Calcula o valor máximo da solução para normalização
float getMaxSolutionValue() {
	float maxVal = 0.0f;
	// Verificar pontos interiores
	for (int idx = 0; idx < (N - 1) * (M - 1); idx++) {
		if (fabs(solution[idx]) > maxVal)
			maxVal = fabs(solution[idx]);
	}

	// Verificar condições de contorno
	float x, y;
	for (int i = 0; i <= N; i++) {
		for (int j = 0; j <= M; j++) {
			getCoordinates(i, j, x, y);
			if (i == N) { // u(2,y) = 2e^y
				float val = 2.0f * expf(y);
				maxVal = std::max(maxVal, val);
			}
			if (j == M) { // u(x,1) = e^x
				float val = expf(x);
				maxVal = std::max(maxVal, val);
			}
		}
	}

	return maxVal;
}

// Atribui uma cor conforme o valor da função
void setColorByValue(float value, float maxValue) {
	// Normaliza o valor entre 0 e 1
	float normalizedValue = 0.5f * (value / maxValue + 1.0f);

	// Mapa de cores HSV-like (do azul ao vermelho)
	if (normalizedValue < 0.25f) {
		// Azul para ciano
		glColor3f(0.0f, 4.0f * normalizedValue, 1.0f);
	} else if (normalizedValue < 0.5f) {
		// Ciano para verde
		glColor3f(0.0f, 1.0f, 1.0f - 4.0f * (normalizedValue - 0.25f));
	} else if (normalizedValue < 0.75f) {
		// Verde para amarelo
		glColor3f(4.0f * (normalizedValue - 0.5f), 1.0f, 0.0f);
	} else {
		// Amarelo para vermelho
		glColor3f(1.0f, 1.0f - 4.0f * (normalizedValue - 0.75f), 0.0f);
	}
}

// Função para renderizar
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Configurar câmera
	gluLookAt(0.0, 0.0, zoom, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Aplicar rotações
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	// Translação para centralizar o gráfico
	glTranslatef(-1.0f * scaleX, 0.0f, -3.0f * scaleZ);

	// Aplicar escala
	glScalef(scaleX, scaleY, 0.5f * scaleZ);

	float maxValue = getMaxSolutionValue();

	// Define o modo de desenho
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Desenha a malha
	for (int i = 0; i < N; ++i) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= M; ++j) {
			float x1, y1, x2, y2;

			// Ponto na “linha” i
			getCoordinates(i, j, x1, y1);
			float s1 = getSolutionValue(i, j);
			// Ponto na “linha” i+1
			getCoordinates(i + 1, j, x2, y2);
			float s2 = getSolutionValue(i + 1, j);

			// Vértice (x, altura, z)
			setColorByValue(s1, maxValue);
			glVertex3f(x1, y1, s1);

			setColorByValue(s2, maxValue);
			glVertex3f(x2, y2, s2);
		}
		glEnd();
	}
	// Desenha os eixos
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	// Eixo X - vermelho
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.2f, 0.0f, 0.0f);

	// Eixo Y - verde
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.2f, 0.0f);

	// Eixo Z - azul
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, maxValue * 1.2f);

	glEnd();
	glLineWidth(1.0f);

	glutSwapBuffers();
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC
		delete[] solution;
		exit(0);
		break;
	case 'w':
	case 'W':
		wireframe = !wireframe;
		break;
	case '+':
		zoom -= 0.5f;
		if (zoom < 1.1f)
			zoom = 1.1f;
		break;
	case '-':
		zoom += 0.5f;
		break;
	case 'r':
		scaleX = scaleY = scaleZ = 1.0f;
		zoom = 10.0f;
		break;
	case 'R':
		rotationX = 20.0f;
		rotationY = 180.0f;
		zoom = 10.0f;
		scaleZ = 1.0f;
		scaleX = scaleY = scaleZ = 1.0f;
		break;
	case 'x':
		scaleX += 0.1f;
		break;
	case 'X':
		scaleX -= 0.1f;
		if (scaleX < 0.1f)
			scaleX = 0.1f;
		break;
	// Escala Y
	case 'y':
		scaleY += 0.1f;
		break;
	case 'Y':
		scaleY -= 0.1f;
		if (scaleY < 0.1f)
			scaleY = 0.1f;
		break;
	// Escala Z
	case 'z':
		scaleZ += 0.1f;
		break;
	case 'Z':
		scaleZ -= 0.05f;
		if (scaleZ < 0.05f)
			scaleZ = 0.05f;
		break;
	// Escala uniforme
	case 's':
		scaleX += 0.1f;
		scaleY += 0.1f;
		scaleZ += 0.1f;
		break;
	case 'S':
		scaleX -= 0.1f;
		scaleY -= 0.1f;
		scaleZ -= 0.1f;
		if (scaleX < 0.1f)
			scaleX = 0.1f;
		if (scaleY < 0.1f)
			scaleY = 0.1f;
		if (scaleZ < 0.1f)
			scaleZ = 0.1f;
		break;
	}
	glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		rotationX -= 5.0f;
		break;
	case GLUT_KEY_DOWN:
		rotationX += 5.0f;
		break;
	case GLUT_KEY_LEFT:
		rotationY -= 5.0f;
		break;
	case GLUT_KEY_RIGHT:
		rotationY += 5.0f;
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	// Inicializa o GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Visualização da Equação de Poisson");

	// Configuração do OpenGL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	scaleX = scaleY = scaleZ = 1.0f;

	// Aloca memória para a solução
	solution = new float[(N - 1) * (M - 1)];

	// Calcula a solução numérica
	SolPoisson(N, M, solution);

	std::cout << "Controles:\n";
	std::cout << "Setas: Rotacionar visualização\n";
	std::cout << "+/-: Zoom in/out\n";
	std::cout << "W/w: Alternar modo wireframe\n";
	std::cout << "x/X: Aumentar/diminuir escala X\n";
	std::cout << "y/Y: Aumentar/diminuir escala Y\n";
	std::cout << "z/Z: Aumentar/diminuir escala Z\n";
	std::cout << "s/S: Aumentar/diminuir escala uniforme\n";
	std::cout << "R: Resetar visualização\n";
	std::cout << "ESC: Sair\n";

	// Registra callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeys);

	// Inicia o loop principal
	glutMainLoop();

	return 0;
}
