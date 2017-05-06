#include "MainScene.h"
#include <random>

USING_NS_CC;

Scene* MainScene::createScene()
{
	auto scene = Scene::create();

	auto layer = MainScene::create();

	scene->addChild(layer);
	
	return scene;
}

bool MainScene::init()
{
	if( !Layer::init() )
		return false;


	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	auto label = Label::createWithTTF("世界,你好", "fonts/SIMLI.TTF", 24);
	label->setColor(Color3B::RED);
	label->setPosition(Vec2(origin.x + visibleSize.width / 2,
		origin.y + visibleSize.height - label->getContentSize().height));
	this->addChild(label,0);

	std::random_device rd;						//采用非确定性随机数发生器产生随机数种子
	std::default_random_engine gen(rd());		//采用默认随机数引擎产生随机数
	std::uniform_int_distribution<> unif_x(origin.x, origin.x + visibleSize.width);		//采用整数均匀分布器产生x均匀分布的随机数
	std::uniform_int_distribution<> unif_y(origin.y, origin.y + visibleSize.height);		//采用整数均匀分布器产生y均匀分布的随机数

	for (int i = 0; i < 5; i++)
	{
		Airplane* plane;
		plane = Airplane::createPlane("Picture/airplane.png");
		plane->setScale(0.1, 0.1);
		plane->setPosition(unif_x(gen), unif_y(gen));
		this->addChild(plane);
		this->my_planes.push_back(plane);
	}

	for (int i = 0; i < 5; i++)
	{
		Airplane* plane;
		plane = Airplane::createPlane("Picture/airplane_red.png");
		plane->setScale(0.1, 0.1);
		plane->setPosition(unif_x(gen), unif_y(gen));
		this->addChild(plane);
		this->enemy_planes.push_back(plane);
	}
	

	this->schedule(schedule_selector(MainScene::update));


	auto listen = EventListenerTouchOneByOne::create();
	listen->onTouchBegan = CC_CALLBACK_2(MainScene::onTouchBegan, this);
	listen->onTouchMoved = CC_CALLBACK_2(MainScene::onTouchMoved, this);
	listen->onTouchEnded = CC_CALLBACK_2(MainScene::onTouchEnded, this);
	listen->setSwallowTouches(true);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listen, this);

	return true;

}

void MainScene::update(float f)
{
	for (auto my_plane : this->my_planes)
		if (my_plane->isActive())
			my_plane->update();

	//log("touch point:%f,%f",touchPoint.x,touchPoint.y);
}

bool MainScene::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event*)
{
	
	Point touch = pTouch->getLocation();//返回点击的位置
	this->touchPoint = touch;

	if (state == 0)
	{
		for (auto my_plane : this->my_planes)
			if (my_plane->getBoundingBox().containsPoint(touch))
			{
				my_plane->select();
				this->state = 1;
				return true;
			}
	}

	if (this->state == 1)
	{
		for (auto my_plane : my_planes)
			if (my_plane->isSelected())
			{
				my_plane->setDest(touch);
				my_plane->activate();
			}
		this->state = 0;
	}
	else
	{
		this->state = 2;
		this->mouse_rect = DrawNode::create();
		this->addChild(this->mouse_rect, 2);
	}

	return true;
}

void MainScene::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touch = pTouch->getLocation();//返回点击的位置  

	//
	if (this->state == 2)
	{
		this->mouse_rect->clear();
		Vec2 mouse_rect_points[4];
		mouse_rect_points[0] = this->touchPoint;
		mouse_rect_points[1] = Vec2(this->touchPoint.x, touch.y);
		mouse_rect_points[2] = touch;
		mouse_rect_points[3] = Vec2(touch.x, this->touchPoint.y);

		//绘制空心多边形
		//填充颜色：Color4F(1, 0, 0, 0), 透明
		//轮廓颜色：Color4F(0, 1, 0, 1), 绿色
		this->mouse_rect->drawPolygon(mouse_rect_points, 4, Color4F(1, 0, 0, 0), 1, Color4F(0, 1, 0, 1));
		//
	}
}

void MainScene::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	if (this->state == 2)
	{
		this->removeChild(this->mouse_rect);
		this->state = 1;

		Point touch = pTouch->getLocation();//返回点击的位置  

		log("Last touch: (%f, %f), Touch: (%f, %f)", this->touchPoint.x, this->touchPoint.y, touch.x, touch.y);
		/*float min_x = (this->touchPoint.x < touch.x) ? (this->touchPoint.x) : (touch.x);
		float max_x = (this->touchPoint.x >= touch.x) ? (this->touchPoint.x) : (touch.x);
		float min_y = (this->touchPoint.y < touch.y) ? (this->touchPoint.y) : (touch.y);
		float max_y = (this->touchPoint.y >= touch.y) ? (this->touchPoint.y) : (touch.y);
		Rect select_rect{ min_x, min_y, max_x - min_x, max_y - min_y};*/
		Rect select_rect{ MIN(this->touchPoint.x, touch.x), MIN(this->touchPoint.y, touch.y), abs(this->touchPoint.x - touch.x), abs(this->touchPoint.y - touch.y) };
		log("Select Rect: O(%f, %f), width %f, height %f", select_rect.origin.x, select_rect.origin.y, select_rect.size.width, select_rect.size.height);
	
		for (auto my_plane : my_planes)
		{
			log("Plane Position: (%f, %f)", my_plane->getPosition().x, my_plane->getPosition().y);
			log("Plane Rect: O(%f, %f), width %f, height %f", my_plane->getBoundingBox().origin.x, my_plane->getBoundingBox().origin.y, my_plane->getBoundingBox().size.width, my_plane->getBoundingBox().size.height);
			if (select_rect.containsPoint(my_plane->getPosition()))
				my_plane->select();
		}
	}
}

void Airplane::update()
{
	if (selected)
		if ((dest - getPosition()).getLength() < 10)
			active = 0;
		else
		{
			active = 1;
			auto esp = (dest - getPosition()).getNormalized();
			setPosition(getPosition() + esp * move_speed);
		}
}