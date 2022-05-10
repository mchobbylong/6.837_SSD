#include "Mesh.h"

using namespace std;

void Mesh::load( const char* filename )
{
	// 2.1.1. load() should populate bindVertices, currentVertices, and faces

	ifstream file(filename);
	string action;
	float x, y, z;
	unsigned int n1, n2, n3;
	while (file >> action) {
		if (action == "v") {
			file >> x >> y >> z;
			bindVertices.push_back(Vector3f(x, y, z));
		}
		else if (action == "f") {
			file >> n1 >> n2 >> n3;
			faces.push_back(Tuple3u(n1, n2, n3));
		}
	}
	file.close();

	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::draw()
{
	// Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".
	
	glBegin(GL_TRIANGLES);
	for (int i = 0, numFaces = faces.size(); i < numFaces; ++i) {
		Vector3f vx = currentVertices[faces[i][0] - 1],
			vy = currentVertices[faces[i][1] - 1],
			vz = currentVertices[faces[i][2] - 1];
		
		Vector3f normal = Vector3f::cross(vy - vx, vz - vx).normalized();
		
		glNormal3f(normal[0], normal[1], normal[2]);
		glVertex3f(vx[0], vx[1], vx[2]);
		glVertex3f(vy[0], vy[1], vy[2]);
		glVertex3f(vz[0], vz[1], vz[2]);
	}
	glEnd();
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
}
