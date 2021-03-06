#pragma once

#define SERVER

#include "Unit.h"
#include "SocketServer.h"
#include "SocketClient.h"

class ControlPanel;

class MouseRect : public cocos2d::DrawNode
{
public:
	CREATE_FUNC(MouseRect);
	cocos2d::Point touch_start, touch_end;
	cocos2d::Point start, end;
	void update(float f) override;
};

class BattleScene : public cocos2d::Layer
{
public:
	GameMessageSet msg_set;

	void win();
	void lose();

	static cocos2d::Scene* createScene(SocketClient* _socket_client, SocketServer* _socket_server = nullptr);
	virtual bool init(SocketClient* _socket_client, SocketServer* _socket_server);
	void update(float f) override;
	bool onTouchBegan(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchMoved(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchEnded(cocos2d::Touch*, cocos2d::Event*)override;

	void scrollMap();

	void focusOnBase();


	void onKeyPressed(cocos2d::EventKeyboard::KeyCode, cocos2d::Event *) override;


	static BattleScene* create(SocketClient* _socket_client, SocketServer* _socket_server);

private:
	void create_figher(Ref*);


	int player_id = 0;

	unsigned long long frame_cnt = 0;


	SocketServer* socket_server = nullptr;
	SocketClient* socket_client = nullptr;

	int start_flag = 0;

	cocos2d::TMXTiledMap* battle_map = nullptr;
	GridMap* grid_map = nullptr;
	UnitManager* unit_manager = nullptr;
	ControlPanel* control_panel_;

	MouseRect* mouse_rect = nullptr;

	cocos2d::Point last_touch{0, 0};
	cocos2d::Point crusor_position{0, 0};

	void initPlayerID();

	//EventListenerMouse* _mouseListener;
};

class ControlPanel:public cocos2d::Menu
{
public:
	CREATE_FUNC(ControlPanel);
	bool init() override;
	void setFighterCallback(std::function<void(Ref*)>);

	void setTankCallback(std::function<void(Ref*)>);
	void setSoldierCallback(std::function<void(Ref*)>);
private:
	cocos2d::MenuItemImage *fighter;
	cocos2d::MenuItemImage *tank;
	cocos2d::MenuItemImage *soldier;

};