#include "Mesh.h"

using namespace std;

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

	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

/*
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
*/

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

Vector3f getRGBfromCGA(double* avgC);
void Mesh::draw()
{
	// Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	vector<double> colorV;
	for (unsigned v = 0; v < currentVertices.size(); v++)
	{
		double sumC = 0.0;
		int rigid = -1;
		for (int a = 0; a < attachments[v].size(); a++) {
			if (attachments[v][a] > 0)
				rigid++;
		}
		/*//======== The first color scheme
		if (rigid > 0) {
			for (int a = 0; a < attachments[v].size(); a++) {
				sumC = sumC + (double)(attachments[v][a] * (a + 1));
			}
			int tmp = floor(sumC + 0.5) - 1;
			//if (tmp == 0)
				//tmp++;
			colorV.push_back((double)tmp);    //produce 0-16 number
		}
		*/
		//=========== The second color scheme
		if (rigid > 0) {
			double tmp = 0.0;
			int idx = -1;
			for (int a = 0; a < attachments[v].size(); a++) {
				if (attachments[v][a] > tmp) {
					tmp = attachments[v][a];
					idx = a;
				}
			}
			colorV.push_back((double)idx);
		}
		else {
			colorV.push_back(-1.0);
		}
	}
	//==========================================================

	glBegin(GL_TRIANGLES);
	for (unsigned i = 0; i < faces.size(); i++)
	{

		double c_1 = colorV.at(faces[i][0] - 1);
		double c_2 = colorV.at(faces[i][0] - 1);
		double c_3 = colorV.at(faces[i][0] - 1);

		double* c1 = &c_1;		//Pass by Reference
		double* c2 = &c_2;
		double* c3 = &c_3;

		Vector3f t1 = currentVertices[faces[i][0] - 1];
		Vector3f t2 = currentVertices[faces[i][1] - 1];
		Vector3f t3 = currentVertices[faces[i][2] - 1];

		Vector3f v1 = t2 - t1;
		Vector3f v2 = t3 - t1;

		Vector3f n1 = Vector3f::cross(v1, v2);
		n1 = n1.normalized();

		glNormal3f(n1[0], n1[1], n1[2]);
		//Generate colors and vertices
		//glColor3f(float(c_1), 0.0, 0.0);
		glColor3f(float(getRGBfromCGA(c1)[0]), float(getRGBfromCGA(c1)[1]), float(getRGBfromCGA(c1)[2]));
		glVertex3f(t1[0], t1[1], t1[2]);
		glColor3f(getRGBfromCGA(c3)[0], getRGBfromCGA(c3)[1], getRGBfromCGA(c3)[2]);
		glVertex3f(t3[0], t3[1], t3[2]);
		glColor3f(getRGBfromCGA(c2)[0], getRGBfromCGA(c2)[1], getRGBfromCGA(c2)[2]);
		glVertex3f(t2[0], t2[1], t2[2]);

	}
	glEnd();
}


Vector3f getRGBfromCGA(double* avgC) {
	double cgaColor = *avgC;

	float red;
	float green;
	float blue;
	if (cgaColor == -1) { red = 0; green = 0; blue = 0; }				//Rigidly attached	- Black
	else if (cgaColor == 0) { red = 0.25; green = 0.25; blue = 0.25; }	//Chest				- Black
	else if (cgaColor == 1) { red = 0; green = 0; blue = 0.5; }			//Waist				- Navy
	else if (cgaColor == 2) { red = 0; green = 0.5; blue = 0; }			//Neck				- Green
	else if (cgaColor == 3) { red = 0; green = 0.5; blue = 0.5; }		//Right hip			- Teal
	else if (cgaColor == 4) { red = 0.5; green = 0; blue = 0; }			//Right leg			- Maroon
	else if (cgaColor == 5) { red = 0.5; green = 0; blue = 0.5; }		//Right knee		- Purple
	else if (cgaColor == 6) { red = 0.5; green = 0.5; blue = 0; }		//Right foot		- Brown
	else if (cgaColor == 7) { red = 0.75; green = 0.75; blue = 0.75; }	//Left hip			- Silver
	else if (cgaColor == 8) { red = 0.5; green = 0.5; blue = 0.5; }		//Left leg			- Gray
	else if (cgaColor == 9) { red = 0; green = 0; blue = 1; }			//Left knee			- Blue
	else if (cgaColor == 10) { red = 0; green = 1; blue = 0; }			//Left foot			- Lime
	else if (cgaColor == 11) { red = 0; green = 1; blue = 1; }			//Right collarbone	- Aqua
	else if (cgaColor == 12) { red = 1; green = 0; blue = 0; }			//Right shoulder	- Red
	else if (cgaColor == 13) { red = 1; green = 0; blue = 1; }			//Right elbow		- Fuchsia
	else if (cgaColor == 14) { red = 1; green = 1; blue = 0; }			//Left collarbone	- Yellow
	else if (cgaColor == 15) { red = 1; green = 1; blue = 0.5; }		//Left shoulder		- Faded Yellow
	else { red = 1; green = 1; blue = 1; }								//Left elbow		- White

	return Vector3f(red, green, blue);
}
