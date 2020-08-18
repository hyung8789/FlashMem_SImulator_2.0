#ifndef _SPARE_AREA_H_
#define _SPARE_AREA_H_

#include "FlashMem.h"

// Spare Area�� ���� ��Ʈ ���� ó��, Meta-data �ǵ��� ���� �Լ� SPARE_init, SPARE_read, SPARE_write ����
// ������ ���� ���� ���� ������ Garbage Collection�� ���� SPARE_reads, update_v_flash_info_for_reorganization, update_v_flash_info_for_erase, calc_block_invalid_ratio ����
// Meta-data�� ���� �� �Ϲ� ���� ��� Ž�� �� Ư�� ���� ��� ���� �� ���� ������ Ž�� ���� search_empty_block, search_empty_offset_in_block ����

//8��Ʈ ũ���� Spare Area �ʱⰪ ����
#define SPARE_INIT_VALUE (0xff) //0xff(16) = 11111111(2) = 255(10)

//��Ʈ ���� boolean�� ����
#define TRUE_bit (0x1)
#define FALSE_bit (0x0)

/***
	1byte == 8bit
	Soare Area�� ũ��� 16����Ʈ�̹Ƿ�, �� 128��Ʈ�� ũ��
	�� ���͸��� Spare ������ (Spare ��� ���� || ��� ��ȿȭ ���� || �� ��� ���� || ���� ��ȿȭ ���� || �� ���� ���� || ������� �ʴ� ����) ������ ���

	- �ʱⰪ ��� 0x1 (true)�� �ʱ�ȭ
	- ��� ���� Erase ���� ���� �� ��� 0x1 (true)�� �ʱ�ȭ
	
	-----------------------------------------------------------

	< meta ���� ���� ��� >

	- FTL_write :
	1) �ش� ����(������)�� meta ���� �Ǵ� ����� meta ���� �Ǻ��� ���Ͽ� ���ǵ� Spare Area ó�� �Լ�(Spare_area.h, Spare_area.cpp)�κ��� meta������ �޾ƿ� ó�� ����
	2) ���� ����(������)�� �������� ������ ����� ���Ͽ� meta������ ���� �� ��� �� �����͸� Flash_write�� �����Ͽ� ����
	3) � ��ġ�� ���Ͽ� Overwrite�� ������ ��� �ش� ��ġ(������ �Ǵ� ���)�� ��ȿȭ��Ű�� ���Ͽ� ���ǵ� Spare Areaó�� �Լ��� ���Ͽ� ����

	- Flash_write : 
	1) �������� ����(������)�� ���� ������ ��� �� �ش� ����(������)�� Spare Area�� ���� meta ���� ��� ����
	2) ���� ����(������)�� ���� ������ ����� ���ؼ��� meta ���� (�� ����(������) ����)�� �ݵ�� ���� ���Ѿ� ��
	=> �̿� ����, ȣ�� ������ �ش� ����(������)�� ���� �̸� ĳ�õ� meta������ ���� ��� ���� Spare Area�� ���� meta������ �о���̰�,
	�ش� ����(������)�� ���������, ��� ����. ������� ������, �÷��� �޸��� Ư���� ���� Overwrite ����
	3) �̸� ���Ͽ�, ���� ������ FTL_write���� �� ����(������) ���θ� �̸� �����ų ���, Flash_write���� meta ���� �Ǻ��� �������� ���� �ܼ� ��ϸ� ������ �� ������,
	���� ����� ������� ���� ���, ���� �۾��� ���� Overwrite ������ �����ϱ� ���ؼ��� ���� ������ ������ �аų�, ���� ��ĺ��� ������ ó�� ������ ������ �Ѵ�.
	�̿� ����, Common ������ �ܼ�ȭ�� ���Ͽ�, �������� ����(������)������ ����� �߻��ϴ� Flash_write�󿡼��� �� ����(������) ���θ� �����Ѵ�.

***/

enum class META_DATA_BIT_POS : const __int8 //Spare Area�� meta-data�� ���� ��Ʈ ��ġ ������ ����
{
	/***
		128bitũ���� Spare Area�� ���� �� meta-data�� Spare Area�󿡼� ��Ʈ ��ġ ����
		2^127 ~ 2^0������ �������� ��ġ ����
	***/
	//==========================================================================================================================
	//�� ����� ù ��° ����(������)�� Spare ������ �ش� ��� ������ ����
	not_spare_block = 127, //�ش� ����� �ý��ۿ��� �����ϴ� Spare Block ���� (1bit) - ���� ��Ʈ ���� ó���� ���� �Ǻ��� �����ϴ� ��ġ
	valid_block = 126, //�ش� ����� ��ȿȭ ���� (1bit)
	empty_block = 125, //�ش� ��Ͽ� ������ ��� ���� (1bit)

