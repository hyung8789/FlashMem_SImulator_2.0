#include "Build_Options.h"

// Spare Area�� ���� ��Ʈ ���� ó��, Meta-data �ǵ��� ���� �Լ� SPARE_init, SPARE_read, SPARE_write ����
// ������ ���� ���� ���� ������ Garbage Collection�� ���� SPARE_reads, update_victim_block_info, update_v_flash_info_for_reorganization, update_v_flash_info_for_erase ����
// Meta-data�� ���� �� �Ϲ� ���� ��� Ž�� �� Ư�� ���� ��� ���� �� ���� ������ Ž�� ���� search_empty_block, search_empty_offset_in_block ����

META_DATA::META_DATA()
{
	this->block_state = BLOCK_STATE::NORMAL_BLOCK_EMPTY;
	this->sector_state = SECTOR_STATE::EMPTY;
	this->block_update_state = UPDATE_STATE::INIT;
	this->sector_update_state = UPDATE_STATE::INIT;
}

META_DATA::~META_DATA()
{
}

BLOCK_STATE META_DATA::get_block_state()
{
	return this->block_state;
}

SECTOR_STATE META_DATA::get_sector_state()
{
	return this->sector_state;
}

UPDATE_STATE META_DATA::get_block_update_state()
{
	return this->block_update_state;
}

UPDATE_STATE META_DATA::get_sector_update_state()
{
	return this->sector_update_state;
}

void META_DATA::set_block_state(const BLOCK_STATE src_block_state)
{
	this->block_state = src_block_state;

	switch (this->block_update_state)
	{
	case UPDATE_STATE::INIT: //�ʱ� ������ �������κ��� �о���̱� �� ������ �� OUT_DATED ���·� ����
		this->block_update_state = UPDATE_STATE::OUT_DATED;
		break;

	case UPDATE_STATE::OUT_DATED: //������ �������κ��� �о���� ���¿��� ���� �� UPDATED ���·� ����
		this->block_update_state = UPDATE_STATE::UPDATED;\
		break;
	}
}

void META_DATA::set_sector_state(const SECTOR_STATE src_sector_state)
{
	this->sector_state = src_sector_state;

	switch (this->sector_update_state)
	{
	case UPDATE_STATE::INIT: //�ʱ� ������ �������κ��� �о���̱� �� ������ �� OUT_DATED ���·� ����
		this->sector_update_state = UPDATE_STATE::OUT_DATED;
		break;

	case UPDATE_STATE::OUT_DATED: //������ �������κ��� �о���� ���¿��� ���� �� UPDATED ���·� ����
		this->sector_update_state = UPDATE_STATE::UPDATED;
		break;
	}
}

int SPARE_init(class FlashMem*& flashmem, FILE*& storage_spare_pos) //���� ����(������)�� Spare Area�� ���� �ʱ�ȭ
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned char* write_buffer = NULL; //��ü Spare Area�� ����� ����

	if (storage_spare_pos != NULL)
	{
		write_buffer = new unsigned char[SPARE_AREA_BYTE];
		memset(write_buffer, SPARE_INIT_VALUE, SPARE_AREA_BYTE);
		fwrite(write_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, storage_spare_pos);

		delete[] write_buffer;
	}
	else
		goto NULL_FILE_PTR_ERR;

	return SUCCESS;

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : nullptr (SPARE_init)\n");
	system("pause");
	exit(1);
}

int SPARE_read(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& dst_meta_buffer) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ���·� ��ȯ
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	
	unsigned char* read_buffer = NULL; //��ü Spare Area�κ��� �о���� ����

	if (storage_spare_pos != NULL)
	{
		if (dst_meta_buffer != NULL)
			goto MEM_LEAK_ERR;

		dst_meta_buffer = new META_DATA(); //�������� META_DATA ����

		read_buffer = new unsigned char[SPARE_AREA_BYTE];
		fread(read_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, storage_spare_pos);

		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/
		unsigned char bits_8_buffer = read_buffer[0]; //1byte == 8bitũ���� ��� �� ����(������) ������ ���Ͽ� Spare Area�� �о���� ���� 

		/*** �о���� 8��Ʈ(2^7 ~2^0)�� ���ؼ� ��� ����(2^7 ~ 2^5) �Ǻ� ***/
		switch ((((bits_8_buffer) >> (5)) & (0x7))) //���� ������ 2^5 �ڸ��� LSB�� ������ ���������� 5�� ����Ʈ�Ͽ�, 00000111(2)�� AND ����
		{
		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_EMPTY:
			dst_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_EMPTY);
			break;

		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_VALID:
			dst_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);
			break;

		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_INVALID:
			dst_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_INVALID);
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_EMPTY:
			dst_meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_EMPTY);
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_VALID:
			dst_meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_VALID);
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_INVALID:
			dst_meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_INVALID);
			break;

		default:
			printf("Block State Err\n");
			bit_disp(bits_8_buffer, 7, 0);
			goto WRONG_META_ERR;
		}

		/*** �о���� 8��Ʈ(2^7 ~2^0)�� ���ؼ� ���� ����(2^4 ~ 2^3) �Ǻ� ***/
		switch ((((bits_8_buffer) >> (3)) & (0x3))) //���� ������ 2^3 �ڸ��� LSB�� ������ ���������� 3�� ����Ʈ�Ͽ�, 00000011(2)�� AND ����
		{
		case (const unsigned)SECTOR_STATE::EMPTY:
			dst_meta_buffer->set_sector_state(SECTOR_STATE::EMPTY);
			break;

		case (const unsigned)SECTOR_STATE::VALID:
			dst_meta_buffer->set_sector_state(SECTOR_STATE::VALID);
			break;

		case (const unsigned)SECTOR_STATE::INVALID:
			dst_meta_buffer->set_sector_state(SECTOR_STATE::INVALID);
			break;

		default:
			printf("Sector State Err\n");
			bit_disp(bits_8_buffer, 7,0);
			goto WRONG_META_ERR;
		}

		/*** DUMMY 3��Ʈ ó�� (2^2 ~ 2^0) ***/
		switch ((bits_8_buffer) &= (0x7)) //00000111(2)�� AND ����
		{
		case (0x7): //111(2)�� �ƴ� ��� ����
			break;
			
		default:
			printf("DUMMY bit Err\n");
			bit_disp(bits_8_buffer, 7, 0);
			goto WRONG_META_ERR;
		}
		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/

		//��Ÿ Meta ���� �߰� �� �о ó�� �� �ڵ� �߰�
	}
	else
		goto NULL_FILE_PTR_ERR;

	delete[] read_buffer;

	/*** trace���� ���� ��� ***/
	flashmem->v_flash_info.flash_read_count++; //Global �÷��� �޸� �б� ī��Ʈ ����

