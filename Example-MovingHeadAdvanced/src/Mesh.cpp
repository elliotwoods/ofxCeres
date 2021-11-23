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
			this->models.clear();
		}
	}
}

//---------
void
Mesh::drawWorld()
{
	const auto& enableTextures = this->parameters.drawStyle.enableTextures.get();
	const auto& enableMaterials = this->parameters.drawStyle.enableMaterials.get();

	const auto& cullFaces = this->parameters.drawStyle.cullBackFaces.fill.get();
	const auto& cullWireframe = this->parameters.drawStyle.cullBackFaces.wireframe.get();

	ofPushMatrix();
	{
		ofMultMatrix(this->getTransform());

		ofPushStyle();
		{
			ofSetColor(255);

			auto drawCall = [this, &enableMaterials, &enableTextures, &cullFaces, &cullWireframe](bool wireframe) {
				const auto& cull = wireframe ? cullWireframe : cullFaces;

				if (cull) {
					glEnable(GL_CULL_FACE);
					glCullFace(GL_BACK);
				}

				for (const auto& model : this->models) {
					if (enableTextures) {
						model.texture.bind();
					}

					if (enableMaterials) {
						model.material.begin();
					}

					wireframe ? model.mesh.drawWireframe()
						: model.mesh.drawFaces();

					if (enableMaterials) {
						model.material.end();
					}

					if (enableTextures) {
						model.texture.unbind();
					}
				}

				if (cull) {
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

					drawCall(true);

					ofSetColor(100);

					drawCall(false);
				}
				glPopAttrib();
				break;
			}
			case DrawStyle::Fill:
			{
				drawCall(false);
				break;
			}
			case DrawStyle::Wireframe:
			{
				drawCall(true);
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
		return this->sceneMin;
		});
	inspector->addLiveValue<glm::vec3>("Scene Max", [this]() {
		return this->sceneMax;
		});
	inspector->addLiveValue<size_t>("Model count", [this]() {
		return this->models.size();
		});
	inspector->addButton("Reload", [this]() {
		this->models.clear();
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
	for (size_t i = 0; i < meshCount; i++) {
		auto& meshHelper = modelLoader.getMeshHelper(i);
		this->models.push_back({
			meshHelper.cachedMesh
			, meshHelper.assimpTexture.getTextureRef()
			, meshHelper.material
			});

		// Apply transform
		{
			auto& mesh = this->models[i].mesh;
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

		for (const auto& model : this->models) {
			const auto& vertices = model.mesh.getVertices();
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

		this->sceneMin = modelMin;
		this->sceneMax = modelMax;
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
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3) this->sceneMin);
}

//---------
glm::vec3
Mesh::getMaxWorld()
{
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3)this->sceneMax);
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

	for(const auto& model : this->models) {
		auto& vertices = model.mesh.getVertices();
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