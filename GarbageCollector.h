#ifndef _GARBAGECOLLECTOR_H_
#define _GARBAGECOLLECTOR_H_

// Garbage Collecter ������ ���� Ŭ���� ����

#include "FlashMem.h"
#include "Victim_Queue.h"

/***
	< gc_lazy_mode >
	���ӵ� ���� �۾��� ���� Performance ����� ���Ͽ�
	�����ٷ� ȣ�� �� Victim Block ť�� ���� �� ������ �ƹ��� �۾��� �������� �ʴ´�.
	�̴� ��� ������ ������ �ÿ� �׻� ��� ���� Ȯ���� ���� Victim Block�� �����Ǵ� ��� ó���ϵ��� ��Ȱ����Ų��.
	---
	true : ��� (Default)
	false : ������� ����
***/

class GarbageCollector //GarbageCollector.cpp
{
public:
	GarbageCollector();
	~GarbageCollector();

	bool RDY_terminate; //���� ��� ����
	bool RDY_v_flash_info_for_set_invalid_ratio_threshold; //��ȿ�� �Ӱ谪 ������ ���� ������ �÷��� �޸� ���� �غ� ����
	bool gc_lazy_mode;

	void print_invalid_ratio_threshold();
	int scheduler(class FlashMem** flashmem, int mapping_method); //main scheduling function for GC

private: //Execution by scheduling function
	float invalid_ratio_threshold; //Victim Block ���� ���� ����� ��ȿ�� �Ӱ谪

	int one_dequeue_job(class FlashMem** flashmem, int mapping_method); //Victim Block ť�κ��� �ϳ��� Victim Block�� ���ͼ� ó��
	int all_dequeue_job(class FlashMem** flashmem, int mapping_method); //Victim Block ť�� ��� Victim Block�� ���ͼ� ó��
	int enqueue_job(class FlashMem** flashmem, int mapping_method); //Victim Block ť�� ����
	void set_invalid_ratio_threshold(class FlashMem** flashmem); //���� ��� ������ ���丮�� �뷮�� ���� ������ ��ȿ�� �Ӱ谪 ����
};
#endif