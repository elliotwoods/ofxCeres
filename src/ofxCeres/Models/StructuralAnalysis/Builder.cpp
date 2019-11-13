#include "pch_ofxCeres.h"
#include "Builder.h"

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			namespace Builder {
				//----------
				void build(System & system
					, vector<ChainLink> & chainLinks
					, map<string, function<shared_ptr<System::Body> ()>> factories) {
					for (const auto & chainLink : chainLinks) {
						auto nextBody = factories[chainLink.bodyType]();

					}
				}
			}
		}
	}
}