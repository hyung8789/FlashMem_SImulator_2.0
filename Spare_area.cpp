#include "Spare_area.h"

// Spare Area�� ���� ��Ʈ ���� ó��, Meta-data �ǵ��� ���� �Լ� SPARE_init, SPARE_read, SPARE_write ����
// ������ ���� ���� ���� ������ Garbage Collection�� ���� SPARE_reads, update_v_flash_info_for_reorganization, update_v_flash_info_for_erase, calc_block_invalid_ratio ����
// Meta-data�� ���� �� �Ϲ� ���� ��� Ž�� �� Ư�� ���� ��� ���� �� ���� ������ Ž�� ���� search_empty_block, search_empty_offset_in_block ����

META_DATA::META_DATA()
{
	this->is_full = false; //�ʱ� ��� ����
	this->write_index = SPARE_AREA_BIT - 1; //������ ��� �� ��ġ (127 ~ 0)
	this->read_index = SPARE_AREA_BIT - 1; //������ ���� ��ġ (127 ~ 0)
	this->meta_data_array = new bool[SPARE_AREA_BIT]; //(2^0 ~ 2^127 ���� �ڸ����� �ش��ϴ� meta������ �����ϴ� �迭 ����)
}

META_DATA::~META_DATA()
{
	if (this->meta_data_array != NULL)
	{
		delete [] this->meta_data_array;
		this->meta_data_array = NULL;
	}
}

int META_DATA::seq_write(unsigned flag_bit) //�о���� flag_bit�κ��� �ڿ������� ����������(sequential) ���
{
	//�ڿ������� ���(2^127 �ڸ����� �ش��ϴ� �ε�������)

	if (this->is_full == true) //�� �̻� ��� �Ұ�
	{
		return FAIL;
	}

	switch (flag_bit)
	{
	case TRUE_bit: //0x1
		this->meta_data_array[write_index] = true;
		break;

	case FALSE_bit: //0x0
		this->meta_data_array[write_index] = false;
		break;

	default:
		fprintf(stderr, "flag_bit �Ǻ� ����\n");
		system("pause");
		exit(1);
		break;
	}
	
	this->write_index--;
	/***
		���� ������� meta������ ��Ʈ ��ġ(META_DATA_BIT_POS::empty_sector)������ ����ϵ��� �Ͽ����Ƿ�
		�߰����� meta���� ����� �ϰ��� �Ѵٸ� META_DATA_BIT_POS�� (��Ʈ �뵵) = (��Ʈ �ڸ� ��ġ(127~0))�� �߰�,
		record ����� �ϰ��� �ϴ� scope�� ���� �ٽ� �����ؾ���
	***/
	if (this->write_index < 0 || this->write_index < (__int8)META_DATA_BIT_POS::empty_sector)
	{
		this->is_full = true; //�� �̻� ��� �Ұ�
		return COMPLETE; //write complete
	}
	
	return SUCCESS;
}

int META_DATA::seq_read(bool& dst_result) //�ڿ������� ����������(sequential) �о �� ����
{
	//�ڿ������� ����(2^127 �ڸ����� �ش��ϴ� �ε�������)
	//�� ȣ��ø��� ���������� �ε����� ���ҽ�Ű�鼭 ������ ��ȯ

	if (this->is_full != true) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		return FAIL;
	}

	dst_result = this->meta_data_array[read_index];
	read_index--;
	
	/***
		���� ������� meta������ ��Ʈ ��ġ(META_DATA_BIT_POS::empty_sector)������ �е��� �Ͽ����Ƿ�
		�߰������� ����� meta������ �а��� �ϰ��� �Ѵٸ� META_DATA_BIT_POS�� (��Ʈ �뵵) = (��Ʈ �ڸ� ��ġ(127~0))�� �߰�,
		record �б⸦ �ϰ��� �ϴ� scope�� ���� �ٽ� �����ؾ���
	***/
	if (this->read_index < 0 || this->read_index < (__int8)META_DATA_BIT_POS::empty_sector)
	{
		this->read_index = SPARE_AREA_BIT - 1; 
		return COMPLETE; //read complete
	}

	return SUCCESS;
}

int SPARE_init(class FlashMem** flashmem, FILE** storage_spare_pos) //���� ����(������)�� Spare Area�� ���� �ʱ�ȭ
{
	unsigned char* write_buffer = NULL; //Spare Area�� ����ϱ� ���� ����

	if ((*storage_spare_pos) != NULL)
	{
		write_buffer = new unsigned char[SPARE_AREA_BYTE];

		for (int byte_unit = 0; byte_unit < SPARE_AREA_BYTE; byte_unit++) //1����Ʈ���� �ʱ�ȭ
		{
			write_buffer[byte_unit] = SPARE_INIT_VALUE; //0xff(16) = 11111111(2) = 255(10) �� �ʱ�ȭ
		}

		fwrite(write_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, (*storage_spare_pos));

		delete[] write_buffer;
	}
	else
		return FAIL;

	return SUCCESS;
}

