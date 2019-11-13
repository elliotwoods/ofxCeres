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

				struct ChainLink {
					std::string bodyType;
					std::string bodyName;
					std::vector<ChainJoint> jointConnections;
					std::vector<std::string> groundSupports;
				};

				void build(System & system
					, vector<ChainLink> & chainLinks
					, map<string, function<shared_ptr<System::Body> ()>> factories);
			}
		}
	}
}