#ifdef PAGE_TRACE_MODE //Trace for Per Sector(Page) Wear-leveling
	/***
		���� �бⰡ �߻��� PSN ���, Spare Area�� ���� ó���� ���� ��
		ftell(���� ���� ������, ����Ʈ ����) - SECTOR_INC_SPARE_BYTE == Flash_read���� read_pos
		PSN = read_pos / SECTOR_INC_SPARE_BYTE
	***/
	flashmem->page_trace_info[((ftell(storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE)].read_count++; //�ش� ����(������)�� ����� ī��Ʈ ����
#endif

#ifdef BLOCK_TRACE_MODE //Trace for Per Block Wear-leveling
	/***
		���� �бⰡ �߻��� PBN ���, Spare Area�� ���� ó���� ���� ��
		ftell(���� ���� ������, ����Ʈ ����) - SECTOR_INC_SPARE_BYTE == Flash_read���� read_pos
		PSN = read_pos / SECTOR_INC_SPARE_BYTE
		PBN = PSN / BLOCK_PER_SECTOR
	***/
	flashmem->block_trace_info[((ftell(storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE) / BLOCK_PER_SECTOR].read_count++; //�ش� ����� �б� ī��Ʈ ����
#endif

	return SUCCESS;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (SPARE_read)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (SPARE_read)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : nullptr (SPARE_read)\n");
	system("pause");
	exit(1);
}

int SPARE_read(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& dst_meta_buffer) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ���·� ��ȯ
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	FILE* storage_spare_pos = NULL;

	unsigned int read_pos = 0; //�а��� �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����

	if (dst_meta_buffer != NULL)
		goto MEM_LEAK_ERR;

	if ((storage_spare_pos = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
		goto NULL_FILE_PTR_ERR;

	read_pos = SECTOR_INC_SPARE_BYTE * PSN; //������ �ϴ� ��ġ
	spare_pos = read_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�

	if (SPARE_read(flashmem, storage_spare_pos, dst_meta_buffer) != SUCCESS)
	{
		/*** �÷��� �޸𸮰� �Ҵ���� �ʾ� �бⰡ ������ ��� �÷��� �޸� �б� ī��Ʈ �������� FAIL return ***/
		fclose(storage_spare_pos);
		return FAIL;
	}

	fclose(storage_spare_pos);
	return SUCCESS;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (SPARE_read)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �б���� �� �� �����ϴ�. (SPARE_read)\n");
	system("pause");
	exit(1);
}

int SPARE_write(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& src_meta_buffer) //META_DATA ���޹޾�, ���� ������ Spare Area�� ���
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned char* write_buffer = NULL; //��ü Spare Area�� ��� �� ����
	unsigned bits_8_buffer; //8��Ʈ(1����Ʈ) ������ �и��Ͽ� �ǵ� ���� ����

	if (storage_spare_pos != NULL)
	{
		if (src_meta_buffer == NULL)
			goto NULL_SRC_META_ERR;

		/*** for Remaining Space Management ***/
		//�ش� ���Ͱ� ��ȿȭ �Ǿ���, �ֽ� ���¶�� ��ȿ ī��Ʈ�� ������Ų��
		if (src_meta_buffer->get_sector_state() == SECTOR_STATE::INVALID && src_meta_buffer->get_sector_update_state() == UPDATE_STATE::UPDATED)
			flashmem->v_flash_info.invalid_sector_count++; //��ȿ ������ �� ����

		write_buffer = new unsigned char[SPARE_AREA_BYTE];
		memset(write_buffer, SPARE_INIT_VALUE, SPARE_AREA_BYTE);

		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/
		//BLOCK_TYPE(Normal or Spare, 1bit) || IS_VALID (BLOCK, 1bit) || IS_EMPTY (BLOCK, 1bit) || IS_VALID (SECTOR, 1bit) || IS_EMPTY(SECTOR, 1bit) || DUMMY (3bit)
		bits_8_buffer = ~(SPARE_INIT_VALUE); //00000000(2)

		switch (src_meta_buffer->get_block_state()) //1����Ʈ ũ���� bits_8_buffer�� ���Ͽ�
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //2^7, 2^6, 2^5 ��Ʈ�� 0x1���� ����
			bits_8_buffer |= (0x7 << 5); //111(2)�� 5�� ���� ����Ʈ�Ͽ� 11100000(2)�� OR ����
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //2^7, 2^6 ��Ʈ�� 0x1�� ����
			bits_8_buffer |= (0x6 << 5); //110(2)�� 5�� ���� ����Ʈ�Ͽ� 11000000(2)�� OR ����
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //2^7 ��Ʈ�� 0x1�� ����
			bits_8_buffer |= (TRUE_BIT << 7); //10000000(2)�� OR ����
			break;

		case BLOCK_STATE::SPARE_BLOCK_EMPTY: //2^6, 2^5 ��Ʈ�� 0x1�� ����
			bits_8_buffer |= (0x6 << 4); //110(2)�� 4�� ���� ����Ʈ�Ͽ� 01100000(2)�� OR ����
			break;

		case BLOCK_STATE::SPARE_BLOCK_VALID: //2^6 ��Ʈ�� 0x1�� ����
			bits_8_buffer |= (TRUE_BIT << 6); //01000000(2)�� OR ����
			break;

		case BLOCK_STATE::SPARE_BLOCK_INVALID: //do nothing
			break;

		default:
			printf("Block State Err\n");
			bit_disp(bits_8_buffer, 7, 0);
			goto WRONG_META_ERR;
		}

		switch (src_meta_buffer->get_sector_state()) //1����Ʈ ũ���� bits_8_buffer�� ���Ͽ�
		{
		case SECTOR_STATE::EMPTY: //2^4, 2^3 ��Ʈ�� 0x1�� ����
			bits_8_buffer |= (0x3 << 3); //11(2)�� 3�� ���� ����Ʈ�Ͽ� 00011000(2)�� OR ����
			break;

		case SECTOR_STATE::VALID: //2^4��Ʈ�� 0x1�� ����
			bits_8_buffer |= (TRUE_BIT << 4); //00010000(2)�� OR ����
			break;

		case SECTOR_STATE::INVALID: //do nothing
			break;

		default:
			printf("Sector State Err\n");
			bit_disp(bits_8_buffer, 7, 0);
			goto WRONG_META_ERR;
		}

		//DUMMY 3��Ʈ ó�� (2^2 ~ 2^0)
		bits_8_buffer |= (0x7); //00000111(2)�� OR ����

		write_buffer[0] = bits_8_buffer;

#ifdef DEBUG_MODE
		bit_disp(write_buffer[0], 7, 0);
#endif
		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/

		//��Ÿ Meta ���� �߰� �� ��� �� �ڵ� �߰�
	}
	else
		goto NULL_FILE_PTR_ERR;

	fwrite(write_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, storage_spare_pos);
	delete[] write_buffer;

	/*** trace���� ���� ��� ***/
	flashmem->v_flash_info.flash_write_count++; //Global �÷��� �޸� ���� ī��Ʈ ����

#ifdef PAGE_TRACE_MODE //Trace for Per Sector(Page) Wear-leveling
	/***
		���� ���Ⱑ �߻��� PSN ���, Spare Area�� ���� ó���� ���� ��
		ftell(���� ���� ������, ����Ʈ ����) - SECTOR_INC_SPARE_BYTE == Flash_write���� write_pos
		PSN = write_pos / SECTOR_INC_SPARE_BYTE
	***/
	flashmem->page_trace_info[((ftell(storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE)].write_count++; //�ش� ����(������)�� ����� ī��Ʈ ����
#endif

#ifdef BLOCK_TRACE_MODE //Trace for Per Block Wear-leveling
	/***
		���� ���Ⱑ �߻��� PBN ���, Spare Area�� ���� ó���� ���� ��
		ftell(���� ���� ������, ����Ʈ ����) - SECTOR_INC_SPARE_BYTE == Flash_write���� write_pos
		PSN = write_pos / SECTOR_INC_SPARE_BYTE
		PBN = PSN / BLOCK_PER_SECTOR
	***/
	flashmem->block_trace_info[((ftell(storage_spare_pos) - SECTOR_INC_SPARE_BYTE) / SECTOR_INC_SPARE_BYTE) / BLOCK_PER_SECTOR].write_count++; //�ش� ����� ���� ī��Ʈ ����
#endif

	return SUCCESS;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (SPARE_write)\n");
	system("pause");
	exit(1);

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ��� ���� meta ������ �������� ���� (SPARE_write)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : nullptr (SPARE_write)\n");
	system("pause");
	exit(1);
}

int SPARE_write(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& src_meta_buffer) //META_DATA ���޹޾�, ���� ������ Spare Area�� ���
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	FILE* storage_spare_pos = NULL;

	unsigned int write_pos = 0; //������ �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����

	if (src_meta_buffer == NULL)
		goto NULL_SRC_META_ERR;

	if ((storage_spare_pos = fopen("storage.bin", "rb+")) == NULL) //�а� ���� ��� + �������� ���
		goto NULL_FILE_PTR_ERR;

	write_pos = SECTOR_INC_SPARE_BYTE * PSN; //PSN ��ġ
	spare_pos = write_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�

	if (SPARE_write(flashmem, storage_spare_pos, src_meta_buffer) != SUCCESS)
	{
		/*** �÷��� �޸𸮰� �Ҵ���� �ʾ� ���Ⱑ ������ ��� �÷��� �޸� ���� ī��Ʈ �������� FAIL return ***/
		fclose(storage_spare_pos);
		return FAIL;
	}

	fclose(storage_spare_pos);
	return SUCCESS;

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ��� ���� meta ������ �������� ���� (SPARE_write)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (SPARE_write)\n");
	system("pause");
	exit(1);
}

/*** Depending on Spare area processing function ***/
//for Remaining Space Management and Garbage Collection
int SPARE_reads(class FlashMem*& flashmem, unsigned int PBN, META_DATA**& dst_block_meta_buffer_array) //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA �迭 ���·� ��ȯ
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	FILE* storage_spare_pos = NULL;

	unsigned int read_pos = 0; //�а��� �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����
	__int8 offset_index_dump = OFFSET_MAPPING_INIT_VALUE; //debug

	if (dst_block_meta_buffer_array != NULL)
		goto MEM_LEAK_ERR;

	dst_block_meta_buffer_array = new META_DATA * [BLOCK_PER_SECTOR](); //��� �� ����(������)���� META_DATA �ּҸ� ���� �� �ִ� ����(row)

	if ((storage_spare_pos = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
		goto NULL_FILE_PTR_ERR;

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		dst_block_meta_buffer_array[offset_index] = NULL; //���� �ʱ�ȭ

		read_pos = ((SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR) * PBN) + (SECTOR_INC_SPARE_BYTE * offset_index);
		spare_pos = read_pos + SECTOR_PER_BYTE; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)
		fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�

		if (SPARE_read(flashmem, storage_spare_pos, dst_block_meta_buffer_array[offset_index]) != SUCCESS) //�� ���� ��� ���� �� ���� ������ ��ġ(������)�� ���� ���������� ����(col)
		{
			offset_index_dump = offset_index;
			goto READ_BLOCK_META_ERR;
		}
	}

	fclose(storage_spare_pos);
	return SUCCESS;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (SPARE_read)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �б���� �� �� �����ϴ�. (SPARE_read)\n");
	system("pause");
	exit(1);

READ_BLOCK_META_ERR:
	fprintf(stderr, "ġ���� ���� :  PBN : %u �� Offset : %d ���� meta ���� �б� ���� (SPARE_reads)\n", PBN, offset_index_dump);
	system("pause");
	exit(1);
}

int SPARE_writes(class FlashMem*& flashmem, unsigned int PBN, META_DATA**& src_block_meta_buffer_array) //�� ���� ��� ���� ��� ����(������)�� ���� meta���� ���
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	FILE* storage_spare_pos = NULL;

	unsigned int write_pos = 0; //������ �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����
	__int8 offset_index_dump = OFFSET_MAPPING_INIT_VALUE; //debug

	if (src_block_meta_buffer_array == NULL)
		goto NULL_SRC_META_ERR;

	if ((storage_spare_pos = fopen("storage.bin", "rb+")) == NULL) //�а� ���� ��� + �������� ���
		goto NULL_FILE_PTR_ERR;

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		write_pos = ((SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR) * PBN) + (SECTOR_INC_SPARE_BYTE * offset_index);
		spare_pos = write_pos + SECTOR_PER_BYTE; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)
		fseek(storage_spare_pos, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�

		if (SPARE_write(flashmem, storage_spare_pos, src_block_meta_buffer_array[offset_index]) != SUCCESS)
		{
			offset_index_dump = offset_index;
			goto WRITE_BLOCK_META_ERR;
		}
	
	}
	fclose(storage_spare_pos);
	return SUCCESS;

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ��� ���� meta ������ �������� ���� (SPARE_write)\n");
	system("pause");
	exit(1);

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (SPARE_write)\n");
	system("pause");
	exit(1);

WRITE_BLOCK_META_ERR:
	fprintf(stderr, "ġ���� ���� :  PBN : %u �� Offset : %d �� ���� meta ���� ���� ���� (SPARE_writes)\n", PBN, offset_index_dump);
	system("pause");
	exit(1);
}