META_DATA* SPARE_read(class FlashMem** flashmem, FILE** storage_spare_pos) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� ���·� ��ȯ
{
	META_DATA* meta_data = NULL; //Spare Area�κ��� �о���� ���۷κ��� �Ҵ�
	unsigned char* read_buffer = NULL; //Spare Area�κ��� �о���� ����

	if ((*storage_spare_pos) != NULL)
	{
		read_buffer = new unsigned char[SPARE_AREA_BYTE];
		fread(read_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, (*storage_spare_pos)); //����Ʈ ������ �о read_buffer�� �Ҵ�

		meta_data = new META_DATA(); //�о���� ���۷κ��� �Ҵ��ϱ� ���� META_DATA ����

		__int8 META_DATA_BIT_POS = (__int8)META_DATA_BIT_POS::not_spare_block; //meta-data�� ��Ʈ ��ġ �ε���

		for (int byte_unit = 0; byte_unit < SPARE_AREA_BYTE; byte_unit++) //read_buffer�κ��� 1byte�� �и��Ͽ� ��Ʈ������ �Ǻ�
		{
			unsigned char bits_8; //1byte = 8bit
			bits_8 = read_buffer[byte_unit]; //1byte ũ�⸸ŭ read_buffer�κ��� bits_8�� �Ҵ�

			for (int bit_digits = 7; bit_digits >= 0; bit_digits--) //8��Ʈ(2^7 ~ 2^0)�� ���ؼ� 2^7 �ڸ����� �� �ڸ��� �Ǻ��Ͽ� META_DATA�� ���������� ����
			{
				//0x1(16) = 1(10) = 1(2) �� ���ϰ��� �ϴ� ��Ʈ �ڸ� ��ġ��ŭ �������� ����Ʈ ��Ű��, ���� ������ �ϴ� ��Ʈ���� & ������ ���� �ش� �ڸ��� �˻�
				meta_data->seq_write(((bits_8) & (0x1 << (bit_digits))) ? TRUE_bit : FALSE_bit);
			}

			/***
				�߰����� meta������ ��� �ϰ��� �Ѵٸ� META_DATA_BIT_POS��(��Ʈ �뵵) = (��Ʈ �ڸ� ��ġ(127~0))�� �߰�,
				�а��� �ϴ� Spare ������ ��Ʈ �ڸ��� ���� �ٽ� �����ؾ���
			***/
			META_DATA_BIT_POS -= 8; //ó���� 1byte = 8bit��ŭ ����
			if (META_DATA_BIT_POS < (__int8)META_DATA_BIT_POS::empty_sector) //Spare area�� ���� ������� �ʴ� ����(122 ~ 0)�� ��� �� �̻� ó�� �� �ʿ� ����
				break;
		}
	}
	else 
		return NULL;
	
	/*** trace���� ���� ��� ***/
	(*flashmem)->v_flash_info.flash_read_count++; //�÷��� �޸� �б� ī��Ʈ ����

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	/***
		���� �бⰡ �߻��� PBN ���, Spare Area�� ���� ó���� ���� ��
		ftell(���� ���� ������, ����Ʈ ����) - SECTOR_INC_SPARE_BYTE == Flash_read���� write_pos
		PSN = write_pos / SECTOR_INC_SPARE_BYTE
		PBN = PSN / BLOCK_PER_SECTOR
	***/
	(*flashmem)->block_trace_info[((ftell(*storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE) / BLOCK_PER_SECTOR].block_read_count++; //�ش� ����� �б� ī��Ʈ ����
#endif

	return meta_data;
}

META_DATA* SPARE_read(class FlashMem** flashmem, unsigned int PSN) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� ���·� ��ȯ
{
	FILE* storage_spare_pos = NULL;

	META_DATA* meta_data = NULL; //Spare Area�κ��� �о���� ���۷κ��� �Ҵ�

	unsigned int read_pos = 0; //�а��� �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����

	if ((storage_spare_pos = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
	{
		fprintf(stderr, "storage.bin ������ �б���� �� �� �����ϴ�. (SPARE_read)");
		system("pause");
		exit(1);
	}

	read_pos = SECTOR_INC_SPARE_BYTE * PSN; //������ �ϴ� ��ġ
	spare_pos = read_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�
	
	/*** �бⰡ ������ ��� �ܼ� return�ǹǷ�, �÷��� �޸� �б� ī��Ʈ�� ������ ���� ***/
	meta_data = SPARE_read(flashmem, &storage_spare_pos);
	fclose(storage_spare_pos);

	return meta_data;
}

int SPARE_write(class FlashMem** flashmem, FILE** storage_spare_pos, META_DATA** src_data) //META_DATA�� ���� Ŭ���� ���޹޾�, ���� ������ Spare Area�� ���
{
	unsigned char* write_buffer = NULL; //Spare Area�� ����ϱ� ���� ����

	if ((*storage_spare_pos) != NULL && (*src_data) != NULL)
	{
		/*** for Remaining Space Management ***/
		//�ش� ���Ͱ� ��ȿȭ �Ǿ��� ��� ��ȿ ī��Ʈ�� ������Ų��
		if ((*src_data)->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == false)
			(*flashmem)->v_flash_info.invalid_sector_count++; //��ȿ ���� �� ����

		write_buffer = new unsigned char[SPARE_AREA_BYTE];

		unsigned char bits_8 = SPARE_INIT_VALUE; //1byte = 8bit, 0xff�� �ʱ�ȭ�� ��Ʈ��
		int write_buffer_idx_count = 0; //�Ʒ� ���꿡 ���� 1����Ʈ ������ ��ϵ� write_buffer�� �ε��� ī����(������ ��ϵ� �ε��� ����)
		int bit_digits = 7; //1byte�� 2^7 �ڸ��� ǥ��

		for (int bit_unit = 0; bit_unit < SPARE_AREA_BIT; bit_unit++) //Spare area�� 128bit(16byte)�� ���� �ݺ�
		{
			bool result;
			if ((*src_data)->seq_read(result) != FAIL) //read SUCCESS or read COMPLETE
			{
				///0x1(16) = 1(10) = 1(2) �� loc(�����ϰ��� �ϴ� ��Ʈ �ڸ� ��ġ)��ŭ �������� ����Ʈ ��Ű��, data(���� �����ϰ��� �ϴ� ��Ʈ��)�� |(or)������ ���� �ش� ��ġ�� 1�� ����
				//(�̹� �ش� �ڸ��� 1�� ��� ��������, or ���� : �� ��Ʈ ��� 0�� ��쿡�� 0)
				switch (result)
				{
				case true:
					//bits_8 |= 0x1 << bit_digits;
					bit_digits--; //�̹� 0x1�̹Ƿ� �ڸ����� ����
					break;

				case false:
					//�ش� ��ġ�� ^(exclusive not) ����
					bits_8 ^= 0x1 << bit_digits;
					bit_digits--;
					break;
				}
			}

			if ((bit_unit + 1) % 8 == 0) //8bit(1byte)��ŭ ����Ǿ�����,
			{
				//1����Ʈ��ŭ�� ������ ���� ��� write_buffer�� ��� �� bits_8 �ʱ�ȭ
				write_buffer[write_buffer_idx_count] = bits_8; //�ش� ��Ʈ���� ���� ���ۿ� �Ҵ�
				bits_8 = SPARE_INIT_VALUE; //0xff�� �ʱ�ȭ
				bit_digits = 7; //���� �ʱ�ȭ�� ��Ʈ�� bits_8�� ��Ʈ ������ ���� �ڸ��� 2^7�� �ʱ�ȭ
				write_buffer_idx_count++; //write_buffer�� �ε��� ī���� ����(������ ��ϵ� �ε����� ���� ī��Ʈ)
			}
		}

		fwrite(write_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, (*storage_spare_pos));

		delete[] write_buffer;
	}
	else
		return FAIL;

	/*** trace���� ���� ��� ***/
	(*flashmem)->v_flash_info.flash_write_count++; //�÷��� �޸� ���� ī��Ʈ ����

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	(*flashmem)->block_trace_info[((ftell(*storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE) / BLOCK_PER_SECTOR].block_write_count++; //�ش� ����� ���� ī��Ʈ ����
#endif

	return SUCCESS;
}

int SPARE_write(class FlashMem** flashmem, unsigned int PSN, META_DATA** src_data) //META_DATA�� ���� Ŭ���� ���޹޾�, ���� ������ Spare Area�� ���
{
	FILE* storage_spare_pos = NULL;

	unsigned int write_pos = 0; //������ �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����

	if ((storage_spare_pos = fopen("storage.bin", "rb+")) == NULL) //�а� ���� ��� + �������� ���
	{
		fprintf(stderr, "storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (SPARE_write)");
		return FAIL;
	}

	write_pos = SECTOR_INC_SPARE_BYTE * PSN; //PSN ��ġ
	spare_pos = write_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�

	if (SPARE_write(flashmem, &storage_spare_pos, src_data) != SUCCESS)
	{
		/*** ���Ⱑ ������ ��� �÷��� �޸� ���� ī��Ʈ �������� FAIL return ***/
		fclose(storage_spare_pos);
		return FAIL;
	}

	fclose(storage_spare_pos);
	return SUCCESS;
}

META_DATA** SPARE_reads(class FlashMem** flashmem, unsigned int PBN) //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ���·� ��ȯ
{
	META_DATA** block_meta_data_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����
	block_meta_data_array = new META_DATA*[BLOCK_PER_SECTOR]; //��� �� ����(������)���� META_DATA �ּҸ� ���� �� �ִ� ���� ����(row)

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		unsigned int PSN = (PBN * BLOCK_PER_SECTOR) + offset_index;

		block_meta_data_array[offset_index] = new META_DATA; //�� ������ ���� META_DATA���¸� ���� �� �ִ� ���� ����(col)
		block_meta_data_array[offset_index] = SPARE_read(flashmem, PSN); //�� ���� ��� ���� �� ���� ������ ��ġ(������)�� ���� ���������� ����
	}
	/***
		< META_DATA Ŭ���� �迭�� ���� �޸� ���� >
		�÷��� �޸��� ������ ������ Victim Block ���� ���� ���� ���� ���� ���� ������ �Լ����� ���� ����
		---
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
			delete block_meta_data_array[offset_index];
		delete[] block_meta_data_array;
	***/

	return block_meta_data_array;
}

int SPARE_writes(class FlashMem** flashmem, unsigned int PBN, META_DATA** block_meta_data_array) //�� ���� ��� ���� ��� ����(������)�� ���� meta���� ���
{
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		unsigned int PSN = (PBN * BLOCK_PER_SECTOR) + offset_index;

		if (SPARE_write(flashmem, PSN, &block_meta_data_array[offset_index]) != SUCCESS)
			return FAIL;
	}

	return SUCCESS;
}

int update_victim_block_info(class FlashMem** flashmem, bool is_logical, unsigned int src_Block_num, int mapping_method) //Victim Block ������ ���� ��� ���� ����ü ����
{
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE;
	float LBN_invalid_ratio = -1;
	float PBN_invalid_ratio = -1; //PBN1 or PBN2 (���� ��Ͽ� ���� ��ȿ�� ���)
	float PBN1_invalid_ratio = -1;
	float PBN2_invalid_ratio = -1;

	META_DATA** block_meta_data_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����

	/***
		Victim Block ���� ����ü �ʱⰪ
		---
		victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
		victim_block_invalid_ratio = -1;
	***/
	//�ʱⰪ�� �ƴϸ�, Victim Block ������ ���� �ǳʶ�
	if ((*flashmem)->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE && (*flashmem)->victim_block_info.victim_block_invalid_ratio != -1)
		return FAIL;
	
	/***
		< Block Mapping >
		
		- Block Mapping���� Overwrite�߻� �� �ش� ����� �׻� ��ȿȭ�ȴ�. �̿� ����, GC���� ������ �ñ⿡ PBN�� Erase�����Ͽ��� ��
		=> Overwrite�߻� �� ��ȿ �����ʹ� ���ο� ���(������ �� Spare Block�� ���)���� copy�Ǿ���, ���� ����� �� �̻� ������� �����Ƿ�, 
		��ȿ�� ����� ���� ��ĳ���� ���� �ʰ� �׻� ��ȿ���� 1.0 ���� ����

		------------------------------------------------------------------

		< Hybrid Mapping (Log algorithm) >
		
		- Hybrid Mapping (Log algorithm)���� PBN1(Data Block) �Ǵ� PBN2(Log Block) �� �ϳ��� ���� ��ȿȭ�Ǵ� ������
		�� ��� ���� Ư�� �����¿� ���� �ݺ��� Overwrite�� �߻����� �ʰ�, ��� �����¿� ���Ͽ� Overwrite�� �߻��� ���

		- �̿� ����, Erase ������ LBN�� ������ PBN1(Data Block) �Ǵ� PBN2(Log Block) �� �ϳ��� ���� ��ȿȭ�Ǵ� ��� Victim Block���� �����Ǿ� GC�� ���� ó��
		
		- Merge�� �ϰ��� �Ѵٸ�, LBN�� PBN1(Data Block)�� PBN2(Log Block) ��� �����Ǿ� �־�� �ϸ�(��, ��ȿ�� ���), ���� ��� �Ϻ� ��ȿ �����͸� �����ϰ� �־�� �Ѵ�.

		- Merge ������, 
			1) PBN1(Data Block)�� ���� Overwrite�� �߻��Ͽ� PBN2(Log Block)�� ����Ͽ��� �ϴµ�, PBN2(Log Block)�� �� �̻� ����� ������ ���� ���
			: ���� �۾� �߿� FTL�Լ��� ���Ͽ� Merge ����

			2) ������ ���� ���� Ȯ��
			: GC�� ���� ���� ���� ���̺��� ���� ��ü ��ϵ� �� Merge ������ LBN�� ���ؼ� Merge ����
		
		- Hybrid Mapping (Log algorithm)���� GC�� ������ ������ ���� ����(Physical Remaining Space)�� ���� Block Invalid Ratio Threshold�� ����
			1) ���� ��ȿȭ�� PBN�� ��� Erase ����
			2) �Ϻ� ��ȿ �����͸� �����ϰ� �ִ� LBN�� ������ PBN1�� PBN2�� ���Ͽ� Merge ����
			=> ���� �۾��� �߻��� LBN�� PBN1�� PBN2�� ���� ���յ� ��ȿ�� �� ���
	***/

	switch (mapping_method)
	{
	case 2: //��� ����
		if (is_logical == true) //src_Block_num�� LBN�� ���
			return FAIL;
		else //src_Block_num�� PBN�� ���
		{
			(*flashmem)->victim_block_info.is_logical = false;
			(*flashmem)->victim_block_info.victim_block_num = PBN = src_Block_num;
			(*flashmem)->victim_block_info.victim_block_invalid_ratio = 1.0;
			
			goto BLOCK_MAPPING;
		}
			

	case 3: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical == true) //src_Block_num�� LBN�� ���
		{
			LBN = src_Block_num;
			PBN1 = (*flashmem)->log_block_level_mapping_table[LBN][0];
			PBN2 = (*flashmem)->log_block_level_mapping_table[LBN][1];

			goto HYBRID_LOG_LBN;
		}
		else //src_Block_num�� PBN�� ���
		{
			PBN = src_Block_num;
			(*flashmem)->victim_block_info.is_logical = false;
			
			goto HYBRID_LOG_PBN;
		}

	default:
		return FAIL;
	}

BLOCK_MAPPING:
	goto END_SUCCESS;

HYBRID_LOG_PBN: //PBN1 or PBN2 (���� ��Ͽ� ���� ��ȿ�� ���)
	(*flashmem)->victim_block_info.victim_block_num = PBN;

	/*** Calculate PBN Invalid Ratio ***/
	block_meta_data_array = SPARE_reads(flashmem, PBN); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	if (calc_block_invalid_ratio(block_meta_data_array, PBN_invalid_ratio) != SUCCESS)
	{
		fprintf(stderr, "���� : nullptr (block_meta_data_array)");
		system("pause");
		exit(1);
	}
	/*** Deallocate block_meta_data_array ***/
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		delete block_meta_data_array[offset_index];
	delete[] block_meta_data_array;
	block_meta_data_array = NULL;

	try
	{
		if (PBN_invalid_ratio >= 0 && PBN_invalid_ratio <= 1)
			(*flashmem)->victim_block_info.victim_block_invalid_ratio = PBN_invalid_ratio;
		else
			throw PBN_invalid_ratio;
	}
	catch (float PBN_invalid_ratio)
	{
		fprintf(stderr, "���� : �߸��� ��ȿ��(%f)", &PBN_invalid_ratio);
		system("pause");
		exit(1);
	}

	goto END_SUCCESS;

HYBRID_LOG_LBN:
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //���� �� ����Ǿ� ���� ������,
		goto NON_ASSIGNED_TABLE;

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //�ϳ��� �����Ǿ� ���� ������, ���� ��Ͽ� ���� ���� ����
	{
		(*flashmem)->victim_block_info.is_logical = false;
		
		//�����Ǿ� �ִ� ��Ͽ� ���� ���� ��� ó�� ��ƾ���� ����� PBN���� �Ҵ�
		if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE)
			PBN = PBN1;
		else
			PBN = PBN2;

		//���� ��� ó�� ��ƾ���� �̵�
		goto HYBRID_LOG_PBN;
	}

	/*** Calculate PBN1 Invalid Ratio ***/
	block_meta_data_array = SPARE_reads(flashmem, PBN1); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	if (calc_block_invalid_ratio(block_meta_data_array, PBN1_invalid_ratio) != SUCCESS)
	{
		fprintf(stderr, "���� : nullptr (block_meta_data_array)");
		system("pause");
		exit(1);
	}
	/*** Deallocate block_meta_data_array ***/
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		delete block_meta_data_array[offset_index];
	delete[] block_meta_data_array;

	/*** Calculate PBN2 Invalid Ratio ***/
	block_meta_data_array = SPARE_reads(flashmem, PBN2); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	if (calc_block_invalid_ratio(block_meta_data_array, PBN2_invalid_ratio) != SUCCESS)
	{
		fprintf(stderr, "���� : nullptr (block_meta_data_array)");
		system("pause");
		exit(1);
	}
	/*** Deallocate block_meta_data_array ***/
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		delete block_meta_data_array[offset_index];
	delete[] block_meta_data_array;
	block_meta_data_array = NULL;

	(*flashmem)->victim_block_info.is_logical = true;
	(*flashmem)->victim_block_info.victim_block_num = LBN;
	
	try
	{
		LBN_invalid_ratio = (float)((PBN1_invalid_ratio + PBN2_invalid_ratio) / 2); //LBN�� ��ȿ�� ���
		if (LBN_invalid_ratio >= 0 && LBN_invalid_ratio <= 1)
			(*flashmem)->victim_block_info.victim_block_invalid_ratio = LBN_invalid_ratio;
		else
			throw LBN_invalid_ratio;
	}
	catch (float LBN_invalid_ratio)
	{
		fprintf(stderr, "���� : �߸��� ��ȿ��(%f)", &LBN_invalid_ratio);
		system("pause");
		exit(1);
	}
	
	goto END_SUCCESS;
	
END_SUCCESS:
	return SUCCESS;

NON_ASSIGNED_TABLE:
	return COMPLETE;

}

int update_v_flash_info_for_erase(class FlashMem** flashmem, META_DATA** src_data) //Erase�ϰ��� �ϴ� Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� �÷��� �޸��� ������ ���� ����
{
	//for Remaining Space Management

	if (src_data != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
				//������� �ʰ�, ��ȿ�� �������̸�
			{
				(*flashmem)->v_flash_info.written_sector_count--; //��ϵ� ������ �� ����
			}
			else if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
			{
				//������� �ʰ�, ��ȿ���� ���� �������̸�
				(*flashmem)->v_flash_info.written_sector_count--; //��ϵ� ������ �� ����
				(*flashmem)->v_flash_info.invalid_sector_count--; //��ȿ ������ �� ����
			}
			else //������� ���, �׻� ��ȿ�� �������̴�
			{
				//do nothing
			}
		}
	}
	else
		return FAIL;

	return SUCCESS;
}

int update_v_flash_info_for_reorganization(class FlashMem** flashmem, META_DATA** src_data) //Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� ������ ���� ���� ���� ��� ���� ������ �÷��� �޸� ���� ����
{
	if (src_data != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
				//������� �ʰ�, ��ȿ�� �������̸�
			{
				(*flashmem)->v_flash_info.written_sector_count++; //��ϵ� ������ �� ����
			}
			else if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
			{
				//������� �ʰ�, ��ȿ���� ���� �������̸�
				(*flashmem)->v_flash_info.written_sector_count++; //��ϵ� ������ �� ����
				(*flashmem)->v_flash_info.invalid_sector_count++; //��ȿ ������ �� ����
			}
			else //������� ���, �׻� ��ȿ�� �������̴�
			{
				//do nothing
			}
		}
	}
	else
		return FAIL;

	return SUCCESS;
}

