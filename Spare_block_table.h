#ifndef _SPARE_BLOCK_TABLE_H_
#define _SPARE_BLOCK_TABLE_H_

#include "FlashMem.h"

// Round-Robin ����� Wear-leveling�� ���� Spare Block Selection Algorithm�� ����
// ���� ������ Spare Block ���̺� ����

#define SWAP(x,y,temp) ((temp)=(x) ,(x)=(y), (y)=(temp)) //x,y�� ��ȯ�ϴ� ��ũ�� ����

/***
	< Spare Block Table ���� �� ��� �ñ� >

	- ��� ���� (Static Table, Dynamic Table)
	1) FTL_write �� Overwrite�� ���� ��ȿ ������ copy�� Spare Block�� ���
	2) ���� Spare Block�� �Ϲ� ������� ���� �� ���� ���̺� �󿡼� SWAP

	- ���̺긮�� ���� (Log Algorithm, Dunamic Table)
	1) FTL_write �� PBN1�� PBN2�� ���� full_merge�߻� �� merge�������� Spare Block�� ����� ��ȿ ������ copy
	2) ���� Spare Block�� �Ϲ� ������� ���� �� ���� ���̺� �󿡼� SWAP

	- GC Scheduler
	1) Log Algorithm�� ������ ���̺긮�� ���ο��� �ϳ��� �� ��� LBN�� ������ PBN1�� PBN2�� ���� Merge��������
	Spare Block�� ��ȿ ������ copy �� ���� PBN1�� PBN2 ��� Erase ����ȴ���, �ϳ��� Spare Block�� SWAP����
	=> �̿� ����, Merge�� �߻��ϸ�, GC���� Erase �߻����� ����

	2) ��� ���ο��� FTL_write������ Overwrite�� ���� ��� ��ȿȭ�� �Ͼ�ٸ�, ���� �۾��� ������ ���Ͽ�
	��ȿȭ�� ����� Victim Block���� ���� �� Spare Block�� SWAP����
	=> �̿� ����, GC�� �ش� Victim Block�� Erase �����Ͽ��� �� 
***/

/***
	< Round-Robin Based Spare Block Table�� ���� ���� read_index ó�� ��� >

	1) ���� �÷��� �޸� �����ϰ� ���ο� �÷��� �޸� �Ҵ� �� (*flashmem != NULL)
	2) Bootloader�� ���� Reorganization�� ���� ���� �� �÷��� �޸𸮸� �����ϰ�, ���ο� �÷��� �޸� �Ҵ� �� (*flashmem == NULL)
	---
	=> Physical_func�� init�Լ� ���� �� ���� read_index ����
***/

typedef unsigned int spare_block_element;

class Spare_Block_Table
{
public:
	Spare_Block_Table();
	/*** 
		FIXED_FLASH_INFO(F_FLASH_INFO) : �÷��� �޸� ���� �� �����Ǵ� ������ ������ spare_block_size ���޹޾� �ʱ�ȭ
		---
		=> �ʱ� ���� �� Round-Robin ����� Wear-leveling�� ���� ���� read_index �� �� �Ҵ� (���� �� ���)
	***/
	Spare_Block_Table(unsigned int spare_block_size);
	~Spare_Block_Table();

	spare_block_element* table_array; //Spare Block�� ���� ���� �迭

	void print(); //Spare Block�� ���� ���� �迭 ��� �Լ�(debug)

	/***
		�Ϲ� ��ϰ� Spare Block�� SWAP�� ���� �� read_index�� ���Ͽ� table_array�� �����Ͽ� SWAP ����
		�̿� ���� �׻� �ʱ� �Ҵ� �� ��ŭ�� Spare Block�� ������
		---
		��� Spare Block�� ���� ���ư��鼭 ����ϹǷ� Ư�� ��Ͽ��� ���� �۾��� ����Ǵ� ���� ����
	***/
	int rr_read(class FlashMem*& flashmem, spare_block_element& dst_spare_block, unsigned int& dst_read_index); //���� read_index�� ���� read_index ����, Spare Block ��ȣ ���� �� ���� Spare Block ��ġ�� �̵�
	int seq_write(spare_block_element src_spare_block); //���̺� �� ���� �Ҵ�

	int save_read_index(); //Reorganization�� ���� ���� read_index �� ����
	int load_read_index(); //Reorganization�� ���� ���� read_index �� �ҷ���
	
private:
	void init(); //Spare Block ���̺� �ʱ�ȭ

	bool is_full; //Spare Block ���̺��� ��� ������ ��� ����
	unsigned int write_index; //Spare Block ���̺������� ������ ����� ���� ���� ��� ��ġ ����
	unsigned int read_index; //Wear-leveling�� ���� �а��� �� ���� Spare Block ��ġ
	unsigned int table_size; //���̺��� �Ҵ� ũ��
};
#endif