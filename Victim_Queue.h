#ifndef _VICTIM_QUEUE_H_
#define _VICTIM_QUEUE_H_

#include "FlashMem.h"

// Victim Block Queue ������ ���� ���� ť ����

/*** Build Option ***/
/***
	Victim Block Queue�� ũ��� Spare Block ������ �ٸ��� �� ��� Round-Robin Based Wear-leveling�� ����
	������ Victim Block��� Spare Block ���� ũ���� Spare Block Table�� ���� SWAP �߻� �ÿ�
	Spare Block Table�� ��� Spare Block���� GC�� ���� ó������ �ʾ��� ���, GC�� ���� ������ ó���ϵ��� �ؾ� ��
	=> ��������
	---
	Victim Block Queue�� ũ��� Spare Block ������ ���� �����μ�, Victim Block Queue�� ���¿� ���� ó���� ����
***/
#define VICTIM_BLOCK_QUEUE_RATIO 0.08 //Victim Block ť�� ũ�⸦ ������ �÷��� �޸��� ��ü ��� ������ ���� ���� ũ��� ����

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
	void print(); //����ť ��� �Լ�(debug)
	unsigned int get_count(); //ť�� �����ϴ� ����� ������ ��ȯ

	int enqueue(victim_element src_data); //���� �Լ�
	int dequeue(victim_element& dst_data); //���� �Լ�

private:
	void init(unsigned int block_size); //Victim Block ������ ���� ť �ʱ�ȭ
	unsigned int queue_size; //ť�� �Ҵ� ũ��
};
#endif