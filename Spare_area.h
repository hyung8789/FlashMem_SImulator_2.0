#ifndef _SPARE_AREA_H_
#define _SPARE_AREA_H_

// Spare Area�� ���� ��Ʈ ���� ó��, Meta-data �ǵ��� ���� �Լ� SPARE_init, SPARE_read, SPARE_write ����
// ������ ���� ���� ���� ������ Garbage Collection�� ���� SPARE_reads, update_victim_block_info, update_v_flash_info_for_reorganization, update_v_flash_info_for_erase, calc_block_invalid_ratio ����
// Meta-data�� ���� �� �Ϲ� ���� ��� Ž�� �� Ư�� ���� ��� ���� �� ���� ������ Ž�� ���� search_empty_block, search_empty_offset_in_block ����

//8��Ʈ ũ���� Spare Area �ʱⰪ ����
#define SPARE_INIT_VALUE (0xff) //0xff(16) = 11111111(2) = 255(10)

//��Ʈ ���� boolean�� ����
#define TRUE_BIT (0x1)
#define FALSE_BIT (0x0)

static struct META_DATA* DO_NOT_READ_META_DATA = NULL; //Non-FTL Ȥ�� ������ ó�� �������� �̹� meta ������ �о��� ��� �ٽ� ���� �ʱ� ���� ���

/***
	�ʱⰪ ��� 0x1�� �ʱ�ȭ
	��� ���� Erase ���� ���� �� ��� 0x1�� �ʱ�ȭ
	---------
	
	�� ����(������)�� Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���
	BLOCK_TYPE(Normal or Spare, 1bit) || IS_VALID (BLOCK, 1bit) || IS_EMPTY (BLOCK, 1bit) || IS_VALID (SECTOR, 1bit) || IS_EMPTY(SECTOR, 1bit) || DUMMY (3bit)
	��Ÿ, Block Address(Logical) Area, ECC Area �� �߰������� ��� ��, META_DATA ����ü, SPARE_read, SPARE_write ����
	---------
	
	< meta ���� ���� ��� >

	- FTL_write (���� ����) :
	1) �ش� ����(������)�� meta ���� �Ǵ� ����� meta ���� �Ǻ��� ���Ͽ� ���ǵ� Spare Area ó�� �Լ�(Spare_area.h, Spare_area.cpp)�κ��� meta������ �޾ƿ� ó�� ����
	2) ���� ����(������)�� �������� ������ ����� ���Ͽ� meta������ ���� �� ��� �� �����͸� Flash_write�� �����Ͽ� ����
	3) � ��ġ(���� ������)�� ���Ͽ� Overwrite�� ������ ��� �ش� ��ġ(���� ������)�� ��ȿȭ��Ű�� ���Ͽ� ���ǵ� Spare Area ó�� �Լ��� ���Ͽ� ����

	- Flash_write , Spare Area ó�� �Լ��� (���� ����) :
	1) �������� ����(������)�� ���� ������ ��� �� �ش� ����(������)�� Spare Area�� ���� meta ���� ��� ����
	2) ���� ����(������)�� ���� ������ ����� ���ؼ��� meta ���� (����(������) ���� Ȥ�� ��� ����)�� �ݵ�� ���� ���Ѿ� ��
	=> �̿� ����, ȣ�� ������ �ش� ����(������)�� ���� �̸� ĳ�õ� meta������ ���� ��� ���� Spare Area�� ���� meta������ �о���̰�,
	�ش� ����(������)�� ���������, ��� ����. ������� ������, �÷��� �޸��� Ư���� ���� Overwrite ����
	3) �̸� ���Ͽ�, ���� ������ FTL_write���� �� ����(������) ���θ� �̸� �����ų ���, Flash_write���� meta ���� �Ǻ��� �������� ���� �ܼ� ��ϸ� ������ �� ������,
	���� ����� ������� ���� ���, ���� �۾��� ���� Overwrite ������ �����ϱ� ���ؼ��� ���� ������ ������ �аų�, ���� ��ĺ��� ������ ó�� ������ ������ �Ѵ�.
	�̿� ����, ���� ������ �ܼ�ȭ�� ���Ͽ�, �������� ����(������)������ ����� �߻��ϴ� Flash_write�󿡼��� �� ����(������) ���θ� �����Ѵ�.

	< trace�� ���� Global �÷��� �޸� �۾� ī��Ʈ�� ���, ����(������) �� ���� ī��Ʈ ���� ��� >
	
	Global �÷��� �޸� �۾� Ƚ�� ������ FlashMem.h�� VARIABLE_FLASH_INFO�� ���뿡 ������.
	Spare Area�� ���� ���� read, write, �ʱ�ȭ(erase)�۾��� �� ���� �÷��� �޸��� read, write, erase �۾����� ���
	��, ������ ������ ������ Spare Area�� ���� ó�� �߻� �� �̴� �� ���� �۾����� ó���Ѵ�.
	---
		1) Spare Area�� ������ ������ ������ ���� ��(��, ���� �������� ���� meta ���� �ǵ��� �����Ͽ��� ���)�� Flash_read������ read ī��Ʈ ���� 
		(�бⰡ �ش� ����(������)�� �ٽ� �߻��Ͽ����Ƿ�)

		2) erase ī��Ʈ�� ��� �� �� ���̹Ƿ� Flash_erase���� ����
		3) ��� ��ġ�� ���Ͽ� ���� �۾� �� �ݵ�� meta ����(���� ���� Ȥ�� ��� ����)�� �Բ� �����ؾ��ϹǷ�, ������ ������ ó���� �� ����
			=> �̿� ���� write ī��Ʈ�� Spare Area�� ó�� �Լ������� ����
***/

