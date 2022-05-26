//
//  oneTower.h
//  Example-MovingHeadAdvanced-smartyhybrid
//
//  Created by Stephan Schulz on 2022-05-26.
//

#ifndef oneTower_h
#define oneTower_h

enum ElbSleketonParts{
    ELB_HEAD = 0,
    ELB_SHOLDER_LEFT_TOP = 1,
    ELB_SHOLDER_LEFT_BOTTOM = 2,
    ELB_SHOLDER_RIGHT_TOP = 3,
    ELB_SHOLDER_RIGHT_BOTTOM = 4,
    ELB_HIP_LEFT = 5,
    ELB_HIP_RIGHT = 6,
    ELB_LEG_LEFT = 7,
    ELB_LEG_RIGHT = 8,
};

class oneTower {
private:
    
public:
        
    void addMovingHead(ElbSleketonParts part, shared_ptr<DMX::MovingHead> & ptr){
        if(movingHeads.size() >= (int)part){
            movingHeads.resize((int)part+1);
        }
        movingHeads[(int)part] = ptr;
    }
    shared_ptr<DMX::MovingHead> getMovingHead(ElbSleketonParts part){
        if((int)part < movingHeads.size()){
            return movingHeads[(int)part];
        }
        return nullptr;
    }
    
    vector<shared_ptr<DMX::MovingHead>> movingHeads;
    
    
    
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
