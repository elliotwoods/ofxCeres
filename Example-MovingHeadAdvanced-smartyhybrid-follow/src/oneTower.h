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
    
//    map<string, shared_ptr<MovingHead>> movingHeads;
    vector<shared_ptr<DMX::MovingHead>> movingHeads; //& getMovingHeads();
    
//    shared_ptr<MovingHead> & movingHead_head;
    void setup(){
        
    }
};
#endif /* oneTower_h */
