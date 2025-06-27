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

float maxValue = 1.0f;

// Ponto central do gráfico (alvo). Ajuste conforme a posição real do seu gráfico.
float targetX = 0.0f, targetY = 1.5f, targetZ = 0.0f;

// Parâmetros polares da câmera em torno do alvo
float radius = 12.0f; // distância inicial entre câmera e alvo
float azimuth = 0.0f; // ângulo horizontal em graus (gira em torno do eixo Y do alvo)
float elevation = 10.0f; // ângulo vertical em graus (inclinação acima do plano XZ)

// Vetor “up” fixo. Normalmente (0,1,0).
float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

// parâmetros de sensibilidade
const float angularSpeed = 5.0f; // graus por tecla para azimuth/elevation
const float zoomSpeed = 0.5f; // quanto muda o radius por tecla
const float panSpeed = 0.1f;

float meshRotX = 0.0f; // rotação da malha em torno do eixo X local
float meshRotY = 0.0f; // rotação da malha em torno do eixo Y local

// visualização do wireframe
bool wireframe = false;

// converte os índices da malha (i,j) para coordenadas (x,y)
void getCoordinates(int i, int j, float &x, float &y) {
	x = (float)i * h;
	y = (float)j * k;
}

// obtém o valor da solução no ponto (i,j)
float getSolutionValue(int i, int j) {
	// pontos interiores da malha
	if (i > 0 && i < N && j > 0 && j < M) {
		return solution[(i - 1) + (j - 1) * (N - 1)];
	}

	float x, y;
	getCoordinates(i, j, x, y);

	// condições de contorno
	if (i == 0)
		return 0.0f; // u(0,y) = 0
	if (i == N)
		return 2.0f * expf(y); // u(2,y) = 2e^y
	if (j == 0)
		return x; // u(x,0) = x
	if (j == M)
		return expf(x); // u(x,1) = e^x

	return 0.0f;
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

void drawMesh() {
	// Desenha faixas entre i e i+1
	for (int i = 0; i < N; ++i) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= M; ++j) {
			// ponto (i, j)
			float x1, y1;
			getCoordinates(i, j, x1, y1);
			float s1 = getSolutionValue(i, j);
			// ponto (i+1, j)
			float x2, y2;
			getCoordinates(i + 1, j, x2, y2);
			float s2 = getSolutionValue(i + 1, j);

			// Vértice: X, altura (s), profundidade (y)
			setColorByValue(s1, maxValue);
			glVertex3f(x1, s1, y1);

			setColorByValue(s2, maxValue);
			glVertex3f(x2, s2, y2);
		}
		glEnd();
	}
}

void drawAxes() {
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	// Eixo X - vermelho
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.2f, 0.0f, 0.0f);

	// Eixo Y - azul
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.2f);

	// Eixo Z - verde
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, maxValue * 1.f, 0.0f);

	glEnd();
	glLineWidth(1.0f);
}

// Função para renderizar
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float radAz = azimuth * M_PI / 180.0f;
	float radEl = elevation * M_PI / 180.0f;
	// Cálculo da posição da câmera em coordenadas esféricas em torno do alvo:
	float cosEl = cos(radEl);
	float sinEl = sin(radEl);
	float cosAz = cos(radAz);
	float sinAz = sin(radAz);

	float camX = targetX + radius * cosEl * sinAz;
	float camY = targetY + radius * sinEl;
	float camZ = targetZ + radius * cosEl * cosAz;

	// Configura view matrix com gluLookAt
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camX, camY, camZ, targetX, targetY, targetZ, upX, upY, upZ);

	// Define o modo de desenho
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glPushMatrix();
	glTranslatef(-targetX, -targetY, -targetZ);
	glRotatef(meshRotX, 1, 0, 0);
	glRotatef(meshRotY, 0, 1, 0);

	drawAxes();
	drawMesh();
	glPopMatrix();

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

	// Zoom (muda radius)
	case 'u': // aproxima
	case 'U':
		radius -= zoomSpeed;
		if (radius < 0.1f)
			radius = 0.1f; // evita radius <= 0
		break;
	case 'j': // afasta
	case 'J':
		radius += zoomSpeed;
		break;
	case 'i':
	case 'I':
		targetY -= panSpeed;
		break;
	case 'k':
	case 'K':
		targetY += panSpeed;
		break;
	// Rotação local da malha
	case 'a':
	case 'A':
		meshRotY -= angularSpeed; // gira malha em torno de Y local
		break;
	case 'd':
	case 'D':
		meshRotY += angularSpeed;
		break;
	case 'w':
	case 'W':
		meshRotX -= angularSpeed; // gira malha em torno de X local
		break;
	case 's':
	case 'S':
		meshRotX += angularSpeed;
		break;
	// Alterna wireframe
	case 'f':
	case 'F':
		wireframe = !wireframe;
		break;

	// Recentralizar orbit
	case 'r':
	case 'R':
		radius = 12.0f;
		azimuth = 0.0f;
		elevation = 10.0f;
		targetX = 0.0f;
		targetY = 1.5f;
		targetZ = 0.f;
		meshRotX = meshRotY = 0.0f;
		break;
	};
	glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		targetX += panSpeed;
		break;
	case GLUT_KEY_RIGHT:
		targetX -= panSpeed;
		break;
	case GLUT_KEY_UP:
		targetZ += panSpeed;
		break;
	case GLUT_KEY_DOWN:
		targetZ -= panSpeed;
		break;
	case GLUT_KEY_PAGE_UP:
		targetY += panSpeed;
		break;
	case GLUT_KEY_PAGE_DOWN:
		targetY -= panSpeed;
		break;
	default:
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

	// Aloca memória para a solução
	solution = new float[(N - 1) * (M - 1)];

	// Calcula a solução numérica
	SolPoisson(N, M, solution);
	maxValue = getMaxSolutionValue();

	std::cout << "Controles:\n";
	std::cout << "w/a/s/d: Rotacionar visualização\n";
	std::cout << "f/F: Alternar modo wireframe\n";
	std::cout << "u/U: Aproxima\n";
	std::cout << "j/J: Afasta\n";
	std::cout << "i/I: Desloca para baixo\n";
	std::cout << "k/K: Desloca para cima\n";
	std::cout << "r/R: Resetar visualização\n";
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