int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, enum VICTIM_BLOCK_PROC_STATE proc_state, unsigned int src_block_num, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ������ ���� ��� ���� ���� �� GC �����ٷ� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE;

	//���� ó������ ���� Victim Block ������ �����ϸ� ġ���� ����
	if (flashmem->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE)
		goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

	switch (proc_state)
	{
	case VICTIM_BLOCK_PROC_STATE::UNLINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::UNLINKED;
		break;

	case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::SPARE_LINKED;
		break;

	case VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE;
		break;

	default:
		goto WRONG_VICTIM_BLOCK_PROC_STATE;
	}

	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (is_logical) //src_block_num�� LBN�� ���
			return FAIL;
		else //src_block_num�� PBN�� ���
		{
			flashmem->victim_block_info.is_logical = false;
			flashmem->victim_block_info.victim_block_num = PBN = src_block_num;

			goto BLOCK_MAPPING;
		}


	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical) //src_block_num�� LBN�� ���
		{
			LBN = src_block_num;
			PBN1 = flashmem->log_block_level_mapping_table[LBN][0];
			PBN2 = flashmem->log_block_level_mapping_table[LBN][1];

			goto HYBRID_LOG_LBN;
		}
		else //src_block_num�� PBN�� ���
		{
			PBN = src_block_num;
			flashmem->victim_block_info.is_logical = false;

			goto HYBRID_LOG_PBN;
		}

	default:
		return FAIL;
	}

