#ifndef _BUILD_OPTIONS_H_
#define _BUILD_OPTIONS_H_

#include "FlashMem.h"

// ���� �ɼ� ����

/***
	< ���� ó�� >
	���� ������ �������� ���� �������� �߰����� ���� ó���� ���� �ʰ�, �ܼ� ȣ�⸸ �ϵ���, 
	�߻� ������ ��� ġ������ ���ܿ� ���ؼ��� ���� �������� ó���Ѵ�.
***/

#define DEBUG_MODE //����� ��� - �߻� ������ ��� ���� ��Ȳ ���� (�ּ� ó�� : ��� ����, �⺻ �� : ���)
#define BLOCK_TRACE_MODE //��� ���� ��� �� ���� ���� ��� - �ؽ�Ʈ ����(trace_per_block_result.txt)�� ��� (�ּ� ó�� : ��� ����, �⺻ �� : ���)
/***
	Victim Block Queue�� ũ��� Spare Block ������ �ٸ��� �� ���(��ü ��� ������ ���� ������ �ٸ� ���)
	Round-Robin Based Wear-leveling�� ���� ������ Victim Block��� Spare Block ���� ũ���� Spare Block Table�� ���� SWAP �߻� �ÿ�
	Spare Block Table�� ��� Spare Block���� GC�� ���� ó������ �ʾ��� ���, GC�� ���� ������ ó���ϵ��� �ؾ� ��
	=> ���� ó���� �߰��ؾ� ������ ��������, �� �� ������ ���� �����ؾ� �Ѵ�.
	---
	ex)
	1) SPARE_BLOCK_RATIO�� 10%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 10%�� ��� ����
	2) SPARE_BLOCK_RATIO�� 8%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 10%�� ��� �Ұ���
	3) SPARE_BLOCK_RATIO�� 10%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 8%�� ��� �Ұ���
***/
#define SPARE_BLOCK_RATIO 0.08 //��ü ��� ������ ���� �ý��ۿ��� ������ Spare Block ���� (8%)
#define VICTIM_BLOCK_QUEUE_RATIO 0.08 //Victim Block ť�� ũ�⸦ ������ �÷��� �޸��� ��ü ��� ������ ���� ���� ũ��� ����
#endif