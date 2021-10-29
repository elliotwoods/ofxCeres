#include "pch_ofApp.h"
#include "Mesh.h"

//---------
Mesh::Mesh()
{
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->populateInspector(args);
	};
}

//---------
string
Mesh::getTypeName() const
{
	return "Mesh";
}

//---------
void
Mesh::drawWorld()
{
	ofPushMatrix();
	{
		ofScale(this->parameters.scale.get() * glm::vec3(1, 1, 1));
		this->model.drawFaces();
	}
	ofPopMatrix();
}

//---------
void
Mesh::serialize(nlohmann::json&)
{

}

//---------
void
Mesh::deserialize(const nlohmann::json&)
{

}

//---------
void
Mesh::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addButton("Load...", [this]() {
		auto result = ofSystemLoadDialog("Load mesh...");
		if (!result.bSuccess) {
			return;
		}

		this->model.loadModel(result.filePath);
		if (this->model.hasMeshes()) {
			this->parameters.filename.set(std::filesystem::path(result.filePath));
		}
		});

	inspector->addEditableValue<float>(this->parameters.scale);
}