BLOCK_MAPPING:
	goto END_SUCCESS;

HYBRID_LOG_PBN: //PBN1 or PBN2
	flashmem->victim_block_info.victim_block_num = PBN;

	goto END_SUCCESS;

HYBRID_LOG_LBN:
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //���� �� �����Ǿ� ���� ������
		goto NON_ASSIGNED_LBN;

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //�ϳ��� �����Ǿ� ���� ������, ���� ��Ͽ� ���� ���� ����
	{
		flashmem->victim_block_info.is_logical = false;

		//�����Ǿ� �ִ� ��Ͽ� ���� ���� ��� ó�� ��ƾ���� ����� PBN���� �Ҵ�
		if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE)
			PBN = PBN1;
		else
			PBN = PBN2;

		goto HYBRID_LOG_PBN; //���� ��� ó�� ��ƾ���� �̵�
	}

	flashmem->victim_block_info.is_logical = true;
	flashmem->victim_block_info.victim_block_num = LBN;

	goto END_SUCCESS;

END_SUCCESS:
	flashmem->gc->scheduler(flashmem, mapping_method, table_type); //���� �Ϸ�� Victim Block ���� ó���� ���� GC �����ٷ� ����
	return SUCCESS;

NON_ASSIGNED_LBN:
	return COMPLETE;

