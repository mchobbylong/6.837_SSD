#include "SkeletalModel.h"

#include <FL/Fl.H>

using namespace std;

static inline void trim_string(string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

vector<Joint*> SkeletalModel::getJoints()
{
	return m_joints;
}

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
	loadSkeleton(skeletonFile);

	m_mesh.load(meshFile);
	m_mesh.loadAttachments(attachmentsFile, m_joints.size());

	computeBindWorldToJointTransforms();
	updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(Matrix4f cameraMatrix, bool skeletonVisible)
{
	// draw() gets called whenever a redraw is required
	// (after an update() occurs, when the camera moves, the window is resized, etc)

	m_matrixStack.clear();
	m_matrixStack.push(cameraMatrix);

	if( skeletonVisible )
	{
		drawJoints();
		drawSkeleton();
	}
	else
	{
		// Clear out any weird matrix we may have been using for drawing the bones and revert to the camera matrix.
		glLoadMatrixf(m_matrixStack.top());

		// Tell the mesh to draw itself.
		m_mesh.draw();
	}
}

void SkeletalModel::loadSkeleton( const char* filename )
{
	// Load the skeleton from file here.
	ifstream stream(filename);
	float x, y, z;
	int parent;
	Joint *joint;
	// m_joints = vector<Joint*>();
	while (stream >> x >> y >> z >> parent) {
		joint = new Joint();
		m_joints.push_back(joint);
		// joint->children = vector<Joint*>();
		joint->transform = Matrix4f::translation(x, y, z);
		if (parent == -1) {
			m_rootJoint = joint;
			m_rootTranslation = Vector3f(x, y, z);
		}
		else
			m_joints[parent]->children.push_back(joint);

		// Read joint name. If not specified, then the name is empty
		getline(stream, joint->name);
		trim_string(joint->name);
	}
	stream.close();
	cout << "Read joints: " << m_joints.size() << endl;
}

void recursiveDrawJoints(Joint *joint, MatrixStack &stack)
{
	// Push current joint transformation into stack
	stack.push(joint->transform);

	// Draw current joint
	glLoadMatrixf(stack.top());
	glutSolidSphere(0.025f, 12, 12);

	// Iterate children joints recursively
	for (auto childJoint : joint->children)
		recursiveDrawJoints(childJoint, stack);

	// Pop out current joint
	stack.pop();
}

void SkeletalModel::drawJoints( )
{
	// If no joint is loaded, then does nothing
	if (m_joints.size() == 0) return;


	// Draw a sphere at each joint. You will need to add a recursive helper function to traverse the joint hierarchy.
	//
	// We recommend using glutSolidSphere( 0.025f, 12, 12 )
	// to draw a sphere of reasonable size.
	//
	// You are *not* permitted to use the OpenGL matrix stack commands
	// (glPushMatrix, glPopMatrix, glMultMatrix).
	// You should use your MatrixStack class
	// and use glLoadMatrix() before your drawing call.

	recursiveDrawJoints(m_rootJoint, m_matrixStack);
}

const Vector3f RND(0, 0, 1);

void recursiveDrawSkeletons(Joint* joint, MatrixStack& stack, bool hasParent)
{
	// Draw the skeleton from parent joint to current joint
	if (hasParent) {
		// Compute the length
		float length = joint->transform.getCol(3).xyz().abs();

		// Assemble the transformation for direction
		Vector3f directionZ = joint->transform.getCol(3).xyz().normalized(),
			directionY = Vector3f::cross(directionZ, RND).normalized(),
			directionX = Vector3f::cross(directionY, directionZ).normalized();
		Matrix4f directionTransform = Matrix4f::identity();
		directionTransform.setSubmatrix3x3(0, 0, Matrix3f(directionX, directionY, directionZ, true));

		// Apply transformations for the cube primitive
		Matrix4f cubeTransform =
			directionTransform
			* Matrix4f::scaling(0.05, 0.05, length)
			* Matrix4f::translation(0, 0, 0.5);

		// Draw the cube
		stack.push(cubeTransform);
		glLoadMatrixf(stack.top());
		glutSolidCube(1.0f);
		stack.pop();
	}

	// Push current joint transformation into stack
	stack.push(joint->transform);

	// Draw skeletons between current joint and its every child
	for (auto childJoint : joint->children)
		recursiveDrawSkeletons(childJoint, stack, true);

	// Pop out current joint
	stack.pop();
}

void SkeletalModel::drawSkeleton()
{
	// If no joint is loaded, then does nothing
	if (m_joints.size() == 0) return;
	// Draw boxes between the joints. You will need to add a recursive helper function to traverse the joint hierarchy.
	recursiveDrawSkeletons(m_rootJoint, m_matrixStack, false);
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
	// Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
	Matrix4f rotateX = Matrix4f::rotateX(rX),
		rotateY = Matrix4f::rotateY(rY),
		rotateZ = Matrix4f::rotateZ(rZ),
		rotate = rotateX * rotateY * rotateZ;

	m_joints[jointIndex]->transform.setSubmatrix3x3(0, 0, rotate.getSubmatrix3x3(0, 0));
}

void SkeletalModel::setRootTranslation(float dX, float dY, float dZ)
{
	Vector3f updatedTranslation = m_rootTranslation + Vector3f(dX, dY, dZ);
	m_rootJoint->transform.setCol(3, Vector4f(updatedTranslation, 1));
}

void recursiveComputeBindWorldToJointTransforms(Joint* joint, MatrixStack& stack)
{
	// Get the inverse transform (joint2world -> world2joint) by inversion
	stack.push(joint->transform);
	joint->bindWorldToJointTransform = stack.top().inverse();

	for (auto childJoint : joint->children)
		recursiveComputeBindWorldToJointTransforms(childJoint, stack);

	stack.pop();
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
	// If no joint is loaded, then does nothing
	if (m_joints.size() == 0) return;


	// 2.3.1. Implement this method to compute a per-joint transform from
	// world-space to joint space in the BIND POSE.
	//
	// Note that this needs to be computed only once since there is only
	// a single bind pose.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.

	recursiveComputeBindWorldToJointTransforms(m_rootJoint, MatrixStack());
}

void recursiveUpdateCurrentJointToWorldTransforms(Joint* joint, MatrixStack& stack)
{
	// Get the transform (joint2world)
	stack.push(joint->transform);
	joint->currentJointToWorldTransform = stack.top();

	for (auto childJoint : joint->children)
		recursiveUpdateCurrentJointToWorldTransforms(childJoint, stack);

	stack.pop();
}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
	// If no joint is loaded, then does nothing
	if (m_joints.size() == 0) return;


	// 2.3.2. Implement this method to compute a per-joint transform from
	// joint space to world space in the CURRENT POSE.
	//
	// The current pose is defined by the rotations you've applied to the
	// joints and hence needs to be *updated* every time the joint angles change.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.

	recursiveUpdateCurrentJointToWorldTransforms(m_rootJoint, MatrixStack());
}

void SkeletalModel::updateMesh()
{
	// 2.3.2. This is the core of SSD.
	// Implement this method to update the vertices of the mesh
	// given the current state of the skeleton.
	// You will need both the bind pose world --> joint transforms.
	// and the current joint --> world transforms.

	m_mesh.currentVertices.clear();
	int numJoints = m_joints.size();

	for (int i = 0, numVertices = m_mesh.bindVertices.size(); i < numVertices; ++i) {
		Vector4f weighted(0.f);

		for (int j = 0; j < numJoints; ++j)
			weighted = weighted +
				m_joints[j]->currentJointToWorldTransform
				* m_joints[j]->bindWorldToJointTransform
				* Vector4f(m_mesh.bindVertices[i], 1.0f) * m_mesh.attachments[i][j];

		m_mesh.currentVertices.push_back(weighted.xyz());
	}
}
