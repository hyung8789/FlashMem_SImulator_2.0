#ifndef _BUILD_OPTIONS_H_
#define _BUILD_OPTIONS_H_
#define _CRT_SECURE_NO_WARNINGS

// ���� �ɼ� �� ��Ÿ ��ó�� ��ũ��, �ʿ��� ��� ���� ����

/***
	< ���� ó�� >
	���� ������ �������� ���� �������� �߰����� ���� ó���� ���� �ʰ�, �ܼ� ȣ�⸸ �ϵ���, 
	�߻� ������ ��� ġ������ ���ܿ� ���ؼ��� ���� �������� ó���Ѵ�.
***/

//#define DEBUG_MODE //����� ��� - �߻� ������ ��� ���� ��Ȳ ���� (�ּ� ó�� : ��� ����, �⺻ �� : ���)
#define PAGE_TRACE_MODE //��� ����(������) �� ���� ���� ��� - �ؽ�Ʈ ����(trace_per_page_result.txt)�� ��� (�ּ� ó�� : ��� ����, �⺻ �� : ���)
#define BLOCK_TRACE_MODE //��� ���� ��� �� ���� ���� ��� - �ؽ�Ʈ ����(trace_per_block_result.txt)�� ��� (�ּ� ó�� : ��� ����, �⺻ �� : ���)
#define SPARE_BLOCK_RATIO 0.08 //��ü ��� ������ ���� �ý��ۿ��� ������ Spare Block ���� (8%)
#define VICTIM_BLOCK_QUEUE_RATIO 0.08 //Victim Block ť�� ũ�⸦ ������ �÷��� �޸��� ��ü ��� ������ ���� ���� ũ��� ����
//#define GC_LAZY_MODE_RATIO_THRESHOLD

/***
	Victim Block Queue�� ũ��� Spare Block ������ �ٸ��� �� ���(��ü ��� ������ ���� ������ �ٸ� ���)
	Round-Robin Based Wear-leveling�� ���� ������ Victim Block��� Spare Block ���� ũ���� Spare Block Queue�� ���� SWAP �߻� �ÿ�
	Spare Block Queue�� ��� Spare Block���� GC�� ���� ó������ �ʾ��� ���, GC�� ���� ������ ó���ϵ��� �ؾ� ��
	=> ���� ó���� �߰��ؾ� ������ ��������, �� �� ������ ���� �����ؾ� �Ѵ�.
	---
	ex)
	1) SPARE_BLOCK_RATIO�� 10%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 10%�� ��� ����
	2) SPARE_BLOCK_RATIO�� 8%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 10%�� ��� �Ұ���
	3) SPARE_BLOCK_RATIO�� 10%�̰�, VICTIM_BLOCK_QUEUE_RATIO�� 8%�� ��� �Ұ���
***/

#include <stdio.h> //fscanf,fprinf,fwrite,fread
#include <stdint.h> //���� �ڷ���
#include <iostream> //C++ �����
#include <sstream> //stringstream
#include <windows.h> //�ý��� ��ɾ�
#include <math.h> //ceil,floor,round
#include <stdbool.h> //boolean
#include <chrono> //trace �ð� ����
#include <random> //random_device, mersenne twister

//���� ��� �� ����
#define SUCCESS 1 //����
#define	COMPLETE 0 //�ܼ� ���� �Ϸ�
#define FAIL -1 //����

#define MAX_CAPACITY_MB 65472 //�������� �� �÷��� �޸��� MB���� �ִ� ũ��

#define MB_PER_BLOCK 64 //1MB�� 64 ���
#define MB_PER_SECTOR 2048 //1MB�� 2048����
#define BLOCK_PER_SECTOR 32 //�� ��Ͽ� �ش��ϴ� ������ ����
#define SECTOR_PER_BYTE 512 //�� ������ �����Ͱ� ��ϵǴ� ������ ũ�� (byte)

#define SPARE_AREA_BYTE 16 //�� ������ Spare ������ ũ�� (byte)
#define SPARE_AREA_BIT 128 //�� ������ Spare ������ ũ�� (bit)
#define SECTOR_INC_SPARE_BYTE 528 //Spare ������ ������ �� ������ ũ�� (byte)

#define DYNAMIC_MAPPING_INIT_VALUE UINT32_MAX //���� ���� ���̺��� �ʱⰪ (���� ������ �÷��� �޸��� �뷮 ������ ���� ���� ���� ������ �ּ� ��(PBN,PSN))
#define OFFSET_MAPPING_INIT_VALUE INT8_MAX //������ ���� ���� ���̺��� �ʱⰪ (�� ��Ͽ� ���� ������ ���̺��� �ִ� ũ��� ��� �� ����(������)���̹Ƿ� �� ���� ������ ���� �� ���� ��)

#include "Spare_area.h"
#include "Circular_Queue.h" //��⿭�� ���� Meta ���� ���� ���� �ʿ�
#include "FlashMem.h"
#include "utils.hpp"
#include "GarbageCollector.h"
#endif