VICTIM_BLOCK_INFO_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ó������ ���� Victim Block ���� (update_victim_block_info)\n");
	system("pause");
	exit(1);

WRONG_VICTIM_BLOCK_PROC_STATE:
	fprintf(stderr, "ġ���� ���� : Wrong VICTIM_BLOCK_PROC_STATE (update_victim_block_info)\n");
	system("pause");
	exit(1);
}

int update_v_flash_info_for_reorganization(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array) //Ư�� ���� ��� �ϳ��� ���� META_DATA �迭�� ���� �Ǻ��� �����Ͽ� ������ ���� ���� ���� ��� ���� ������ �÷��� �޸� ���� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	if (src_block_meta_buffer_array != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			switch (src_block_meta_buffer_array[Poffset]->get_sector_state())
			{
			case SECTOR_STATE::EMPTY:  //������� ���
				//do nothing
				break;

			case SECTOR_STATE::VALID: //������� �ʰ�, ��ȿ�� �������̸�
				flashmem->v_flash_info.written_sector_count++; //��ϵ� ������ �� ����
				break;

			case SECTOR_STATE::INVALID: //������� �ʰ�, ��ȿ���� ���� �������̸�
				flashmem->v_flash_info.written_sector_count++; //��ϵ� ������ �� ����
				flashmem->v_flash_info.invalid_sector_count++; //��ȿ ������ �� ����
				break;	
			}
		}
	}
	else
		goto NULL_SRC_META_ERR;

	return SUCCESS;

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ������ �÷��� �޸� ���� ���� ���� meta ������ �������� ���� (update_v_flash_info_for_reorganization)\n");
	system("pause");
	exit(1);
}

int update_v_flash_info_for_erase(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array) //Erase�ϰ��� �ϴ� Ư�� ���� ��� �ϳ��� ���� META_DATA �迭�� ���� �Ǻ��� �����Ͽ� �÷��� �޸��� ������ ���� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	//for Remaining Space Management
	if (src_block_meta_buffer_array != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			switch (src_block_meta_buffer_array[Poffset]->get_sector_state())
			{
			case SECTOR_STATE::EMPTY:  //������� ���
				//do nothing
				break;

			case SECTOR_STATE::VALID: //������� �ʰ�, ��ȿ�� �������̸�
				flashmem->v_flash_info.written_sector_count--; //��ϵ� ������ �� ����
				break;

			case SECTOR_STATE::INVALID: //������� �ʰ�, ��ȿ���� ���� �������̸�
				flashmem->v_flash_info.written_sector_count--; //��ϵ� ������ �� ����
				flashmem->v_flash_info.invalid_sector_count--; //��ȿ ������ �� ����
				break;
			}
		}
	}
	else
		goto NULL_SRC_META_ERR;

	return SUCCESS;

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ������ �÷��� �޸� ���� ���� ���� meta ������ �������� ���� (update_v_flash_info_for_erase)\n");
	system("pause");
	exit(1);
}