int calc_block_invalid_ratio(META_DATA** src_data, float& dst_block_invalid_ratio) //Ư�� ���� ��� �ϳ��� ���� META_DATA Ŭ���� �迭�� ���� �Ǻ��� �����Ͽ� ��ȿ�� ��� �� ����
{
	//for Calculate Block Invalid Ratio
	__int8 block_per_written_sector_count = 0;
	__int8 block_per_invalid_sector_count = 0;
	__int8 block_per_empty_sector_count = 0;

	if (src_data != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
				//������� �ʰ�, ��ȿ�� �������̸�
			{
				block_per_written_sector_count++; //��ϵ� ������ �� ����
			}
			else if (src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
				src_data[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
			{
				//������� �ʰ�, ��ȿ���� ���� �������̸�
				block_per_written_sector_count++; //��ϵ� ������ �� ����
				block_per_invalid_sector_count++; //��ȿ ������ �� ����
			}
			else //������� ���, �׻� ��ȿ�� �������̴�
			{
				block_per_empty_sector_count++; //�� ������ �� ����
			}
		}
	}
	else
		return FAIL;

	try
	{
		float block_invalid_ratio = (float)block_per_invalid_sector_count / (float)BLOCK_PER_SECTOR; //���� ����� ��ȿ�� ���
		if (block_invalid_ratio >= 0 && block_invalid_ratio <= 1)
			dst_block_invalid_ratio = block_invalid_ratio;
		else 
			throw block_invalid_ratio;
	}
	catch (float block_invalid_ratio)
	{
		fprintf(stderr, "���� : �߸��� ��ȿ��(%f)", &block_invalid_ratio);
		system("pause");
		exit(1);
	}

	return SUCCESS;
}

int search_empty_normal_block(class FlashMem** flashmem, unsigned int& dst_Block_num, META_DATA** dst_data, int mapping_method, int table_type) //�� �Ϲ� ���� ���(PBN)�� ���������� Ž���Ͽ� PBN�Ǵ� ���̺� �� LBN ��, �ش� PBN�� meta���� ����
{
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	switch (mapping_method) //���� ��Ŀ� ���� �ش� ó�� ��ġ�� �̵�
	{
	case 2: //��� ����
		if (table_type == 0) //��� ���� Static Table
			goto BLOCK_MAPPING_STATIC_PROC;

		else if (table_type == 1) //��� ���� Dynamic Table
			goto DYNAMIC_COMMON_PROC;

		else 
			goto WRONG_TABLE_TYPE_ERR;

	case 3: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		goto DYNAMIC_COMMON_PROC;

	default:
		goto WRONG_FUNC_CALL_ERR;
	}

BLOCK_MAPPING_STATIC_PROC: //��� ���� Static Table : ��� ���� ���� ���̺��� ���� �� LBN�� ������ PBN�� Spare ���� �Ǻ�, �� ��� ����Ž�� �� LBN����
	for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++) //���������� ����ִ� ����� ã�´� (���� ���̺��̹Ƿ� ���̺� ũ�⸸ŭ ����)
	{
		//���� ���̺��̹Ƿ� LBN������ ���Ͽ� ���̺��� ���Ͽ� �˻�
		PSN = ((*flashmem)->block_level_mapping_table[table_index] * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
		meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == true)
			//Spare ����� �ƴϸ�, ����ְ�, ��ȿ�� ����̸�
		{
			//LBN �� meta ���� ����
			dst_Block_num = table_index;
			(*dst_data) = meta_buffer; //dst_data ����

			return SUCCESS;
		}
		delete meta_buffer;
		meta_buffer = NULL;
	}
	//���� �� ����� ã�� ��������
	return COMPLETE;

DYNAMIC_COMMON_PROC: //Dynamic Table ���� ó�� ��ƾ : �� ���� ����� Spare ���� �Ǻ�, �� ��� ����Ž�� �� PBN����
	for (unsigned int block_index = 0; block_index < f_flash_info.block_size; block_index++) //���������� ����ִ� ����� ã�´� (Spare ����� ��ġ�� ���Ⱑ �Ͼ�� ���� �������̹Ƿ� ��� ��Ͽ� ���� ��ĳ��)
	{
		PSN = (block_index * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
		meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == true)
			//Spare ����� �ƴϸ�, ����ְ�, ��ȿ�� ����̸�
		{
			//PBN �� meta ���� ����
			dst_Block_num = block_index;
			(*dst_data) = meta_buffer;

			return SUCCESS;
		}
		delete meta_buffer;
		meta_buffer = NULL;
	}
	
	//���� �� ����� ã�� ��������
	return COMPLETE;

WRONG_TABLE_TYPE_ERR: //�߸��� ���̺� Ÿ��
	fprintf(stderr, "���� : �߸��� ���̺� Ÿ��\n");
	system("pause");
	exit(1);

WRONG_FUNC_CALL_ERR:
	fprintf(stderr, "���� : �߸��� �Լ� ȣ��\n");
	system("pause");
	exit(1);
}

