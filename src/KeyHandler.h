#pragma once

#include <GLFW/glfw3.h>

const int NUM_KEYS = GLFW_KEY_LAST + 2;	// Need offset of 1 (UNKOWN = -1) and 1 additional Space to allow last key

class KeyHandler
{
public:
	KeyHandler()
	{
		for (int i = 0; i < NUM_KEYS; i++)
		{
			mKeyDown[i] = false;
			mKeyPressed[i] = false;
			mKeyReleased[i] = false;
		}
	}

	~KeyHandler()
	{
	}

	void KeyPressed(int key)
	{
		key++;
		mKeyPressed[key] = true;
		mKeyDown[key] = true;
		mKeyReleased[key] = false;
	}

	void KeyReleased(int key)
	{
		key++;
		mKeyPressed[key] = false;
		mKeyDown[key] = false;
		mKeyReleased[key] = true;
	}

	void FrameUpdate()
	{
		for (int i = 0; i < NUM_KEYS; i++)
		{
			mKeyReleased[i] = false;
			mKeyPressed[i] = false;
		}
	}

	bool IsKeyDown(int key)
	{
		return mKeyDown[key + 1];
	}

	bool WasKeyReleased(int key)
	{
		return mKeyReleased[key + 1];
	}

	bool WasKeyPressed(int key)
	{
		return mKeyPressed[key + 1];
	}

protected:
	bool mKeyDown[NUM_KEYS];
	bool mKeyReleased[NUM_KEYS];
	bool mKeyPressed[NUM_KEYS];
};
