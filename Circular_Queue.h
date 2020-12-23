#ifndef _CIRCULAR_QUEUE_H_
#define _CIRCULAR_QUEUE_H_

// Circular_Queue Ŭ���� ���ø� �� ���� Ŭ���� ����
// Round-Robin ����� Wear-leveling�� ���� Empty Block ��⿭, Spare Block ��⿭ ����
// Victim Block ó���� ȿ������ ���� Victim Block ��⿭ ����

struct VICTIM_BLOCK_INFO //FlashMem.cpp
{
	/***
		��ȿȭ�� ����� ��� Dynamic Table Ÿ�Կ��� ���� ���̺��� �������� �ʱ⿡ PBN�� ����
		Static Table Ÿ�Կ����� �����Ǿ� ������, ������� �ʴ´�. ����, ���������� PBN�� ����
		---
		< is_logical >
		true : victim_block_num�� �� ��� ��ȣ�� ���
		false : victim_block_num�� ���� ��� ��ȣ�� ���
	***/

	bool is_logical;
	unsigned int victim_block_num;
	float victim_block_invalid_ratio;

	void clear_all(); //Victim Block ������ ���� ���� �ʱ�ȭ
}; //Victim Block ������ ���� ��� ���� ����ü

typedef struct VICTIM_BLOCK_INFO victim_block_element;
typedef unsigned int empty_block_num;
typedef unsigned int spare_block_num;

template <typename data_type, typename element_type> // <ť�� �Ҵ� �� �ڷ���, ť�� ����� Ÿ��>
class Circular_Queue //Circular_queue.hpp
{
public:
	Circular_Queue();
	Circular_Queue(data_type queue_size);
	~Circular_Queue();

	element_type* queue_array; //���� �迭
	data_type front, rear;

	bool is_empty(); //���� ���� ����
	bool is_full(); //��ȭ ���� ����
	data_type get_count(); //ť�� ��� ���� ��ȯ

protected:
	data_type queue_size; //ť�� �Ҵ� ũ��
};


/*
init���� Spare ��� ������ ��ü ��� ũ��� �Ҵ�
bootloader�� ���� ��� ��ĵ �� �� ��Ͽ� ���Ͽ� �߰�
Static Table�� ���, ���� �߻� �� �� ��� �Ҵ� ������ �ʿ� �����Ƿ� Empty Block Queue�� ������� �ʴ´�
*/

class Empty_Block_Queue : public Circular_Queue<unsigned int, empty_block_num> //Circular_queue.hpp
{
public:
	inline Empty_Block_Queue(unsigned int queue_size) : Circular_Queue<unsigned int, empty_block_num>(queue_size) {};

	inline void print(); //���
	inline int enqueue(empty_block_num src_block_num); //Empty Block ���� �Ҵ�
	inline int dequeue(empty_block_num& dst_block_num); //Empty Block ��������
};

/***
	< Spare Block Table ���� �� ��� �ñ� >

	- ��� ���� (Static Table, Dynamic Table)
	1) FTL_write �� Overwrite�� ���� ��ȿ ������ copy�� Spare Block�� ���
	2) ���� Spare Block�� �Ϲ� ������� ���� �� ���� ���̺� �󿡼� SWAP

	- ���̺긮�� ���� (Log Algorithm, Dunamic Table)
	1) FTL_write �� PBN1�� PBN2�� ���� full_merge�߻� �� merge�������� Spare Block�� ����� ��ȿ ������ copy
	2) ���� Spare Block�� �Ϲ� ������� ���� �� ���� ���̺� �󿡼� SWAP

	- ����
	��� ������ ������ ��ȿ �����ͷ� ���� á�� ��� ������ ��ȿ �����Ϳ� ���� Overwrite�� �߻��ϸ� Overwrite�� �߻���
	�ش� ����� ���� �����͵鿡 ���� Spare Block���� copy�� ���ο� ������ ��� �� �Ϲ� ������� ����, ���� ����� Erase �� Spare Block���� ���� (SWAP)

	- GC Scheduler
	1) Log Algorithm�� ������ ���̺긮�� ���ο��� �ϳ��� �� ��� LBN�� ������ PBN1�� PBN2�� ���� Merge��������
	Spare Block�� ��ȿ ������ copy �� ���� PBN1�� PBN2 ��� Erase ����ȴ���, �ϳ��� Spare Block�� SWAP����
	=> �̿� ����, Merge�� �߻��ϸ�, GC���� Erase �߻����� ����

	2) ��� ���ο��� FTL_write������ Overwrite�� ���� ��� ��ȿȭ�� �Ͼ�ٸ�, ���� �۾��� ������ ���Ͽ�
	��ȿȭ�� ����� Victim Block���� ���� �� Spare Block�� SWAP����
	=> �̿� ����, GC�� �ش� Victim Block�� Erase �����Ͽ��� ��
***/

class Spare_Block_Queue : public Circular_Queue<unsigned int, spare_block_num> //Circular_queue.hpp
{
public:
	inline Spare_Block_Queue(unsigned int queue_size); //�ʱ� ���� �� Round-Robin ����� Wear-leveling�� ���� ���� read_index �� ���� �� ��� �� �Ҵ�

	inline void print(); //���
	inline int enqueue(spare_block_num src_block_num); //Spare Block ���� �Ҵ�

	/***
		�Ϲ� ��ϰ� Spare Block�� SWAP�� ���� �� read_index�� ���Ͽ� queue_array�� �����Ͽ� SWAP ����
		�̿� ���� �׻� �ʱ� �Ҵ� �� ��ŭ�� Spare Block�� ������.
		---
		��� Spare Block�� ���� ���ư��鼭 ����ϹǷ� Ư�� ��Ͽ��� ���� �۾��� ����Ǵ� ���� ����
	***/
	inline int dequeue(class FlashMem*& flashmem, spare_block_num& dst_block_num, unsigned int& dst_read_index); //�� Spare Block, �ش� ����� index ��������

private:
	int save_read_index(); //���� read_index �� ����
	int load_read_index(); //���� read_index �� �ҷ�����
};

class Victim_Block_Queue : public Circular_Queue<unsigned int, victim_block_element> //Circular_queue.hpp
{
public:
	inline Victim_Block_Queue(unsigned int queue_size) : Circular_Queue<unsigned int, victim_block_element>(queue_size) {};

	inline void print(); //���
	inline int enqueue(victim_block_element src_block_element); //����
	inline int dequeue(victim_block_element& dst_block_element); //����
};

#include "Circular_Queue.hpp" //Circular_Queue Ŭ���� ���ø� �� ���� Ŭ���� ��� ���Ǻ�
#endif