int search_empty_offset_in_block(class FlashMem** flashmem, unsigned int src_PBN, __int8& dst_Poffset, META_DATA** dst_data) //�Ϲ� ���� ���(PBN) ���θ� ���������� Ž���Ͽ� Poffset ��, �ش� ��ġ�� meta���� ����
{
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ

	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++) //���������� ����ִ� �������� ã�´�
	{
		PSN = (src_PBN * BLOCK_PER_SECTOR) + offset_index;
		meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

		if (
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
			//����ְ�, ��ȿ�� �������̸�
		{
			//Poffset �� meta ���� ����
			dst_Poffset = offset_index;
			(*dst_data) = meta_buffer;

			return SUCCESS;
		}
		delete meta_buffer;
		meta_buffer = NULL;
	}

	//���� �� �������� ã�� ��������
	return FAIL;
}

void print_block_meta_info(class FlashMem** flashmem, bool is_logical, unsigned int src_Block_num, int mapping_method) //��� ���� ��� ����(������)�� meta ���� ���
{
	FILE* block_meta_output = NULL;

	META_DATA** block_meta_data_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE;

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if ((block_meta_output = fopen("block_meta_output.txt", "wt")) == NULL)
	{
		fprintf(stderr, "block_meta_output.txt ������ ������� �� �� �����ϴ�. (print_block_meta_info)");
		return;
	}
	
	switch (mapping_method)
	{
	case 2: //��� ����
		if (is_logical == true) //src_Block_num�� LBN�� ���
			goto BLOCK_LBN;
		else //src_Block_num�� PBN�� ���
			goto COMMON_PBN;

	case 3: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical == true) //src_Block_num�� LBN�� ���
			goto HYBRID_LOG_LBN;
		else //src_Block_num�� PBN�� ���
			goto COMMON_PBN;

	default:
		goto COMMON_PBN;
	}

