#pragma once

class ResourceFactory
{
public:


private:


#pragma region �V���O���g��
public:
	static ResourceFactory& GetInstance()
	{
		static ResourceFactory instance;
		return instance;
	}

private:
	ResourceFactory() {}

#pragma endregion

};

#define ResFac ResourceFactory::GetInstance()