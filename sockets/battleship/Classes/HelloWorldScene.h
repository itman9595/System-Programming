#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "iostream"

#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

using namespace std;
using namespace cocos2d;
using namespace ui;

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void error(char *msg);
    
    void requestFromClient(int clickedTile = 0);

    Label* createLabel(string title, Vec2 position, Color3B color);
    
    void loadBattleArea(Button* btn, int& countForP, Vec2 position);
    
    void reloadTileSet(int& count, Vector<Button*>tileSet);
    
    void restoreTileSet(Vector<Button*>& tileSet);
    
    void showMap();
    
    void startGame(Ref* pSender);
    
    void shouldStartGame(int& count, Button* btn, Vector<Button*>& tileSet);
    
    void updateLeftShips();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__
