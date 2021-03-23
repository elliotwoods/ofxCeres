#pragma once

#include "System.h"
#include <string>
#include <vector>

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			namespace Builder {
				struct ChainJoint {
					std::string jointNameThis;
					std::string jointNamePrevious;
				};

				struct ChainBody {
					std::string bodyType;
					std::string bodyName;
					std::vector<ChainJoint> jointConnections;
					std::vector<std::string> groundSupports;
					glm::quat orientation = glm::quat(1, 0, 0, 0);
				};

				typedef std::vector<ChainBody> Chain;
				class FactoryRegister : public std::map<std::string, function<shared_ptr<System::Body>()>> {
				public:
					template<typename T>
					void addFactory(const string & name) {
						auto factoryFunction = []() {
							auto typed = make_shared<T>();
							auto untyped = static_pointer_cast<System::Body>(typed);
							return untyped;
						};
						this->insert({ name, factoryFunction });
					}
				};

				void build(System & system
					, Chain & chain
					, map<string, function<shared_ptr<System::Body> ()>> factories);
			}
		}
	}
}