COMMON_PBN: //PBN�� ���� ���� ó�� ��ƾ
	PBN = src_Block_num;

	if (PBN == DYNAMIC_MAPPING_INIT_VALUE || PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1))
		goto OUT_OF_RANGE;

	block_meta_data_array = SPARE_reads(flashmem, PBN); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	fprintf(block_meta_output, "===== PBN : %u =====", PBN);
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
		fprintf(block_meta_output, "not_spare_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] ? "true" : "false");
		fprintf(block_meta_output, "\nvalid_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] ? "true" : "false");
		fprintf(block_meta_output, "\nempty_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] ? "true" : "false");
		fprintf(block_meta_output, "\nvalid_sector : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] ? "true" : "false");
		fprintf(block_meta_output, "\nempty_sector : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] ? "true" : "false");
	}

	/*** Deallocate block_meta_data_array ***/
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		delete block_meta_data_array[offset_index];
	delete[] block_meta_data_array;

	goto END_SUCCESS;

BLOCK_LBN: //��� ���� LBN ó�� ��ƾ
	LBN = src_Block_num;
	PBN = (*flashmem)->block_level_mapping_table[LBN];
	
	//LBN ������ Spare Block ������ŭ ������ ������ ����
	if (LBN == DYNAMIC_MAPPING_INIT_VALUE || LBN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size) - 1))
		goto OUT_OF_RANGE;

	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
		goto NON_ASSIGNED_TABLE;

	if (PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //PBN�� �߸��� ������ �����Ǿ� ���� ���
		goto WRONG_ASSIGNED_TABLE;

	fprintf(block_meta_output, "===== LBN : %u =====\n", LBN);
	block_meta_data_array = SPARE_reads(flashmem, PBN); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	fprintf(block_meta_output, "===== PBN : %u =====", PBN);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
		fprintf(block_meta_output, "not_spare_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] ? "true" : "false");
		fprintf(block_meta_output, "\nvalid_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] ? "true" : "false");
		fprintf(block_meta_output, "\nempty_block : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] ? "true" : "false");
		fprintf(block_meta_output, "\nvalid_sector : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] ? "true" : "false");
		fprintf(block_meta_output, "\nempty_sector : ");
		fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] ? "true" : "false");
	}

	/*** Deallocate block_meta_data_array ***/
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		delete block_meta_data_array[offset_index];
	delete[] block_meta_data_array;

	goto END_SUCCESS;

HYBRID_LOG_LBN: //���̺긮�� ���� LBN ó�� ��ƾ
	LBN = src_Block_num;
	PBN1 = (*flashmem)->log_block_level_mapping_table[LBN][0];
	PBN2 = (*flashmem)->log_block_level_mapping_table[LBN][1];

	//LBN ������ Spare Block ������ŭ ������ ������ ����
	if (LBN == DYNAMIC_MAPPING_INIT_VALUE || LBN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size) - 1))
		goto OUT_OF_RANGE;

	//��� �����Ǿ� ���� ������
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto NON_ASSIGNED_TABLE;
	
	if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN1 > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1) ||
		PBN2 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //PBN1 �Ǵ� PBN2�� �߸��� ������ �����Ǿ� ���� ���
		goto WRONG_ASSIGNED_TABLE;

	fprintf(block_meta_output, "===== LBN : %u =====\n", LBN);
	if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE)
	{
		block_meta_data_array = SPARE_reads(flashmem, PBN1); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
		fprintf(block_meta_output, "===== PBN1 : %u =====", PBN1);

		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
			fprintf(block_meta_output, "not_spare_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] ? "true" : "false");
			fprintf(block_meta_output, "\nvalid_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] ? "true" : "false");
			fprintf(block_meta_output, "\nempty_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] ? "true" : "false");
			fprintf(block_meta_output, "\nvalid_sector : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] ? "true" : "false");
			fprintf(block_meta_output, "\nempty_sector : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] ? "true" : "false");
		}

		/*** Deallocate block_meta_data_array ***/
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
			delete block_meta_data_array[offset_index];
		delete[] block_meta_data_array;
		block_meta_data_array = NULL;
	}
	else
		fprintf(block_meta_output, "===== PBN1 : non-assigned =====");

	if (PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
	{
		block_meta_data_array = SPARE_reads(flashmem, PBN2); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
		fprintf(block_meta_output, "\n===== PBN2 : %u =====", PBN2);

		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
			fprintf(block_meta_output, "not_spare_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] ? "true" : "false");
			fprintf(block_meta_output, "\nvalid_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] ? "true" : "false");
			fprintf(block_meta_output, "\nempty_block : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] ? "true" : "false");
			fprintf(block_meta_output, "\nvalid_sector : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] ? "true" : "false");
			fprintf(block_meta_output, "\nempty_sector : ");
			fprintf(block_meta_output, block_meta_data_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] ? "true" : "false");
		}
		/*** Deallocate block_meta_data_array ***/
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
			delete block_meta_data_array[offset_index];
		delete[] block_meta_data_array;
	}
	else
		fprintf(block_meta_output, "\n===== PBN2 : non-assigned =====");

	goto END_SUCCESS;

END_SUCCESS:
	fclose(block_meta_output);
	printf(">> block_meta_output.txt\n");
	system("notepad block_meta_output.txt");
	return;

OUT_OF_RANGE: //���� �ʰ�
	fclose(block_meta_output);
	printf("out of range\n");
	return;

NON_ASSIGNED_TABLE: //�������� ���� ���̺�
	fclose(block_meta_output);
	printf("non-assigned table\n");
	return;

WRONG_ASSIGNED_TABLE: //LBN�� �߸� ������ PBN ����
	fclose(block_meta_output);
	fprintf(stderr, "���� : WRONG_ASSIGNED_TABLE\n");
	system("pause");
	exit(1);
}