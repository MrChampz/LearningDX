#pragma once

#include <D3DApp.h>
#include <D3DUtil.h>
#include <GameTimer.h>

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInst);
	InitDirect3DApp(const InitDirect3DApp& rhs) = delete;
	InitDirect3DApp& operator=(const InitDirect3DApp& rhs) = delete;
	~InitDirect3DApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& timer) override;
	virtual void Draw(const GameTimer& timer) override;
};