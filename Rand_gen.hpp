#ifndef _RANDOM_GEN_H_
#define _RANDOM_GEN_H_

// Wear-leveling�� ���� ������ ������ ��ȣ�� �����ϴ� ���� ���� �Լ� ����, ����

static std::random_device rand_device; //non-deterministic generator
static std::mt19937 gen(rand_device()); //to seed mersenne twister
static std::uniform_int_distribution<short> dist(0, BLOCK_PER_SECTOR - 1); //0 ~ 31 ����

inline __int8 get_rand_offset() //0 ~ 31 ���� �� ��ȯ
{
	return (__int8)dist(gen); //���� ���� �޸��� Ʈ�����Ϳ� ���� ������ ���� ��ȯ
}
#endif