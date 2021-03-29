#pragma once
#ifndef BASESCENE_CLASS_DECL
#define BASESCENE_CLASS_DECL

class BaseScene
{
public:
	virtual void Setup() = 0;
	virtual void Render() = 0;
private:
};

#endif // !BASESCENE_CLASS_DECL



