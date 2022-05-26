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
//    map<string, shared_ptr<MovingHead>> movingHeads;
    map<string, shared_ptr<DMX::MovingHead>> movingHeads; //& getMovingHeads();
    
    shared_ptr<MovingHead> & movingHead_head;
    void setup(){
        
    }
};
#endif /* oneTower_h */
