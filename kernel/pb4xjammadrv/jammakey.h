
#pragma once


#define KEY_SETTING      0x4c // setting but
#define KEY_CREDIT       0x4b // credit sign
#define KEY_1P_START     0x4a // 1p start   
#define KEY_1P_UP        0x49 // 1p up      
#define KEY_1P_DOWN      0x48 // 1p down    
#define KEY_1P_LEFT      0x47 // 1p left    
#define KEY_1P_RIGHT     0x46 // 1p right   
#define KEY_1P_FIRE_1    0x45 // 1p fire 1  
#define KEY_1P_FIRE_2    0x44 // 1p fire 2  
#define KEY_1P_FIRE_3    0x43 // 1p fire 3  
#define KEY_1P_FIRE_4    0x42 // 1p fire 4  
#define KEY_1P_FIRE_5    0x41 // 1p fire 5  
#define KEY_1P_FIRE_6    0x40 // 1p fire 6  
#define KEY_2P_START     0x80 // 2p start   
#define KEY_2P_UP        0x81 // 2p up      
#define KEY_2P_DOWN      0x82 // 2p down    
#define KEY_2P_LEFT      0x83 // 2p left    
#define KEY_2P_RIGHT     0x84 // 2p right   
#define KEY_2P_FIRE_1    0x8a // 2p fire 1  
#define KEY_2P_FIRE_2    0x89 // 2p fire 2  
#define KEY_2P_FIRE_3    0x88 // 2p fire 3  
#define KEY_2P_FIRE_4    0x87 // 2p fire 4  
#define KEY_2P_FIRE_5    0x86 // 2p fire 5  
#define KEY_2P_FIRE_6    0x85 // 2p fire 6  


#define IS_KEYDOWN(key) ( readKey(key)==0 )
int initmem();
int uninitmem();
int readKey(int key_code);
bool checkTimeSleep(int i);