	//��� ����(������)���� ����
	valid_sector = 124, //�ش� ����(������)�� ��ȿȭ ���� (1bit)
	empty_sector = 123 //�ش� ����(������)�� ������ ��� ���� (1bit) - ���� ��Ʈ ���� ó���� ���� �Ǻ��� ������ ��ġ
	//==========================================================================================================================
	//�� ���� ��Ʈ �ڸ��鿡 ���ؼ��� ��� ���� (2^122 ~ 2^0)
};

class META_DATA //Spare Area�� ��ϵ� meta-data�� ���� �ǵ��� ���� �ϱ� ���� Ŭ����
{
public:
	META_DATA();
	~META_DATA();
	
	/***
		���������� ���ǵ� META_DATA_BIT_POS�� ���� �� meta������ ��Ʈ�ڸ�(2^127 ~ 2^0)�� �����Ǿ����Ƿ�
		�迭�� index(0~127)�� ��Ʈ�ڸ��� �����Ͽ�, �аų�(read) ��� ��(write)�� 
		�� �� (2^127�� �ش��ϴ� �ڸ�, index : 127)�������� index 0���� ���������� ����
	***/

	bool* meta_data_array; //meta������ ���� �迭(��� �Ǵ� ���� �ÿ� �ڿ�������)

	int seq_write(unsigned flag_bit); //�о���� flag_bit�κ��� �ڿ������� ����������(sequential) ���
	int seq_read(bool& dst_result); //�ڿ������� ����������(sequential) �о �� ����

private:
	bool is_full; //META_DATA�� ��� ������ ��� ����
	int write_index; //META_DATA�������� ������ ����� ���� ���� ��� ��ġ ����
	int read_index; //META_DATA�������� ������ �б⸦ ���� ���� �б� ��ġ ����
};

//Spare_area.cpp
//Spare Area �����Ϳ� ���� ó�� �Լ�
int SPARE_init(class FlashMem** flashmem, FILE** storage_spare_pos); //���� ����(������)�� Spare Area�� ���� �ʱ�ȭ
META_DATA* SPARE_read(class FlashMem** flashmem, FILE** storage_spare_pos); //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� ���·� ��ȯ
int SPARE_write(class FlashMem** flashmem, FILE** storage_spare_pos, META_DATA** src_data); //META_DATA�� ���� Ŭ���� ���޹޾�, ���� ������ Spare Area�� ���

//Overloading for FTL function
META_DATA* SPARE_read(class FlashMem** flashmem, unsigned int PSN); //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� ���·� ��ȯ
int SPARE_write(class FlashMem** flashmem, unsigned int PSN, META_DATA** src_data); //META_DATA�� ���� Ŭ���� ���޹޾�, ���� ������ Spare Area�� ���

/*** Depending on Spare area processing function ***/
//for Remaining Space Management and Garbage Collection
META_DATA** SPARE_reads(class FlashMem** flashmem, unsigned int PBN); //�� ���� ��ϳ��� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ���·� ��ȯ
int update_victim_block_info(class FlashMem** flashmem, bool is_logical, unsigned int src_Block_num, int mapping_method); //Victim Block ������ ���� ��� ���� ����ü ����
int update_v_flash_info_for_reorganization(class FlashMem** flashmem, META_DATA** src_data); //Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� ������ ���� ���� ���� ��� ���� ������ �÷��� �޸� ���� ����
int update_v_flash_info_for_erase(class FlashMem** flashmem, META_DATA** src_data); //Erase�ϰ��� �ϴ� Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� �÷��� �޸��� ������ ���� ����
int calc_block_invalid_ratio(META_DATA** src_data, float& dst_block_invalid_ratio); //Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� ��ȿ�� ��� �� ����
//meta������ ���� �� �Ϲ� ���� ��� Ž��
int search_empty_block(class FlashMem** flashmem, unsigned int& dst_Block_num, META_DATA** dst_data, int mapping_method, int table_type); //�� �Ϲ� ���� ���(PBN)�� ���������� Ž���Ͽ� PBN�Ǵ� ���̺� �� LBN ��, �ش� PBN�� meta���� ����
//meta������ ���� ���� ��� ���� �� ���� ������ Ž��
int search_empty_offset_in_block(class FlashMem** flashmem, unsigned int src_PBN, __int8& dst_Poffset, META_DATA** dst_data); //�Ϲ� ���� ���(PBN) ���θ� ���������� Ž���Ͽ� Poffset ��, �ش� ��ġ�� meta���� ����
#endif