#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include "FlashMem.h"

// Garbage Collector�� ���� Victim Block ������ ���� ���� ����(���� ť) ����
/*
* �ϴ� ���Ⱑ �߻��� ��Ͽ� ���Ͽ� vq�� ������ �߰��ϰ� gc�� ���� �˻��ؼ� 
���� �Ӱ谪�� ���� ó���ϵ��� �ϴ°� ��⿭ ť, Victim Block ť�� ���� �����ϴ� �ͺ��� ������ ����
--
ftl write
�ϴ� �۾� �߻��� ��Ͽ� ���ؼ� ������ vq�� �߰�
���� end success �ܰ迡�� gc�� ���� �߰��� ��ϵ鿡 ���� ��ȿ�� ��� �� ���� �Ӱ谪�� ���� victim���� ���� �� �ʿ䰡 ���� ��� vq���� ����
*/

typedef struct VICTIM_BLOCK_INFO victim_element;

class Ring_Buffer
{
public:
	Ring_Buffer();
	Ring_Buffer(unsigned int block_size);
	~Ring_Buffer();

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