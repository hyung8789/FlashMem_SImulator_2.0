#include "Random_gen.h"

// Wear-leveling�� ���� ������ ������ ��ȣ�� �����ϴ� ���� ���� �Լ� ����

__int8 get_rand_offset() //0 ~ 31 ���� �� ��ȯ
{ 
	return dist(gen); //���� ���� �޸��� Ʈ�����Ϳ� ���� ������ ���� ��ȯ
}