#include "SquarePolygon.h"

void SquarePolygon::Init(float w, float h, DWORD color)
{
	// ���_���W
	m_vertex[0].pos = { -w/2,-h/2,0 };
	m_vertex[1].pos = { -w/2, h/2,0 };
	m_vertex[2].pos = {  w/2,-h/2,0 };
	m_vertex[3].pos = {  w/2, h/2,0 };
	// ���_�F
	m_vertex[0].color = color;
	m_vertex[1].color = color;
	m_vertex[2].color = color;
	m_vertex[3].color = color;
	// UV���W
	m_vertex[0].uv = { 0,1 };
	m_vertex[1].uv = { 0,0 };
	m_vertex[2].uv = { 1,1 };
	m_vertex[3].uv = { 1,0 };
}

void SquarePolygon::Draw()
{
}
