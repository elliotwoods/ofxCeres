//
//  oneTower.h
//  Example-MovingHeadAdvanced-smartyhybrid
//
//  Created by Stephan Schulz on 2022-05-26.
//

#ifndef oneTower_h
#define oneTower_h

class oneTower {
private:
    
public:
    
    int towerID = -1;
    string towerLabel = "?";
    ofColor towerColor;
    ofPoint towerPosition;
    
    void setup(int _id, string _label){
        towerID = _id;
        towerLabel = _label;
        towerColor = ofColor((_id*23+311)%200, (_id*41+431)%200, (_id*33+197)%200);
        
    }
    
    void draw(){
        ofPushStyle();
        
        float temp_x = cos(ofGetElapsedTimef()*.5);
        temp_x *= 2*(towerID+1);
        float temp_z = sin(ofGetElapsedTimef()*.5);
        temp_z *= 4;
        ofPoint spherePosition = ofPoint(temp_x, 5, temp_z);
        ofSetColor(towerColor);
        ofDrawSphere(spherePosition, 0.3);
//        ofLog()<<towerID<<" x "<<temp_x<<" y "<<temp_y;
        towerPosition = ofPoint(temp_x,6,temp_z);
        
        ofxCvGui::Utils::drawTextAnnotation(towerLabel, towerPosition, towerColor);
        ofDrawLine(towerPosition, spherePosition);
        ofPopStyle();
    }
};
#endif /* oneTower_h */
