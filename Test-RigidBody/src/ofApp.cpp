#define GLM_FORCE_UNRESTRICTED_GENTYPE

#include "ofApp.h"
#include <ceres/ceres.h>

#define COUNT 100
#define NOISE 0.01

vector<glm::vec3> untransformedPoints;
vector<glm::vec3> transformedPoints;

template<typename T>
glm::tquat<T> eulerToQuat(const glm::tvec3<T> & eulerAngles) {
	glm::tvec3<T> c(cos(eulerAngles[0] * T(0.5)), cos(eulerAngles[1] * T(0.5)), cos(eulerAngles[2] * T(0.5)));
	glm::tvec3<T> s(sin(eulerAngles[0] * T(0.5)), sin(eulerAngles[1] * T(0.5)), sin(eulerAngles[2] * T(0.5)));

	glm::tquat<T> result;

	result.w = c.x * c.y * c.z + s.x * s.y * s.z;
	result.x = s.x * c.y * c.z - c.x * s.y * s.z;
	result.y = c.x * s.y * c.z + s.x * c.y * s.z;
	result.z = c.x * c.y * s.z - s.x * s.y * c.z;

	return result;
}

template<typename T>
glm::tmat4x4<T> createTransform(const glm::tvec3<T> & translation, glm::tvec3<T> & rotationVector)
{
	auto rotationQuat = eulerToQuat(rotationVector);
	auto rotationMat = glm::tmat4x4<T>(rotationQuat);
	return glm::translate(translation) * rotationMat;
}

struct RigidBodyTransformError {
	RigidBodyTransformError(const glm::tvec3<double> & untransformedPoint, const glm::tvec3<double> & transformedPoint)
		: untransformedPoint(untransformedPoint)
		, transformedPoint(transformedPoint) {}

	template <typename T>
	bool operator()(const T * const transformParameters
		, T * residuals) const {

		glm::tvec3<T> translation(transformParameters[0], transformParameters[1], transformParameters[2]);
		glm::tvec3<T> rotationVector(transformParameters[3], transformParameters[4], transformParameters[5]);

		glm::tvec4<T> predictedTransformedPoint(createTransform(translation, rotationVector) * glm::tvec4<T>(this->untransformedPoint, 1.0));
		predictedTransformedPoint /= predictedTransformedPoint.w;

		for (int i = 0; i < 3; i++) {
			residuals[i] = this->transformedPoint[i] - predictedTransformedPoint[i];
		}

		return true;
	}

	static ceres::CostFunction * Create(const glm::tvec3<double> & untransformedPoint, const glm::tvec3<double> & transformedPoint) {
		return (new ceres::AutoDiffCostFunction<RigidBodyTransformError, 3, 6>(
			new RigidBodyTransformError(untransformedPoint, transformedPoint)));
	}

	glm::tvec3<double> untransformedPoint;
	glm::tvec3<double> transformedPoint;
};

//--------------------------------------------------------------
void ofApp::setup(){
	// Create a random transform
	auto translation = glm::vec3(ofRandomf(), ofRandomf(), ofRandomf());
	auto rotationVector = glm::vec3(ofRandomf(), ofRandomf(), ofRandomf());
	auto transform = createTransform(translation, rotationVector);

	// Synthesise some data
	for (int i = 0; i < COUNT; i++) {
		auto untransformedPoint = glm::vec3(ofRandomf(), ofRandomf(), ofRandomf());
		auto transformedPoint = transform * untransformedPoint;

		transformedPoint += glm::vec3(ofRandomf(), ofRandomf(), ofRandomf()) * NOISE;

		untransformedPoints.push_back(untransformedPoint);
		transformedPoints.push_back(transformedPoint);
	}

	double parameters[6];
	for (auto & parameter : parameters) {
		parameter = 0.0;
	}

	ceres::Problem problem;
	for (int i = 0; i < COUNT; i++) {
		ceres::CostFunction * costFunction = RigidBodyTransformError::Create(untransformedPoints[i], transformedPoints[i]);
		problem.AddResidualBlock(costFunction
			, NULL
			, parameters);
	}

	ceres::Solver::Options options;
	options.linear_solver_type = ceres::DENSE_SCHUR;
	options.minimizer_progress_to_stdout = true;
	ceres::Solver::Summary summary;
	ceres::Solve(options, &problem, &summary);
	std::cout << summary.FullReport() << "\n";
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
