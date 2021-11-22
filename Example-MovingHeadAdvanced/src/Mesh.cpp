#include "pch_ofApp.h"
#include "Mesh.h"

//---------
Mesh::Mesh()
{
	RULR_SERIALIZE_LISTENERS;
	RULR_INSPECTOR_LISTENER;
}

//---------
string
Mesh::getTypeName() const
{
	return "Mesh";
}

//---------
void
Mesh::update()
{
	if (this->loadedPath != this->parameters.filename) {
		try {
			this->loadMesh();
		}
		catch (Exception e) {
			ofLogError("Mesh") << "Couldn't load mesh " << this->parameters.filename.get();
			this->parameters.filename.set(filesystem::path());
			this->model.meshes.clear();
			this->model.materials.clear();
		}
	}
}

//---------
void
Mesh::drawWorld()
{
	const auto& enableMaterials = this->parameters.enableMaterials.get();

	ofPushMatrix();
	{
		ofMultMatrix(this->getTransform());

		ofPushStyle();
		{
			ofSetColor(255);

			auto cullOn = [](bool enabled) {
				if (enabled) {
					glEnable(GL_CULL_FACE);
					glCullFace(GL_BACK);
				}
			};

			auto cullOff = [](bool enabled) {
				if (enabled) {
					glDisable(GL_CULL_FACE);
				}
			};

			switch (this->drawStyle.get().get()) {
			case DrawStyle::Mix:
			{
				glPushAttrib(GL_POLYGON_BIT);
				{
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1.0, 1.0);

					cullOn(this->parameters.cullBackFaces.wireframe.get());
					{
						for (size_t i = 0; i < this->model.meshes.size(); i++) {
							if (enableMaterials) {
								this->model.materials[i].begin();
							}
							this->model.meshes[i].drawWireframe();

							if (enableMaterials) {
								this->model.materials[i].end();
							}
						}
					}
					cullOff(this->parameters.cullBackFaces.wireframe.get());

					ofSetColor(100);

					cullOn(this->parameters.cullBackFaces.fill.get());
					{
						for (size_t i = 0; i < this->model.meshes.size(); i++) {
							if (enableMaterials) {
								this->model.materials[i].begin();
							}
							this->model.meshes[i].drawFaces();

							if (enableMaterials) {
								this->model.materials[i].end();
							}
						}
					}
					cullOff(this->parameters.cullBackFaces.fill.get());
				}
				glPopAttrib();
				break;
			}
			case DrawStyle::Fill:
			{
				cullOn(this->parameters.cullBackFaces.fill.get());
				{
					for (size_t i = 0; i < this->model.meshes.size(); i++) {
						if (enableMaterials) {
							this->model.materials[i].begin();
						}
						this->model.meshes[i].drawFaces();

						if (enableMaterials) {
							this->model.materials[i].end();
						}
					}
				}
				cullOff(this->parameters.cullBackFaces.fill.get());
				break;
			}
			case DrawStyle::Wireframe:
			{
				cullOn(this->parameters.cullBackFaces.wireframe.get());
				{
					for (size_t i = 0; i < this->model.meshes.size(); i++) {
						if (enableMaterials) {
							this->model.materials[i].begin();
						}
						this->model.meshes[i].drawWireframe();

						if (enableMaterials) {
							this->model.materials[i].end();
						}
					}
				}
				cullOff(this->parameters.cullBackFaces.wireframe.get());
				break;
			}
			default:
				break;
			}
		}
		ofPopStyle();
	}
	ofPopMatrix();
}

//---------
void
Mesh::serialize(nlohmann::json& json)
{
	Data::serialize(json, this->parameters);
	Data::serializeEnum(json, this->drawStyle);
}

//---------
void
Mesh::deserialize(const nlohmann::json& json)
{
	Data::deserialize(json, this->parameters);
	Data::deserializeEnum(json, this->drawStyle);
}