enum class BLOCK_STATE : const unsigned //��� ���� ����
{
	/***
		�� ����� ù ��° ����(������)�� Spare ������ �ش� ��� ������ ����
		�Ϲ� ������ ��� Ȥ�� ������ ������ ����� �Ұ����� Spare ��Ͽ� ���Ͽ�,
		- EMPTY : �ش� ����� ��� ���� ��� �����¿� ���� ����ְ�, ��ȿ�ϴ�. (�ʱ� ����)
		- VALID : �ش� ����� ��ȿ�ϰ�, �Ϻ� ��ȿ�� �����Ͱ� ��ϵǾ� �ִ�.
		- INVALID : �ش� ��� ���� ��� �����¿� ���� ��ȿ�ϰų�, �Ϻ� ��ȿ �����͸� �����ϰ� ������ �� �̻� ��� �Ұ���. ���� ���ؼ� ��� ���� Erase �����Ͽ��� ��
	***/

	/***
		BLOCK_TYPE || IS_VALID || IS_EMPTY
		��Ͽ� ���� 6���� ����, 2^3 = 8, 3��Ʈ �ʿ�
	***/

	NORMAL_BLOCK_EMPTY = (0x7), //�ʱ� ���� (Default), 0x7(16) = 7(10) = 111(2)
	NORMAL_BLOCK_VALID = (0x6), //0x6(16) = 6(10) = 110(2)
	NORMAL_BLOCK_INVALID = (0x4), //0x4(16) = 4(10) = 100(2)
	SPARE_BLOCK_EMPTY = (0x3), //0x3(16) = 3(10) = 011(2)
	SPARE_BLOCK_VALID = (0x2), //0x2(16) = 2(10) = 010(2)
	SPARE_BLOCK_INVALID = (0x0) //0x0(16) = 0(10) = 000(2)
};

enum class SECTOR_STATE : const unsigned //����(������) ���� ����
{
	/***
		��� ����(������)���� ����
		- EMPTY : �ش� ����(������)�� ����ְ�, ��ȿ�ϴ�. (�ʱ� ����)
		- VALID : �ش� ����(������)�� ��ȿ�ϰ�, ��ȿ�� �����Ͱ� ��ϵǾ� �ִ�.
		- INVALID : �ش� ����(������)�� ��ȿ�ϰ�, ��ȿ�� �����Ͱ� ��ϵǾ� �ִ�. ���� ���ؼ� ��� ���� Erase �����Ͽ��� ��
	***/

	/***
		IS_VALID || IS_EMPTY
		���Ϳ� ���� 3���� ����, 2^2 = 4, 2��Ʈ �ʿ�
	***/

