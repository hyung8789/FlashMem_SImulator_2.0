#ifndef _UTILS_H_
#define _UTILS_H_

// Wear-leveling�� ���� ������ ������ ��ȣ�� �����ϴ� ���� ���� �Լ� ����, ����
// ��Ʈ ��� �Լ� ����, ����

static std::random_device rand_device; //non-deterministic generator
static std::mt19937 gen(rand_device()); //to seed mersenne twister
static std::uniform_int_distribution<short> dist(0, BLOCK_PER_SECTOR - 1); //0 ~ 31 ����

inline __int8 get_rand_offset() //0 ~ 31 ���� �� ��ȯ
{
	return (__int8)dist(gen); //���� ���� �޸��� Ʈ�����Ϳ� ���� ������ ���� ��ȯ
}

inline void bit_disp(const char src_data, int MSB_digits, int LSB_digits)
{
	for (int i = MSB_digits; i >= LSB_digits; i--)
		printf("%1d", (src_data) & (0x1 << (i)) ? 1 : 0);
	printf("\n");

	return;
}
#endif