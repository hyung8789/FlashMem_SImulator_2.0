#include "Spare_block_table.h"

// Round-Robin ����� Wear-leveling�� ���� Spare Block Selection Algorithm�� ����
// ���� ������ Spare Block ���̺� ����

Spare_Block_Table::Spare_Block_Table()
{
	this->is_full = false; //�ʱ� ��� ����

	this->write_index = 0;
	this->read_index = 0;
	this->table_size = 0;
	this->table_array = NULL;
}

Spare_Block_Table::Spare_Block_Table(unsigned int spare_block_size)
{
	this->is_full = false; //�ʱ� ��� ����
	
	this->write_index = 0;
	this->read_index = 0;
	this->table_size = spare_block_size;
	this->table_array = NULL;

	this->init();
}

Spare_Block_Table::~Spare_Block_Table()
{
	if (this->table_array != NULL)
	{
		delete[] this->table_array;
		this->table_array = NULL;
	}
}

void Spare_Block_Table::init()
{
	if (this->table_array == NULL)
		this->table_array = new spare_block_element[this->table_size];

	this->load_read_index(); //�ʱ� ���� �� Round-Robin ����� Wear-leveling�� ���� ���� read_index �� �� �Ҵ� (���� �� ���)
}


void Spare_Block_Table::print() //Spare Block�� ���� ���� �迭 ��� �Լ�(debug)
{
	if (this->is_full != true) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Table\n");
		system("pause");
		exit(1);
	}

	std::cout << "current read_index : " << this->read_index << std::endl;

	for (unsigned int i = 0; i < this->table_size; i++)
	{
		std::cout << "INDEX : " << i << "|| ";
		std::cout << "Spare_block : " << this->table_array[i] << std::endl;
	}
}

int Spare_Block_Table::rr_read(class FlashMem*& flashmem, spare_block_element& dst_spare_block, unsigned int& dst_read_index) //���� read_index�� ���� read_index ����, Spare Block ��ȣ ���� �� ���� Spare Block ��ġ�� �̵�
{
	META_DATA* meta_buffer = NULL;

	if (this->is_full != true) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Table\n");
		system("pause");
		exit(1);
	}

	/*** �Ϲ� ��ϰ� SWAP�� �߻��Ͽ�����, ���� GC�� ���� ó���� ���� ���� Spare Block�� ���Ͽ� ��� �� �� ������ ����ó�� ***/
	unsigned int end_read_index = this->read_index;
	do {
		SPARE_read(flashmem, (this->table_array[this->read_index] * BLOCK_PER_SECTOR), meta_buffer); //PBN * BLOCK_PER_SECTOR == �ش� PBN�� meta ����
		
		if (meta_buffer->block_state == BLOCK_STATE::SPARE_BLOCK_EMPTY) //��ȿ�ϰ� ����ִ� ����� ��� ����
		{
			dst_spare_block = this->table_array[this->read_index]; //Spare Block�� PBN ����
			dst_read_index = this->read_index; //SWAP�� ���� �ش� PBN�� ���̺� �� index ����
			
			this->read_index = (this->read_index + 1) % this->table_size;
			this->save_read_index();
			
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			return SUCCESS;
		}
		else //���� GC�� ���� ó���� ���� ���� ����� ���, �ش� ����� Erase ���� ������ ��� �Ұ�
			this->read_index = (this->read_index + 1) % this->table_size;

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

	} while (this->read_index != end_read_index); //�� ���� ��������

	this->save_read_index();

	return FAIL; //���� ��� ���� �� Spare Block�� �����Ƿ�, Victim Block Queue�� ���� GC���� ó���� �����Ͽ��� ��

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (rr_read)\n");
	system("pause");
	exit(1);
}

int Spare_Block_Table::seq_write(spare_block_element src_spare_block) //���̺� �� ���� �Ҵ�
{
	if (this->is_full == true) //���� á�� ��� �� �̻� ��� �Ұ�
		return FAIL;

	this->table_array[this->write_index] = src_spare_block;
	this->write_index++;
	
	if (this->write_index == this->table_size)
	{
		this->is_full = true;
		return COMPLETE;
	}

	return SUCCESS;
}

int Spare_Block_Table::save_read_index() //Reorganization�� ���� ���� read_index �� ����
{
	FILE* rr_read_index = NULL;

	if (this->is_full != true) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� ó���ؼ��� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Table\n");
		system("pause");
		exit(1);
	}

	if ((rr_read_index = fopen("rr_read_index.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
	{
		fprintf(stderr, "rr_read_index.txt ������ ������� �� �� �����ϴ�. (save_read_index)");

		return FAIL;
	}

	fprintf(rr_read_index, "%u", this->read_index);
	fclose(rr_read_index);

	return SUCCESS;
}

int Spare_Block_Table::load_read_index() //Reorganization�� ���� ���� read_index �� �ҷ���
{
	FILE* rr_read_index = NULL;

	if ((rr_read_index = fopen("rr_read_index.txt", "rt")) == NULL) //�б� + �ؽ�Ʈ���� ���
	{
		this->read_index = 0;
		return COMPLETE;
	}

	fscanf(rr_read_index, "%u", &this->read_index);
	fclose(rr_read_index);

	return SUCCESS;
}