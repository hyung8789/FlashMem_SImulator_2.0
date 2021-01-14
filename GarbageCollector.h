#ifndef _GARBAGECOLLECTOR_H_
#define _GARBAGECOLLECTOR_H_

// Garbage Collecter ������ ���� Ŭ���� ����

#define SWAP(x,y,temp) ((temp)=(x) ,(x)=(y), (y)=(temp)) //x,y�� ��ȯ�ϴ� ��ũ�� ����

class GarbageCollector //GarbageCollector.cpp
{
public:
	GarbageCollector();
	~GarbageCollector();

	bool RDY_terminate; //���� ��� ����

	int scheduler(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type); //main scheduling function for GC

private: //Execution by scheduling function
	int one_dequeue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type); //Victim Block ť�κ��� �ϳ��� Victim Block�� ���ͼ� ó��
	int all_dequeue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type); //Victim Block ť�� ��� Victim Block�� ���ͼ� ó��
	int enqueue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type); //Victim Block ť�� ����
};
#endif