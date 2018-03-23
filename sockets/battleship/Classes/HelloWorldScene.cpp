#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

#define TILE_COUNT 10
#define TILE_SIZE 40

#define SKT_PATH "socket"
#define BUF_SIZE 1024
#define BACKLOG 5
#define DATA "The sea is calm tonight, the tide is full . . ."

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

const int rows = TILE_COUNT;
const int columns = TILE_COUNT;
const int maxSelectedTiles = 2; //maximum ship tiles that can be placed for both players
int countForP1 = 0, countForP2 = 0, selectedForP1 = 0, selectedForP2 = 0;
MenuItemLabel* beginGameBtn;
Color3B inactiveColor = Color3B(125, 125, 125);
Color3B activeColor = Color3B(190, 255, 255);
Menu* menu;
Vector<Button*> selectedTagsForP1;
Vector<Button*> selectedTagsForP2;
Vector<Button*> attackedTilesForP1;
Vector<Button*> attackedTilesForP2;
string player_turn = "Player 1"; //The one who starts to launch his attack on the opponent's battle ship
Label* player_turn_Lbl;
Label* p1_LeftShips;
Label* p2_LeftShips;

auto visibleSize = Size(0, 0);
Vec2 origin = Vec2(0, 0);

Label* HelloWorld::createLabel(string title, Vec2 position, Color3B color) {
    auto label = Label::createWithTTF(title, "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(position);
        
        label->setColor(color);
        
        // add the label as a child to this layer
        this->addChild(label, 0);
    }
    return label;
}

void HelloWorld::reloadTileSet(int& count, Vector<Button*>tileSet) {
    for (int i=0;i<tileSet.size();i++) {
        Button* btn = tileSet.at(i);
        btn->setName("normal");
        btn->loadTextureNormal("tile.png");
    }
    count = 0;
}

void HelloWorld::restoreTileSet(Vector<Button*>& tileSet) {
    for (int i=0; i < tileSet.size(); i++) {
        auto btn = (Button*)tileSet.at(i);
        btn->setEnabled(true);
        tileSet.eraseObject(btn);
        i--;
    }
}

// this is required for enumerating each tile and defining unique tag

void HelloWorld::loadBattleArea(Button* btn, int& countForP, Vec2 position) {
    for(int i=0;i<rows;i++) {
        for(int j=0;j<columns;j++) {
            btn->setPosition(Vec2(position.x + j*TILE_SIZE, position.y + i*TILE_SIZE));
            btn->setTag(countForP++);
            btn->setName("normal");
            this->addChild(btn->clone());
        }
    }
}

