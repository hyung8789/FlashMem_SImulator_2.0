#ifndef _VICTIM_QUEUE_H_
#define _VICTIM_QUEUE_H_

#include "FlashMem.h"

// Victim Block Queue ������ ���� ���� ť ����

typedef struct VICTIM_BLOCK_INFO victim_element;

class Victim_Queue
{
public:
	Victim_Queue();
	Victim_Queue(unsigned int block_size);
	~Victim_Queue();
	
	victim_element* queue_array; //Victim Block�� ���� ���� �迭
	unsigned int front, rear;

	bool is_empty(); //���� ���� ����
	bool is_full(); //��ȭ ���� ����
	void print(); //���(debug)
	unsigned int get_count(); //ť�� ��� ���� ��ȯ

	int enqueue(victim_element src_element); //����
	int dequeue(victim_element& dst_element); //����

private:
	void init(unsigned int block_size); //Victim Block ������ ���� ť �ʱ�ȭ
	unsigned int queue_size; //ť�� �Ҵ� ũ��
};
#endif