//---------
void
Mesh::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addParameterGroup(this->parameters);
	{
		auto widget = inspector->addMultipleChoice("Draw style");
		widget->entangleManagedEnum(this->drawStyle);
	}

	inspector->addTitle("Model info", ofxCvGui::Widgets::Title::H3);
	inspector->addLiveValue<glm::vec3>("Scene Min", [this]() {
		return this->model.modelMin;
		});
	inspector->addLiveValue<glm::vec3>("Scene Max", [this]() {
		return this->model.modelMax;
		});
	inspector->addLiveValue<size_t>("Mesh count", [this]() {
		return this->model.meshes.size();
		});
	inspector->addButton("Reload", [this]() {
		this->model.meshes.clear();
		this->model.materials.clear();
		this->loadedPath = "";
		});

}

//---------
void
Mesh::loadMesh()
{
	ofxAssimpModelLoader modelLoader;

	modelLoader.loadModel(this->parameters.filename.get().string());
	if (!modelLoader.hasMeshes()) {
		throw(Exception("File does not contain any meshes"));
	}
	modelLoader.setScaleNormalization(false);

	auto meshCount = modelLoader.getMeshCount();

	// Copy the meshes across to local
	for (int i = 0; i < meshCount; i++) {
		const auto& meshHelper = modelLoader.getMeshHelper(i);
		this->model.meshes.push_back(meshHelper.cachedMesh);
		this->model.materials.push_back(meshHelper.material);

		// Apply transform
		{
			auto& mesh = this->model.meshes[i];
			auto vertices = mesh.getVertices();
			for (auto& vertex : vertices) {
				vertex = ofxCeres::VectorMath::applyTransform((glm::mat4) meshHelper.matrix, vertex);
			}
		}
	}

	// Take the min and max
	{
		auto modelMin = glm::vec3(1.0, 1.0, 1.0) * std::numeric_limits<float>::max();
		auto modelMax = glm::vec3(1.0, 1.0, 1.0) * std::numeric_limits<float>::min();

		for (const auto& mesh : this->model.meshes) {
			const auto& vertices = mesh.getVertices();
			for (const auto& vertex : vertices) {
				for (int c = 0; c < 3; c++) {
					if (vertex[c] < modelMin[c]) {
						modelMin[c] = vertex[c];
					}
					if (vertex[c] > modelMax[c]) {
						modelMax[c] = vertex[c];
					}
				}
			}
		}

		this->model.modelMin = modelMin;
		this->model.modelMax = modelMax;
	}

	this->loadedPath = this->parameters.filename;
}

//---------
bool
Mesh::isLoaded()
{
	return !this->loadedPath.empty();
}

//---------
glm::vec3
Mesh::getMinWorld()
{
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3) this->model.modelMin);
}

//---------
glm::vec3
Mesh::getMaxWorld()
{
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3)this->model.modelMax);
}

//---------
glm::mat4
Mesh::getTransform() const
{
	return glm::scale(this->parameters.scale.get() * glm::vec3(1, 1, 1))
		* (glm::mat4) ofxCeres::VectorMath::eulerToQuat(glm::vec3{
				this->parameters.rotation.x.get()
				, this->parameters.rotation.y.get()
				, this->parameters.rotation.z.get()
			} *DEG_TO_RAD);
}

//---------
glm::vec3
Mesh::getPointClosestTo(const glm::vec3& samplePosition, float maxDistance)
{
	// This value will be used if no close vertex is found
	auto closestVertex = samplePosition;
	auto minDistance = numeric_limits<float>::max();

	for(const auto& mesh : this->model.meshes) {
		auto& vertices = mesh.getVertices();
		for (const auto& rawVertex : vertices) {
			auto transform = this->getTransform();

			auto vertexPosition = ofxCeres::VectorMath::applyTransform(transform, rawVertex);
			auto distance = glm::distance(vertexPosition, samplePosition);
			if (distance > maxDistance) {
				continue;
			}

			if (distance < minDistance) {
				minDistance = distance;
				closestVertex = vertexPosition;
			}
		}
	}

	return closestVertex;
}