void HelloWorld::showMap() {
    
    // we create button once which is meant to be a tile for player's area for placing his ships
    
    auto button = Button::create("tile.png", "tile_hovered.png", "tile_disabled.png");
    
    button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
        Button* btn = (Button*)sender;
        switch (type)
        {
            case Widget::TouchEventType::BEGAN:
                break;
            case Widget::TouchEventType::ENDED: {
                
//                requestFromClient(, );
                /* ------------ This condition is for placing ships for each player ------------ */
                
                int tag = btn->getTag();
                
                if(beginGameBtn->isVisible() && beginGameBtn->getString() != "Restart the Game") {
                    
                    /* tag that is between 0 and TILE_COUNT*TILE_COUNT-1
                     Ex: if TILE_COUNT = 10, then TILE_COUNT*TILE_COUNT = 100;
                     so the condition between 0 and 99, for the second player the value varies between 100 and 199; for each 'tile', we use different tag*/
                    
                    if(tag >=0 && tag < countForP1) {
                        shouldStartGame(selectedForP1, btn, selectedTagsForP1);
                    } else {
                    
                    /* tag that is between TILE_COUNT*TILE_COUNT and TILE_COUNT*TILE_COUNT*2-1 */
                        
                        shouldStartGame(selectedForP2, btn, selectedTagsForP2);
                    }
                } else {
                    
                /* ------------ This condition is for attacking ships for each player ------------ */
                    
                    if(selectedTagsForP1.size() != 0 && selectedTagsForP2.size() != 0) {
                        
                        // this one is for making tile non-clickable, once the target is attacked, it can't be touched once again
                        
                        btn->setEnabled(false);
                        
                        if (player_turn == "Player 1") {
                            
                            // after each tile is shot, not missed, then the vector containg player's predefined tiles (the ones that were placed before game starts) will shrink down
                            
                            // This condition is to ensure that in case the player attacks his own area, no effects will be applied
                            if(tag >=0 && tag < countForP1) {
                                btn->setEnabled(true);
                            } else {
                                player_turn = "Player 2";
                
    /*SOCKET USAGE OVER HERE: Every time the player shots at his opponent's area, it will make the request to the server, follow on the function to see it in action*/
                                
                                requestFromClient(btn->getTag());
                            }
                            
                            if(selectedTagsForP2.contains(btn)) {
                                selectedTagsForP2.eraseObject(btn);
                            }
                        } else {
                            
                            if(tag >=countForP1 && tag < countForP2) {
                                btn->setEnabled(true);
                            } else {
                                player_turn = "Player 1";
                                requestFromClient(btn->getTag());
                            }
                            
                            if(selectedTagsForP1.contains(btn)) {
                                selectedTagsForP1.eraseObject(btn);
                            }
                        }
                        
                        updateLeftShips();
                    }
                    
                    // Shows the winner
                    if(selectedTagsForP1.size() == 0 || selectedTagsForP2.size() == 0) {
                        int winnerID = 1;
                        if(selectedTagsForP1.size() == 0) {
                            winnerID = 2;
                        }
                        stringstream sstm;
                        sstm << "Player "<<winnerID<<" wins!!!";
                        player_turn_Lbl->setString(sstm.str());
                        
                        // Next time the game restarts, the loser will make his move first
                        
                        if(winnerID == 1)
                            player_turn = "Player 2";
                        else
                            player_turn = "Player 1";
                        
                        beginGameBtn->setString("Restart the Game");
                        beginGameBtn->setVisible(true);
                    }
                }
            }
                break;
            default:
                break;
        }
    });
    
    loadBattleArea(button, countForP1, Vec2(visibleSize.width/2 + 80, visibleSize.height/2 + 200));
    
    // last tile is required for placing Player 2's area, beginning from the origin.X+size.Width + some indentation, to make the design of the game looking better
    
    Button* lastTileForPlayer1 = (Button*)this->getChildByTag(countForP1-1);
    countForP2 = countForP1;
    
    // tempBtn serves to make the game design better
    
    Button* tempBtn = (Button*)this->getChildByTag(countForP1/2-1 + 5);
    
    auto player1Lbl = Label::createWithTTF("Player 1", "fonts/Marker Felt.ttf", 24);
    if (player1Lbl == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        player1Lbl->setPosition(Vec2(visibleSize.width/2 + tempBtn->getPosition().x + tempBtn->getContentSize().width/2, visibleSize.height/2 + lastTileForPlayer1->getPosition().y + lastTileForPlayer1->getContentSize().height + 20));
        
        // add the label as a child to this layer
        this->addChild(player1Lbl, 1);
    }
    
    loadBattleArea(button, countForP2, Vec2(visibleSize.width/2 + lastTileForPlayer1->getPosition().x + lastTileForPlayer1->getContentSize().width + 100, visibleSize.height/2 + 200));
    
    tempBtn = (Button*)this->getChildByTag(countForP2/2-1 + 5);
    
    auto player2Lbl = Label::createWithTTF("Player 2", "fonts/Marker Felt.ttf", 24);
    if (player2Lbl == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        player2Lbl->setPosition(Vec2(visibleSize.width/2 + tempBtn->getPosition().x + tempBtn->getContentSize().width/2, visibleSize.height/2 + lastTileForPlayer1->getPosition().y + lastTileForPlayer1->getContentSize().height + 20));
        
        // add the label as a child to this layer
        this->addChild(player2Lbl, 1);
    }
    
    beginGameBtn = MenuItemLabel::create(Label::createWithTTF("Start Game", "fonts/Marker Felt.ttf", 24), CC_CALLBACK_1(HelloWorld::startGame, this));
    
    if (beginGameBtn == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        tempBtn = (Button*)this->getChildByTag(0);
        beginGameBtn->setEnabled(false);
        beginGameBtn->setColor(inactiveColor);
        beginGameBtn->setPosition(Vec2(500, tempBtn->getPosition().y - 70));
    }

}