	EMPTY = (0x3), //�ʱ� ���� (Default), 0x3(16) = 3(10) = 11(2)
	VALID = (0x2), //0x2(16) = 2(10) = 10(2)
	INVALID = (0x0) //0x0(16) = 0(10) = 00(2)
};

struct META_DATA //Flash_read,write�� FTL_read,write���� ������ ó���� ���� �ܺ������� ���� �� ����
{
	BLOCK_STATE block_state; //��� ����
	SECTOR_STATE sector_state; //���� ����
};

//Spare_area.cpp
//Spare Area �����Ϳ� ���� ó�� �Լ�
int SPARE_init(class FlashMem*& flashmem, FILE*& storage_spare_pos); //���� ����(������)�� Spare Area�� ���� �ʱ�ȭ

int SPARE_read(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& dst_meta_buffer); //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü ���·� ��ȯ
int SPARE_read(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& dst_meta_buffer); //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü ���·� ��ȯ

int SPARE_write(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& src_meta_buffer); //META_DATA�� ���� ����ü ���޹޾�, ���� ������ Spare Area�� ���
int SPARE_write(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& src_meta_buffer); //META_DATA�� ���� ����ü ���޹޾�, ���� ������ Spare Area�� ���

/*** Depending on Spare area processing function ***/
//for Remaining Space Management and Garbage Collection
int SPARE_reads(class FlashMem*& flashmem, unsigned int PBN, META_DATA**& dst_block_meta_buffer_array); //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü �迭 ���·� ��ȯ
int SPARE_writes(class FlashMem*& flashmem, unsigned int PBN, META_DATA**& src_block_meta_buffer_array); //�� ���� ��� ���� ��� ����(������)�� ���� meta���� ���

int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, unsigned int src_block_num, enum MAPPING_METHOD mapping_method); //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ����
/*** �̹� �о���� meta ������ �̿��Ͽ� ���� ***/
int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, unsigned int src_block_num, META_DATA**& src_block_meta_buffer_array, enum MAPPING_METHOD mapping_method); //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ���� (��� ����)
int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, unsigned int src_block_num, META_DATA**& src_PBN1_block_meta_buffer_array, META_DATA**& src_PBN2_block_meta_buffer_array, enum MAPPING_METHOD mapping_method);  //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ���� (���̺긮�� ����)

int update_v_flash_info_for_reorganization(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array); //Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� ������ ���� ���� ���� ��� ���� ������ �÷��� �޸� ���� ����
int update_v_flash_info_for_erase(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array); //Erase�ϰ��� �ϴ� Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� �÷��� �޸��� ������ ���� ����

int calc_block_invalid_ratio(META_DATA**& src_block_meta_buffer_array, float& dst_block_invalid_ratio); //Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� ��ȿ�� ��� �� ����

//int search_empty_normal_block_for_dynamic_table(class FlashMem*& flashmem, unsigned int& dst_block_num, META_DATA*& dst_meta_buffer, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type); //�� �Ϲ� ���� ���(PBN)�� ���������� Ž���Ͽ� PBN�Ǵ� ���̺� �� LBN ��, �ش� PBN�� meta���� ����
int search_empty_offset_in_block(class FlashMem*& flashmem, unsigned int src_PBN, __int8& dst_Poffset, META_DATA*& dst_meta_buffer, enum MAPPING_METHOD mapping_method); //�Ϲ� ���� ���(PBN) ���θ� �������� ����ִ� ��ġ Ž��, Poffset ��, �ش� ��ġ�� meta���� ����
int search_empty_offset_in_block(META_DATA**& src_block_meta_buffer_array, __int8& dst_Poffset, enum MAPPING_METHOD mapping_method); //�Ϲ� ���� ���(PBN)�� ��� ���� meta ������ �������� ����ִ� ��ġ Ž��, Poffset �� ����
int print_block_meta_info(class FlashMem*& flashmem, bool is_logical, unsigned int src_block_num, enum MAPPING_METHOD mapping_method); //��� ���� ��� ����(������)�� meta ���� ���

//�޸� ����
int deallocate_single_meta_buffer(META_DATA*& src_meta_buffer);
int deallocate_block_meta_buffer_array(META_DATA**& src_block_meta_buffer_array);
#endif