int search_empty_offset_in_block(class FlashMem*& flashmem, unsigned int src_PBN, __int8& dst_Poffset, META_DATA*& dst_meta_buffer, enum MAPPING_METHOD mapping_method) //�Ϲ� ���� ���(PBN) ���θ� �������� ����ִ� ��ġ Ž��, Poffset ��, �ش� ��ġ�� meta���� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta ������ ���� �о���� ����
	META_DATA* meta_buffer_array[BLOCK_PER_SECTOR] = { NULL, }; //���� Ž������ ������ meta ���� �Ҵ� ������ ���� ����
	__int8 low, mid, high, current_empty_index;

	switch (flashmem->search_mode)
	{
	case SEARCH_MODE::SEQ_SEARCH: //���� Ž�� : O(n)
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++) //���������� ����ִ� �������� ã�´�
		{
			PSN = (src_PBN * BLOCK_PER_SECTOR) + offset_index;
			SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

			if (meta_buffer->get_sector_state() == SECTOR_STATE::EMPTY) //����ְ�, ��ȿ�� �������̸�
			{
				//Poffset �� meta ���� ����
				dst_Poffset = offset_index;
				dst_meta_buffer = meta_buffer;

				return SUCCESS;
			}

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
		break;

	case SEARCH_MODE::BINARY_SEARCH: //���� Ž�� : O(log n)
		if (mapping_method != MAPPING_METHOD::HYBRID_LOG) //������ ���� ������ ��� �� ��쿡��, ���� Ž�� ��� ����
			goto WRONG_BINARY_SEARCH_MODE_ERR;

		low = 0;
		high = BLOCK_PER_SECTOR - 1;
		mid = get_rand_offset(); //Wear-leveling�� ���� �ʱ� ������ Ž�� ��ġ ����
		current_empty_index = OFFSET_MAPPING_INIT_VALUE;

		while (low <= high)
		{
			PSN = (src_PBN * BLOCK_PER_SECTOR) + mid; //Ž�� ��ġ
			SPARE_read(flashmem, PSN, meta_buffer_array[mid]); //Spare ������ ����

			if (meta_buffer_array[mid]->get_sector_state() == SECTOR_STATE::EMPTY) //���������
			{
				//�������� Ž��
				current_empty_index = mid;
				high = mid - 1;
			}
			else //������� ������
			{
				//������ ��� ������� ����
				//���������� Ž��
				low = mid + 1;
			}
			
			mid = (low + high) / 2;
		}

		//Ž�� �Ϸ� �� �� ���Ͱ� �����ϴ� ���, meta ���� ���� ���� ���� �� ������ ��ġ�� meta ������ ������ �������� �޸� ����
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			if (current_empty_index != OFFSET_MAPPING_INIT_VALUE && offset_index == current_empty_index) //���� Ž�� ��ġ�� ���ؼ� ������ ���� ���ܵ�
				continue;

			//���� Ž�� ��ġ�� �ƴϰ� �о���� �����Ͱ� �����ϸ� ����
			if(meta_buffer_array[offset_index] != NULL) 
				if (deallocate_single_meta_buffer(meta_buffer_array[offset_index]) != SUCCESS) 
					goto MEM_LEAK_ERR;
		}

		if (current_empty_index != OFFSET_MAPPING_INIT_VALUE)
		{
			//Poffset �� meta ���� ����
			dst_Poffset = current_empty_index;
			dst_meta_buffer = meta_buffer_array[current_empty_index];

			return SUCCESS;
		}
		break;
	}

	//���� �� �������� ã�� ��������
	return FAIL;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (search_empty_offset_in_block)\n");
	system("pause");
	exit(1);

WRONG_BINARY_SEARCH_MODE_ERR:
	fprintf(stderr, "ġ���� ���� : ������ ���� ������ ��� �� ��쿡��, ���� Ž�� ��� ���� (search_empty_offset_in_block)\n");
	system("pause");
	exit(1);
}

int print_block_meta_info(class FlashMem*& flashmem, bool is_logical, unsigned int src_block_num, enum MAPPING_METHOD mapping_method) //��� ���� ��� ����(������)�� meta ���� ���
{
	FILE* block_meta_output = NULL;

	META_DATA** block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE;

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if ((block_meta_output = fopen("block_meta_output.txt", "wt")) == NULL)
	{
		fprintf(stderr, "block_meta_output.txt ������ ������� �� �� �����ϴ�. (print_block_meta_info)");
		return FAIL; //�������� ������ �ܼ� ��¸� ���� �ϹǷ�, ���и� ��ȯ
	}

	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (is_logical) //src_block_num�� LBN�� ���
			goto BLOCK_LBN;
		else //src_block_num�� PBN�� ���
			goto COMMON_PBN;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical) //src_block_num�� LBN�� ���
			goto HYBRID_LOG_LBN;
		else //src_block_num�� PBN�� ���
			goto COMMON_PBN;

	default:
		goto COMMON_PBN;
	}

