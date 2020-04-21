#include <utility>

struct anim {
    std::pair<int, int>
    idle = {0,7}, 
    run = {8, 17}, 
    attack1 = {18, 23}, 
    attack2 = {24, 29}, 
    attack3 = {30, 37}, 
    jump = {38, 40}, 
    fall = {41, 44}, 
    hurt = {45, 47}, 
    death = {48, 57}, 
    blockIdle = {58, 65}, 
    block = {66, 70}, 
    roll = {71, 79}, 
    ledgeGrab = {80, 84}, 
    wallSlide = {85, 89};
};