void HelloWorld::startGame(Ref* pSender) {
    
    if(beginGameBtn->getString() != "Restart the Game") {
        reloadTileSet(selectedForP1, selectedTagsForP1);
        reloadTileSet(selectedForP2, selectedTagsForP2);
        
        //Hide Start Button
        beginGameBtn->setVisible(false);
        if(!player_turn_Lbl) {
            player_turn_Lbl = createLabel(player_turn+" turn to make attack", Vec2(origin.x + visibleSize.width/2+500, 700), Color3B(255, 255, 0));
            Button* tempBtn = (Button*)this->getChildByTag(TILE_COUNT/2);
            p1_LeftShips = createLabel("", Vec2(tempBtn->getPosition().x - tempBtn->getContentSize().width/2, tempBtn->getPosition().y - 40), Color3B(0, 255, 255));
            tempBtn = (Button*)this->getChildByTag(countForP2-(TILE_COUNT*TILE_COUNT)+5);
            p2_LeftShips = createLabel("", Vec2(tempBtn->getPosition().x - tempBtn->getContentSize().width/2, tempBtn->getPosition().y - 40), Color3B(0, 255, 255));
            // tempBtn serves to make the game design better
        } else {
            player_turn_Lbl->setVisible(true);
            p1_LeftShips->setVisible(true);
            p2_LeftShips->setVisible(true);
        }
        
        updateLeftShips();
    } else {
        // Restart the Game
        p1_LeftShips->setVisible(false);
        p2_LeftShips->setVisible(false);
        player_turn_Lbl->setVisible(false);
        selectedTagsForP1.clear();
        selectedTagsForP2.clear();
        restoreTileSet(attackedTilesForP1);
        restoreTileSet(attackedTilesForP2);
        beginGameBtn->setString("Start Game");
        beginGameBtn->setEnabled(false);
        beginGameBtn->setColor(inactiveColor);
    }
    
}


void HelloWorld::shouldStartGame(int& count, Button* btn, Vector<Button*>& tileSet) {
    
    // these conditions are required to change the state of the button between 'normal' and 'clicked', if the player clicks the button that was clicked before, it will be changed to 'normal' again
    
    if(count < maxSelectedTiles && btn->getName() == "normal") {
        btn->loadTextureNormal("tile_clicked.png");
        btn->setName("clicked");
        tileSet.pushBack(btn);
        count++;
    } else {
        btn->loadTextureNormal("tile.png");
        btn->setName("normal");
        if(tileSet.contains(btn)) {
            tileSet.eraseObject(btn);
        }
        count--;
    }
    
    if (selectedForP1 == maxSelectedTiles && selectedForP2 == maxSelectedTiles) {
        beginGameBtn->setEnabled(true);
        beginGameBtn->setColor(activeColor);
    } else {
        beginGameBtn->setEnabled(false);
        beginGameBtn->setColor(inactiveColor);
    }
}

void HelloWorld::updateLeftShips() {
    stringstream sstm;
    sstm << "Ships left: " << selectedTagsForP1.size();
    p1_LeftShips->setString(sstm.str());
    sstm.clear();
    sstm.str("");
    sstm << "Ships left: " << selectedTagsForP2.size();
    p2_LeftShips->setString(sstm.str());
    sstm.clear();
    sstm.str("");
    
    //Updates the text label for a player's turn
    player_turn_Lbl->setString(player_turn+" turn to make attack");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    HelloWorld::showMap();
    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, beginGameBtn, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    auto label = Label::createWithTTF("Battle Ship", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));
        
        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    return true;
}

void HelloWorld::error(char *msg)
{
    perror(msg);
    exit(0);
}

/* ------ SOCKET IMPLEMENTATION: THE REQUEST FROM CLIENT TO SERVER ------ 
 It will send the clicked tile to server, and the server will return the tag of the clicked tile back, so on response it can destroy tile*/

void HelloWorld::requestFromClient(int clickedTile) {
    
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    char buffer[256];
    
    //default PORT NUMBER = 4547
    
    portno = 4547;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }
    
    server = gethostbyname("localhost");
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }
    
    bzero(buffer, 256);
    snprintf(buffer, sizeof clickedTile, "%d", clickedTile);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        error("ERROR writing to socket");
    } else {
        log(n);
    }
    
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
    {
        error("ERROR reading from socket");
    }
    printf("Destroy tile # %s\n", buffer);
    
    int tag = atoi(buffer);
    Button *btn = (Button *)this->getChildByTag(tag);
    if(tag >=0 && tag < countForP1) {
        attackedTilesForP1.pushBack(btn);
    } else {
        attackedTilesForP2.pushBack(btn);
    }
    
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
