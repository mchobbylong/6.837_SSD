#include "Mesh.h"

using namespace std;

static const Vector3f jointColorMapping[] = {
	Vector3f(0, 0, 0),					// Root					- Black
	Vector3f(0.25, 0.25, 0.25),			// Chest				- Faded Black
	Vector3f(0, 0, 0.5),				// Waist				- Navy
	Vector3f(0, 0.5, 0),				// Neck					- Green
	Vector3f(0, 0.5, 0.5),				// Right hip			- Teal
	Vector3f(0.5, 0, 0),				// Right leg			- Maroon
	Vector3f(0.5, 0, 0.5),				// Right knee			- Purple
	Vector3f(0.5, 0.5, 0),				// Right foot			- Brown
	Vector3f(0.75, 0.75, 0.75),			// Left hip				- Silver
	Vector3f(0.5, 0.5, 0.5),			// Left leg				- Gray
	Vector3f(0, 0, 1),					// Left knee			- Blue
	Vector3f(0, 1, 0),					// Left foot			- Lime
	Vector3f(0, 1, 1),					// Right collarbone		- Aqua
	Vector3f(1, 0, 0),					// Right shoulder		- Red
	Vector3f(1, 0, 1),					// Right elbow			- Fuchsia
	Vector3f(1, 1, 0),					// Left collarbone		- Yellow
	Vector3f(1, 1, 0.5),				// Left shoulder		- Faded Yellow
	Vector3f(1, 1, 1),					// Left elbow			- White
};

#define COLOR_SCHEME_1

void Mesh::load( const char* filename )
{
	// 2.1.1. load() should populate bindVertices, currentVertices, and faces

	ifstream file(filename);
	string action, value;
	float x, y, z;
	unsigned int vertexIndex[3];
	int loc;
	while (file >> action) {
		if (action == "v") {
			file >> x >> y >> z;
			bindVertices.push_back(Vector3f(x, y, z));
		}
		else if (action == "f") {
			for (int i = 0; i < 3; ++i) {
				file >> value;
				if ((loc = value.find('/')) != string::npos) {
					// read the first number (vertex index) only
					vertexIndex[i] = stoi(value.substr(0, loc));
				}
				else
					vertexIndex[i] = stoi(value);
			}
			faces.push_back(Tuple3u(vertexIndex[0], vertexIndex[1], vertexIndex[2]));
		}
	}
	file.close();

	// Make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
	// 2.2. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments

	ifstream file(filename);
	float weight;

	for (int i = 0, numVertices = currentVertices.size(); i < numVertices; ++i) {
		auto attachment = vector<float>();
		attachment.push_back(0);

		for (int j = 1; j < numJoints; ++j) {
			file >> weight;
			attachment.push_back(weight);
		}
		attachments.push_back(attachment);
	}
	file.close();

	// Generate vertex colors after loading attachments
	for (int v = 0, numVertices = currentVertices.size(); v < numVertices; v++)
	{
		int numBinds = 0;
		// If rigid, set the default vertex color as black
		Vector3f weightedColor = Vector3f(0);

		for (int a = 0; a < numJoints; a++)
			if (attachments[v][a] > 0)
				numBinds++;

#ifdef COLOR_SCHEME_1
		// Using the first color scheme
		if (numBinds > 1) {
			for (int a = 0; a < numJoints; a++)
				// If number of joints exceed 18 (not matching pre-defined color), assume the matching color is white
				if (a > 17)
					weightedColor += attachments[v][a] * Vector3f(1.0);
				else
					weightedColor += attachments[v][a] * jointColorMapping[a];
		}
#elif defined(COLOR_SCHEME_2)
		// Using the second color scheme
		int idx = 0;
		for (int a = 1; a < numJoints; a++)
			if (attachments[v][a] > attachments[v][idx])
				idx = a;
		// If number of joints exceed 18 (not matching pre-defined color), assume the matching color is white
		weightedColor = idx > 18
			? Vector3f(1.0f)
			: jointColorMapping[idx];
#endif
		vertexColors.push_back(weightedColor);
	}
}

void Mesh::draw()
{
	// Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glBegin(GL_TRIANGLES);
	for (int i = 0, numFaces = faces.size(); i < numFaces; ++i) {
		int ix = faces[i][0] - 1,
			iy = faces[i][1] - 1,
			iz = faces[i][2] - 1;
		Vector3f vx = currentVertices[ix],
			vy = currentVertices[iy],
			vz = currentVertices[iz],
			cx = vertexColors[ix],
			cy = vertexColors[iy],
			cz = vertexColors[iz];

		Vector3f normal = Vector3f::cross(vy - vx, vz - vx).normalized();

		glNormal3f(normal[0], normal[1], normal[2]);
		glColor3f(cx[0], cx[1], cx[2]);
		glVertex3f(vx[0], vx[1], vx[2]);
		glColor3f(cy[0], cy[1], cy[2]);
		glVertex3f(vy[0], vy[1], vy[2]);
		glColor3f(cz[0], cz[1], cz[2]);
		glVertex3f(vz[0], vz[1], vz[2]);
	}
	glEnd();
}
