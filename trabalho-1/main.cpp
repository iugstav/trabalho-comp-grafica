#include "IM472.h"
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>

// Parâmetros da malha e domínio
const int N = 50; // divisões em x [0,XMAX]
const int M = 25; // divisões em y [0,YMAX]
const float XMAX = 4.0f;
const float YMAX = 2.0f;

// Parâmetros de câmera
float targetX = 0.0f, targetY = 0.5f * YMAX, targetZ = 0.0f;
float radius = 12.0f;
float azimuth = 45.0f;
float elevation = 30.0f;

// Sensibilidades
const float angularSpeed = 5.0f;
const float zoomSpeed = 0.5f;
const float panSpeed = 0.1f;

// Wireframe toggle
bool wireframe = false;

// Estrutura de vértice (posição + cor)
struct Vertex {
	float pos[3];
	float col[3];
};

// Arrays de vértices e índices
static std::vector<Vertex> vertices;
static std::vector<GLuint> indices;

// Valor máximo para normalização de cor
float maxValue = 1.0f;

// Converte (i,j) em coordenadas x,y
void getCoordinates(int i, int j, float &x, float &y) {
	x = (XMAX * i) / float(N);
	y = (YMAX * j) / float(M);
}

// Obtém valor da solução (inclui contorno)
float getSolutionValue(int i, int j) {
	// contornos:
	if (i == 0)
		return 0.0f;
	if (i == N)
		return 2.0f * expf((YMAX * j) / M);
	if (j == 0)
		return (XMAX * i) / N;
	if (j == M)
		return expf((XMAX * i) / N);
	// interior: sol armazenada em vertices
	int idx = i * (M + 1) + j;
	return vertices[idx].pos[1];
}

// Calcula maxValue após geração de dados
float computeMaxValue() {
	float mv = 0.0f;
	for (auto &v : vertices)
		mv = std::max(mv, fabsf(v.pos[1]));
	return mv;
}

// Mapeia valor para cor RGB
void valueToColor(float s, float &r, float &g, float &b) {
	float norm = 0.5f * (s / maxValue + 1.0f);
	if (norm < 0.25f) {
		r = 0.0f;
		g = 4.0f * norm;
		b = 1.0f;
	} else if (norm < 0.5f) {
		r = 0.0f;
		g = 1.0f;
		b = 1.0f - 4.0f * (norm - 0.25f);
	} else if (norm < 0.75f) {
		r = 4.0f * (norm - 0.5f);
		g = 1.0f;
		b = 0.0f;
	} else {
		r = 1.0f;
		g = 1.0f - 4.0f * (norm - 0.75f);
		b = 0.0f;
	}
}

// Prepara buffers de vértices e índices
void buildMesh() {
	// gera vértices
	vertices.resize((N + 1) * (M + 1));
	for (int i = 0; i <= N; ++i) {
		for (int j = 0; j <= M; ++j) {
			int idx = i * (M + 1) + j;
			float x, y;
			getCoordinates(i, j, x, y);
			float s;

			// condição de contorno / interior (temporariamente setar s)
			if (i == 0 || i == N || j == 0 || j == M)
				s = getSolutionValue(i, j);
			else
				s = 0.0f; // placeholder, será preenchido por SolPoisson

			vertices[idx].pos[0] = x;
			vertices[idx].pos[1] = s;
			vertices[idx].pos[2] = y;
		}
	}

	// executa SolPoisson para interiores
	std::vector<float> sol((N - 1) * (M - 1));
	SolPoisson(N, M, sol.data());

	int k = 0;
	for (int j = 1; j < M; ++j)
		for (int i = 1; i < N; ++i) {
			int idx = i * (M + 1) + j;
			vertices[idx].pos[1] = sol[k++];
		}

	// calcula maxValue
	maxValue = computeMaxValue();
	// aplica cor
	for (auto &v : vertices) {
		float r, g, b;
		valueToColor(v.pos[1], r, g, b);
		v.col[0] = r;
		v.col[1] = g;
		v.col[2] = b;
	}

	// gera índices (2 triângulos por célula)
	indices.clear();
	indices.reserve(N * M * 6);
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			GLuint v0 = i * (M + 1) + j;
			GLuint v1 = (i + 1) * (M + 1) + j;
			GLuint v2 = (i + 1) * (M + 1) + (j + 1);
			GLuint v3 = i * (M + 1) + (j + 1);
			indices.insert(indices.end(), {v0, v1, v2, v0, v2, v3});
		}
	}
}

void drawAxes() {
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	// X - vermelho
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(XMAX * 1.1f, 0, 0);
	// Y - azul
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, maxValue * 1.1f, 0);
	// Z - verde
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, YMAX * 1.1f);
	glEnd();
	glLineWidth(1.0f);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// calcula posição da câmera
	float radAz = azimuth * M_PI / 180.0f;
	float radEl = elevation * M_PI / 180.0f;
	float cosEl = cos(radEl), sinEl = sin(radEl);
	float cosAz = cos(radAz), sinAz = sin(radAz);
	float camX = targetX + radius * cosEl * sinAz;
	float camY = targetY + radius * sinEl;
	float camZ = targetZ + radius * cosEl * cosAz;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camX, camY, camZ, targetX, targetY, targetZ, 0, 1, 0);

	// eixos
	drawAxes();

	// mesh com vertex arrays
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
	glColorPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].col);

	glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, &indices[0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, float(w) / h, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
	case 'u':
	case 'U':
		radius = std::max(0.1f, radius - zoomSpeed);
		break;
	case 'j':
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
	case 'a':
	case 'A':
		azimuth -= angularSpeed;
		break;
	case 'd':
	case 'D':
		azimuth += angularSpeed;
		break;
	case 'w':
	case 'W':
		elevation -= angularSpeed;
		break;
	case 's':
	case 'S':
		elevation += angularSpeed;
		break;
	case 'f':
	case 'F':
		wireframe = !wireframe;
		break;
	case 'r':
	case 'R':
		radius = 12;
		azimuth = 45;
		elevation = 30;
		targetX = 0;
		targetY = 0.5f * YMAX;
		targetZ = 0;
		break;
	}
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
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	// Inicializa GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Visualização da Equação de Poisson");

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	// Monta mesh de vértices e índices
	buildMesh();

	std::cout << "Controles:\n"
		  << "w/a/s/d: rotacionar\n"
		  << "f: wireframe\n"
		  << "u/j: zoom in/out\n"
		  << "i/k: pan vertical\n"
		  << "setas: pan horizontal\n"
		  << "r: reset\n"
		  << "ESC: sair\n";

	// Callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeys);

	glutMainLoop();
	return 0;
}