COMMON_PBN: //PBN�� ���� ���� ó�� ��ƾ
	PBN = src_block_num;

	if (PBN == DYNAMIC_MAPPING_INIT_VALUE || PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1))
		goto OUT_OF_RANGE;

	SPARE_reads(flashmem, PBN, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	fprintf(block_meta_output, "===== PBN : %u =====", PBN);
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		printf("\n< Offset : %d >\n", offset_index);
		fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);

		printf("Block State : ");
		switch (block_meta_buffer_array[offset_index]->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY:
			fprintf(block_meta_output, "NORMAL_BLOCK_EMPTY\n");
			printf("NORMAL_BLOCK_EMPTY\n");
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID:
			fprintf(block_meta_output, "NORMAL_BLOCK_VALID\n");
			printf("NORMAL_BLOCK_VALID\n");
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID:
			fprintf(block_meta_output, "NORMAL_BLOCK_INVALID\n");
			printf("NORMAL_BLOCK_INVALID\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_EMPTY:
			fprintf(block_meta_output, "SPARE_BLOCK_EMPTY\n");
			printf("SPARE_BLOCK_EMPTY\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_VALID:
			fprintf(block_meta_output, "SPARE_BLOCK_VALID\n");
			printf("SPARE_BLOCK_VALID\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_INVALID:
			fprintf(block_meta_output, "SPARE_BLOCK_INVALID\n");
			printf("SPARE_BLOCK_INVALID\n");
			break;
		}

		printf("Sector State : ");
		switch (block_meta_buffer_array[offset_index]->get_sector_state())
		{
		case SECTOR_STATE::EMPTY:
			fprintf(block_meta_output, "EMPTY\n");
			printf("EMPTY\n");
			break;

		case SECTOR_STATE::VALID:
			fprintf(block_meta_output, "VALID\n");
			printf("VALID\n");
			break;

		case SECTOR_STATE::INVALID:
			fprintf(block_meta_output, "INVALID\n");
			printf("INVALID\n");
			break;
		}
	}

	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	goto END_SUCCESS;

BLOCK_LBN: //��� ���� LBN ó�� ��ƾ
	LBN = src_block_num;
	PBN = flashmem->block_level_mapping_table[LBN];

	//LBN ������ Spare Block ������ŭ ������ ������ ����
	if (LBN == DYNAMIC_MAPPING_INIT_VALUE || LBN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size) - 1))
		goto OUT_OF_RANGE;

	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
		goto NON_ASSIGNED_LBN;

	if (PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //PBN�� �߸��� ������ �����Ǿ� ���� ���
		goto WRONG_ASSIGNED_LBN_ERR;

	fprintf(block_meta_output, "===== LBN : %u =====\n", LBN);
	SPARE_reads(flashmem, PBN, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	fprintf(block_meta_output, "===== PBN : %u =====", PBN);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		printf("\n< Offset : %d >\n", offset_index);
		fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
		
		printf("Block State : ");
		switch (block_meta_buffer_array[offset_index]->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY:
			fprintf(block_meta_output, "NORMAL_BLOCK_EMPTY\n");
			printf("NORMAL_BLOCK_EMPTY\n");
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID:
			fprintf(block_meta_output, "NORMAL_BLOCK_VALID\n");
			printf("NORMAL_BLOCK_VALID\n");
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID:
			fprintf(block_meta_output, "NORMAL_BLOCK_INVALID\n");
			printf("NORMAL_BLOCK_INVALID\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_EMPTY:
			fprintf(block_meta_output, "SPARE_BLOCK_EMPTY\n");
			printf("SPARE_BLOCK_EMPTY\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_VALID:
			fprintf(block_meta_output, "SPARE_BLOCK_VALID\n");
			printf("SPARE_BLOCK_VALID\n");
			break;

		case BLOCK_STATE::SPARE_BLOCK_INVALID:
			fprintf(block_meta_output, "SPARE_BLOCK_INVALID\n");
			printf("SPARE_BLOCK_INVALID\n");
			break;
		}

		printf("Sector State : ");
		switch (block_meta_buffer_array[offset_index]->get_sector_state())
		{
		case SECTOR_STATE::EMPTY:
			fprintf(block_meta_output, "EMPTY\n");
			printf("EMPTY\n");
			break;

		case SECTOR_STATE::VALID:
			fprintf(block_meta_output, "VALID\n");
			printf("VALID\n");
			break;

		case SECTOR_STATE::INVALID:
			fprintf(block_meta_output, "INVALID\n");
			printf("INVALID\n");
			break;
		}
	}

	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	goto END_SUCCESS;

HYBRID_LOG_LBN: //���̺긮�� ���� LBN ó�� ��ƾ
	LBN = src_block_num;
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0];
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1];

	//LBN ������ Spare Block ������ŭ ������ ������ ����
	if (LBN == DYNAMIC_MAPPING_INIT_VALUE || LBN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size) - 1))
		goto OUT_OF_RANGE;

	//��� �����Ǿ� ���� ������
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto NON_ASSIGNED_LBN;

	if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN1 > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1) ||
		PBN2 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //PBN1 �Ǵ� PBN2�� �߸��� ������ �����Ǿ� ���� ���
		goto WRONG_ASSIGNED_LBN_ERR;

	fprintf(block_meta_output, "===== LBN : %u =====\n", LBN);
	if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE)
	{
		SPARE_reads(flashmem, PBN1, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
		fprintf(block_meta_output, "===== PBN1 : %u =====", PBN1);

		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			printf("\n< Offset : %d >\n", offset_index);
			fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);

			printf("Block State : ");
			switch (block_meta_buffer_array[offset_index]->get_block_state())
			{
			case BLOCK_STATE::NORMAL_BLOCK_EMPTY:
				fprintf(block_meta_output, "NORMAL_BLOCK_EMPTY\n");
				printf("NORMAL_BLOCK_EMPTY\n");
				break;

			case BLOCK_STATE::NORMAL_BLOCK_VALID:
				fprintf(block_meta_output, "NORMAL_BLOCK_VALID\n");
				printf("NORMAL_BLOCK_VALID\n");
				break;

			case BLOCK_STATE::NORMAL_BLOCK_INVALID:
				fprintf(block_meta_output, "NORMAL_BLOCK_INVALID\n");
				printf("NORMAL_BLOCK_INVALID\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_EMPTY:
				fprintf(block_meta_output, "SPARE_BLOCK_EMPTY\n");
				printf("SPARE_BLOCK_EMPTY\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_VALID:
				fprintf(block_meta_output, "SPARE_BLOCK_VALID\n");
				printf("SPARE_BLOCK_VALID\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_INVALID:
				fprintf(block_meta_output, "SPARE_BLOCK_INVALID\n");
				printf("SPARE_BLOCK_INVALID\n");
				break;
			}

			printf("Sector State : ");
			switch (block_meta_buffer_array[offset_index]->get_sector_state())
			{
			case SECTOR_STATE::EMPTY:
				fprintf(block_meta_output, "EMPTY\n");
				printf("EMPTY\n");
				break;

			case SECTOR_STATE::VALID:
				fprintf(block_meta_output, "VALID\n");
				printf("VALID\n");
				break;

			case SECTOR_STATE::INVALID:
				fprintf(block_meta_output, "INVALID\n");
				printf("INVALID\n");
				break;
			}
		}

		/*** Deallocate block_meta_buffer_array ***/
		if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;
	}
	else
		fprintf(block_meta_output, "===== PBN1 : non-assigned =====");

	if (PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
	{
		SPARE_reads(flashmem, PBN2, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
		fprintf(block_meta_output, "\n===== PBN2 : %u =====", PBN2);

		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			printf("\n< Offset : %d >\n", offset_index);
			fprintf(block_meta_output, "\n< Offset : %d >\n", offset_index);
			
			printf("Block State : ");
			switch (block_meta_buffer_array[offset_index]->get_block_state())
			{
			case BLOCK_STATE::NORMAL_BLOCK_EMPTY:
				fprintf(block_meta_output, "NORMAL_BLOCK_EMPTY\n");
				printf("NORMAL_BLOCK_EMPTY\n");
				break;

			case BLOCK_STATE::NORMAL_BLOCK_VALID:
				fprintf(block_meta_output, "NORMAL_BLOCK_VALID\n");
				printf("NORMAL_BLOCK_VALID\n");
				break;

			case BLOCK_STATE::NORMAL_BLOCK_INVALID:
				fprintf(block_meta_output, "NORMAL_BLOCK_INVALID\n");
				printf("NORMAL_BLOCK_INVALID\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_EMPTY:
				fprintf(block_meta_output, "SPARE_BLOCK_EMPTY\n");
				printf("SPARE_BLOCK_EMPTY\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_VALID:
				fprintf(block_meta_output, "SPARE_BLOCK_VALID\n");
				printf("SPARE_BLOCK_VALID\n");
				break;

			case BLOCK_STATE::SPARE_BLOCK_INVALID:
				fprintf(block_meta_output, "SPARE_BLOCK_INVALID\n");
				printf("SPARE_BLOCK_INVALID\n");
				break;
			}

			printf("Sector State : ");
			switch (block_meta_buffer_array[offset_index]->get_sector_state())
			{
			case SECTOR_STATE::EMPTY:
				fprintf(block_meta_output, "EMPTY\n");
				printf("EMPTY\n");
				break;

			case SECTOR_STATE::VALID:
				fprintf(block_meta_output, "VALID\n");
				printf("VALID\n");
				break;

			case SECTOR_STATE::INVALID:
				fprintf(block_meta_output, "INVALID\n");
				printf("INVALID\n");
				break;
			}
		}

		/*** Deallocate block_meta_buffer_array ***/
		if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;
	}
	else
		fprintf(block_meta_output, "\n===== PBN2 : non-assigned =====");

	goto END_SUCCESS;

END_SUCCESS:
	fclose(block_meta_output);
	printf(">> block_meta_output.txt\n");
	system("notepad block_meta_output.txt");
	return SUCCESS;

OUT_OF_RANGE: //���� �ʰ�
	fclose(block_meta_output);
	printf("out of range\n");
	return FAIL;

NON_ASSIGNED_LBN:
	fclose(block_meta_output);
	printf("non-assigned LBN\n");
	return FAIL;

WRONG_ASSIGNED_LBN_ERR:
	fclose(block_meta_output);
	fprintf(stderr, "ġ���� ���� : �߸� �Ҵ� �� LBN (print_block_meta_info)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (print_block_meta_info)\n");
	system("pause");
	exit(1);
}

int deallocate_single_meta_buffer(META_DATA*& src_meta_buffer)
{
	if (src_meta_buffer != NULL)
	{
		delete src_meta_buffer;
		src_meta_buffer = NULL;

		return SUCCESS;
	}

	return FAIL; //�Ҵ� ������ meta ������ ���� ���, ���� �������� �޸� ���� �˻�
}

int deallocate_block_meta_buffer_array(META_DATA**& src_block_meta_buffer_array)
{
	if (src_block_meta_buffer_array != NULL)
	{
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			delete src_block_meta_buffer_array[offset_index];
		}
		delete[] src_block_meta_buffer_array;

		src_block_meta_buffer_array = NULL;

		return SUCCESS;
	}

	return FAIL; //�Ҵ� ������ meta ������ ���� ���, ���� �������� �޸� ���� �˻�
}