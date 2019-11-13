#include "pch_ofApp.h"
#include "Block.h"

namespace Data {
	//----------
	BaseBlock::BaseBlock() {
		this->loads.insert({
			{
				"weight",
				{
					{0.5, 0.15, 0}
					, { 0, -9.81 * 100, 0}
				}
			}
		});

		this->joints.insert({
			{
				"bearingTop",
				{
					{0, topBearingHeight, 0}
				}
			},
			{
				"bearingBottom",
				{
					{0, bottomBearingHeight, 0}
				}
			}
		});

		this->onDraw += [this]() {
			ofPushStyle();
			{
				ofSetColor(this->getColor());
				ofNoFill();
				ofDrawBox({0.5, 0.15, 0}, 1.5, 0.3, 0.3);
			}
			ofPopStyle();
		};
	}

	//----------
	Block::Block() {
		this->joints.insert({
			{
				"mountUpBottom",
				{
					{1.0, 0, 0}
				}
			},
			{
				"mountUpTop",
				{
					{1.0, 0.3, 0}
				}
			}
			});
	}

	//----------
	TopBlock::TopBlock() {
		this->joints.insert({
			{
				"bearing2Top",
				{
					{1.0, topBearingHeight, 0}
				}
			},
			{
				"bearing2Bottom",
				{
					{1.0, bottomBearingHeight, 0}
				}
			}
			});
	}

	//----------
	Axle::Axle() {
		this->joints.insert({
			{
				"upTop",
				{
					{0, topBearingHeight, 0}
				}
			},
			{
				"upBottom",
				{
					{0, bottomBearingHeight, 0}
				}
			},
			{
				"downTop",
				{
					{0, -axleGap, 0}
				}
			},
			{
				"downBottom",
				{
					{0, -axleGap -0.3, 0}
				}
			}
			});

		this->onDraw += [this]() {
			ofPushStyle();
			{
				ofSetColor(this->getColor());
				ofNoFill();
				ofSetCircleResolution(4);
				ofDrawCylinder(glm::vec3{ 0, -axleGap / 2.0f, 0 }, 0.1f, 0.3 * 2 + axleGap);
			}
			ofPopStyle();
		};
	}
}