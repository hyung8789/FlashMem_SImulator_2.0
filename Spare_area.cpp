#include "Build_Options.h"

// Spare Area�� ���� ��Ʈ ���� ó��, Meta-data �ǵ��� ���� �Լ� SPARE_init, SPARE_read, SPARE_write ����
// ������ ���� ���� ���� ������ Garbage Collection�� ���� SPARE_reads, update_victim_block_info, update_v_flash_info_for_reorganization, update_v_flash_info_for_erase, calc_block_invalid_ratio ����
// Meta-data�� ���� �� �Ϲ� ���� ��� Ž�� �� Ư�� ���� ��� ���� �� ���� ������ Ž�� ���� search_empty_block, search_empty_offset_in_block ����

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

		/* ����
		for (__int8 byte_unit = 0; byte_unit < SPARE_AREA_BYTE; byte_unit++) //1����Ʈ���� �ʱ�ȭ
		{
			write_buffer[byte_unit] = SPARE_INIT_VALUE;
		}
		*/
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

int SPARE_read(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& dst_meta_buffer) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü ���·� ��ȯ
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

		dst_meta_buffer = new META_DATA; //�������� META_DATA ����ü ����

		read_buffer = new unsigned char[SPARE_AREA_BYTE];
		fread(read_buffer, sizeof(unsigned char), SPARE_AREA_BYTE, storage_spare_pos);

		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/
		unsigned char bits_8_buffer = read_buffer[0]; //1byte == 8bitũ���� ��� �� ����(������) ������ ���Ͽ� Spare Area�� �о���� ���� 
		//���� fread(&bits_8_buffer, sizeof(unsigned char), 1, storage_spare_pos);

		/*** �о���� 8��Ʈ(2^7 ~2^0)�� ���ؼ� ��� ����(2^7 ~ 2^5) �Ǻ� ***/
		switch ((((bits_8_buffer) >> (5)) & (0x7))) //���� ������ 2^5 �ڸ��� LSB�� ������ ���������� 5�� ����Ʈ�Ͽ�, 00000111(2)�� AND ����
		{
		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_EMPTY:
			dst_meta_buffer->block_state = BLOCK_STATE::NORMAL_BLOCK_EMPTY;
			break;

		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_VALID:
			dst_meta_buffer->block_state = BLOCK_STATE::NORMAL_BLOCK_VALID;
			break;

		case (const unsigned)BLOCK_STATE::NORMAL_BLOCK_INVALID:
			dst_meta_buffer->block_state = BLOCK_STATE::NORMAL_BLOCK_INVALID;
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_EMPTY:
			dst_meta_buffer->block_state = BLOCK_STATE::SPARE_BLOCK_EMPTY;
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_VALID:
			dst_meta_buffer->block_state = BLOCK_STATE::SPARE_BLOCK_VALID;
			break;

		case (const unsigned)BLOCK_STATE::SPARE_BLOCK_INVALID:
			dst_meta_buffer->block_state = BLOCK_STATE::SPARE_BLOCK_INVALID;
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
			dst_meta_buffer->sector_state = SECTOR_STATE::EMPTY;
			break;

		case (const unsigned)SECTOR_STATE::VALID:
			dst_meta_buffer->sector_state = SECTOR_STATE::VALID;
			break;

		case (const unsigned)SECTOR_STATE::INVALID:
			dst_meta_buffer->sector_state = SECTOR_STATE::INVALID;
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

int SPARE_read(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& dst_meta_buffer) //���� ����(������)�� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü ���·� ��ȯ
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

int SPARE_write(class FlashMem*& flashmem, FILE*& storage_spare_pos, META_DATA*& src_meta_buffer) //META_DATA�� ���� ����ü ���޹޾�, ���� ������ Spare Area�� ���
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
		//�ش� ���Ͱ� ��ȿȭ �Ǿ��� ��� ��ȿ ī��Ʈ�� ������Ų��
		if (src_meta_buffer->sector_state == SECTOR_STATE::INVALID)
			flashmem->v_flash_info.invalid_sector_count++; //��ȿ ������ �� ����

		write_buffer = new unsigned char[SPARE_AREA_BYTE];
		memset(write_buffer, SPARE_INIT_VALUE, SPARE_AREA_BYTE);

		/*** Spare Area�� ��ü 16byte�� ���� ù 1byte�� ��� �� ����(������)�� ���� ������ ���� ó�� ���� ***/
		//BLOCK_TYPE(Normal or Spare, 1bit) || IS_VALID (BLOCK, 1bit) || IS_EMPTY (BLOCK, 1bit) || IS_VALID (SECTOR, 1bit) || IS_EMPTY(SECTOR, 1bit) || DUMMY (3bit)
		bits_8_buffer = ~(SPARE_INIT_VALUE); //00000000(2)

		switch (src_meta_buffer->block_state) //1����Ʈ ũ���� bits_8_buffer�� ���Ͽ�
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

		switch (src_meta_buffer->sector_state) //1����Ʈ ũ���� bits_8_buffer�� ���Ͽ�
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
		//���� fwrite(&bits_8_buffer, sizeof(unsigned char), 1, storage_spare_pos);
		
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

int SPARE_write(class FlashMem*& flashmem, unsigned int PSN, META_DATA*& src_meta_buffer) //META_DATA�� ���� ����ü ���޹޾�, ���� ������ Spare Area�� ���
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
int SPARE_reads(class FlashMem*& flashmem, unsigned int PBN, META_DATA**& dst_block_meta_buffer_array) //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü �迭 ���·� ��ȯ
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

	dst_block_meta_buffer_array = new META_DATA * [BLOCK_PER_SECTOR]; //��� �� ����(������)���� META_DATA �ּҸ� ���� �� �ִ� ����(row)

	if ((storage_spare_pos = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
		goto NULL_FILE_PTR_ERR;

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		dst_block_meta_buffer_array[offset_index] = NULL; //���� �ʱ�ȭ

		read_pos = ((SECTOR_INC_SPARE_BYTE * PBN) * BLOCK_PER_SECTOR) + (SECTOR_INC_SPARE_BYTE * offset_index);
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
		write_pos = ((SECTOR_INC_SPARE_BYTE * PBN) * BLOCK_PER_SECTOR) + (SECTOR_INC_SPARE_BYTE * offset_index);
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


int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, enum VICTIM_BLOCK_PROC_STATE proc_state, unsigned int src_block_num, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ����
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
	float LBN_invalid_ratio = -1;
	float PBN_invalid_ratio = -1; //PBN1 or PBN2 (���� ��Ͽ� ���� ��ȿ�� ���)
	float PBN1_invalid_ratio = -1;
	float PBN2_invalid_ratio = -1;

	META_DATA** block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA ����ü �迭 ����

	/***
		Victim Block ���� ����ü �ʱⰪ
		---
		victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
		victim_block_invalid_ratio = -1;
	***/

	//���� ó������ ���� Victim Block ������ �����ϸ� ġ���� ����
	if (flashmem->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE && flashmem->victim_block_info.victim_block_invalid_ratio != -1)
		goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

	switch (proc_state)
	{
	case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::SPARE_LINKED;
		break;

	case VICTIM_BLOCK_PROC_STATE::UNLINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::UNLINKED;
		break;

	default:
		goto WRONG_VICTIM_BLOCK_PROC_STATE;
	}

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
			1) Overwrite�� �߻��Ͽ����� PBN1 Ȥ�� PBN2�� ���ο� �����͸� ��� �� ������ ���� ���
			: ���� �۾� �߿� FTL�Լ��� ���Ͽ� Merge ����

			2) ������ ���� ���� Ȯ��
			: GC�� ���� ���� ���� ���̺��� ���� ��ü �� ��ϵ� �� Merge ������ LBN�� ���ؼ� Merge ����

		- Hybrid Mapping (Log algorithm)���� GC�� ������ ������ ���� ����(Physical Remaining Space)�� ���� Block Invalid Ratio Threshold�� ����
			1) ���� ��ȿȭ�� PBN�� ��� Erase ����
			2) �Ϻ� ��ȿ �����͸� �����ϰ� �ִ� LBN�� ������ PBN1�� PBN2�� ���Ͽ� Merge ����
			=> ���� �۾��� �߻��� LBN�� PBN1�� PBN2�� ���� ���յ� ��ȿ�� �� ���
	***/

	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (is_logical == true) //src_block_num�� LBN�� ���
			return FAIL;
		else //src_block_num�� PBN�� ���
		{
			flashmem->victim_block_info.is_logical = false;
			flashmem->victim_block_info.victim_block_num = PBN = src_block_num;
			flashmem->victim_block_info.victim_block_invalid_ratio = 1.0;

			goto BLOCK_MAPPING;
		}


	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical == true) //src_block_num�� LBN�� ���
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

HYBRID_LOG_PBN: //PBN1 or PBN2 (���� ��Ͽ� ���� ��ȿ�� ���)
	flashmem->victim_block_info.victim_block_num = PBN;

	/*** Calculate PBN Invalid Ratio ***/
	SPARE_reads(flashmem, PBN, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	calc_block_invalid_ratio(block_meta_buffer_array, PBN_invalid_ratio);
	
	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	try
	{
		if (PBN_invalid_ratio >= 0 && PBN_invalid_ratio <= 1)
			flashmem->victim_block_info.victim_block_invalid_ratio = PBN_invalid_ratio;
		else
			throw PBN_invalid_ratio;
	}
	catch (float& PBN_invalid_ratio)
	{
		fprintf(stderr, "ġ���� ���� : �߸��� ��ȿ��(%f)", PBN_invalid_ratio);
		system("pause");
		exit(1);
	}

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

		//���� ��� ó�� ��ƾ���� �̵�
		goto HYBRID_LOG_PBN;
	}

	/*** Calculate PBN1 Invalid Ratio ***/
	SPARE_reads(flashmem, PBN1, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	calc_block_invalid_ratio(block_meta_buffer_array, PBN1_invalid_ratio);

	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** Calculate PBN2 Invalid Ratio ***/
	SPARE_reads(flashmem, PBN2, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	calc_block_invalid_ratio(block_meta_buffer_array, PBN2_invalid_ratio);
	
	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	flashmem->victim_block_info.is_logical = true;
	flashmem->victim_block_info.victim_block_num = LBN;

	try
	{
		LBN_invalid_ratio = (float)((PBN1_invalid_ratio + PBN2_invalid_ratio) / 2); //LBN�� ��ȿ�� ���
		if (LBN_invalid_ratio >= 0 && LBN_invalid_ratio <= 1)
			flashmem->victim_block_info.victim_block_invalid_ratio = LBN_invalid_ratio;
		else
			throw LBN_invalid_ratio;
	}
	catch (float& LBN_invalid_ratio)
	{
		fprintf(stderr, "ġ���� ���� : �߸��� ��ȿ��(%f)", LBN_invalid_ratio);
		system("pause");
		exit(1);
	}

	goto END_SUCCESS;

END_SUCCESS:
	flashmem->gc->scheduler(flashmem, mapping_method, table_type); //���� �Ϸ�� Victim Block ���� ó���� ���� GC �����ٷ� ����
	return SUCCESS;

NON_ASSIGNED_LBN:
	return COMPLETE;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (update_victim_block_info)\n");
	system("pause");
	exit(1);

VICTIM_BLOCK_INFO_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ó������ ���� Victim Block ���� (update_victim_block_info)\n");
	system("pause");
	exit(1);

WRONG_VICTIM_BLOCK_PROC_STATE:
	fprintf(stderr, "ġ���� ���� : Wrong VICTIM_BLOCK_PROC_STATE (one_dequeue_job)\n");
	system("pause");
	exit(1);
}

/*** �̹� �о���� meta ������ �̿��Ͽ� ���� ***/
int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, enum VICTIM_BLOCK_PROC_STATE proc_state, unsigned int src_block_num, META_DATA**& src_block_meta_buffer_array, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ���� (��� ����)
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE;
	float PBN_invalid_ratio = -1; //PBN ��ȿ��

	//���� �������� src_block_meta_buffer_array �޸� ���� ����
	if (src_block_meta_buffer_array == NULL)
		goto MEM_LEAK_ERR;

	/***
		Victim Block ���� ����ü �ʱⰪ
		---
		victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
		victim_block_invalid_ratio = -1;
	***/

	//���� ó������ ���� Victim Block ������ �����ϸ� ġ���� ����
	if (flashmem->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE && flashmem->victim_block_info.victim_block_invalid_ratio != -1)
		goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

	switch (proc_state)
	{
	case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::SPARE_LINKED;
		break;

	case VICTIM_BLOCK_PROC_STATE::UNLINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::UNLINKED;
		break;

	default:
		goto WRONG_VICTIM_BLOCK_PROC_STATE;
	}

	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (is_logical == true) //src_block_num�� LBN�� ���
			return FAIL;
		else //src_block_num�� PBN�� ���
		{
			flashmem->victim_block_info.is_logical = false;
			flashmem->victim_block_info.victim_block_num = PBN = src_block_num;
			flashmem->victim_block_info.victim_block_invalid_ratio = 1.0;

			goto BLOCK_MAPPING;
		}

	default:
		return FAIL;
	}

BLOCK_MAPPING:
	goto END_SUCCESS;

END_SUCCESS:
	flashmem->gc->scheduler(flashmem, mapping_method, table_type); //���� �Ϸ�� Victim Block ���� ó���� ���� GC �����ٷ� ����
	return SUCCESS;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (update_victim_block_info)\n");
	system("pause");
	exit(1);

VICTIM_BLOCK_INFO_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ó������ ���� Victim Block ���� (update_victim_block_info)\n");
	system("pause");
	exit(1);

WRONG_VICTIM_BLOCK_PROC_STATE:
	fprintf(stderr, "ġ���� ���� : Wrong VICTIM_BLOCK_PROC_STATE (one_dequeue_job)\n");
	system("pause");
	exit(1);
}

int update_victim_block_info(class FlashMem*& flashmem, bool is_logical, enum VICTIM_BLOCK_PROC_STATE proc_state, unsigned int src_block_num, META_DATA**& src_PBN1_block_meta_buffer_array, META_DATA**& src_PBN2_block_meta_buffer_array, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ������ ���� ��� ���� ����ü ���� �� GC �����ٷ� ���� (���̺긮�� ����)
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE;
	float LBN_invalid_ratio = -1;
	float PBN1_invalid_ratio = -1;
	float PBN2_invalid_ratio = -1;

	//���� �������� src_PBN1_block_meta_buffer_array �� src_PBN2_block_meta_buffer_array �޸� ���� ����
	if (src_PBN1_block_meta_buffer_array == NULL || src_PBN2_block_meta_buffer_array == NULL)
		goto MEM_LEAK_ERR;

	/***
		Victim Block ���� ����ü �ʱⰪ
		---
		victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
		victim_block_invalid_ratio = -1;
	***/

	//���� ó������ ���� Victim Block ������ �����ϸ� ġ���� ����
	if (flashmem->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE && flashmem->victim_block_info.victim_block_invalid_ratio != -1)
		goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

	switch (proc_state)
	{
	case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::SPARE_LINKED;
		break;
	case VICTIM_BLOCK_PROC_STATE::UNLINKED:
		flashmem->victim_block_info.proc_state = VICTIM_BLOCK_PROC_STATE::UNLINKED;
		break;

	default:
		goto WRONG_VICTIM_BLOCK_PROC_STATE;
	}

	switch (mapping_method)
	{
	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical == true) //src_block_num�� LBN�� ���
		{
			LBN = src_block_num;
			PBN1 = flashmem->log_block_level_mapping_table[LBN][0];
			PBN2 = flashmem->log_block_level_mapping_table[LBN][1];

			goto HYBRID_LOG_LBN;
		}
		else //src_block_num�� PBN�� ��� : PBN1�� PBN2�� ��� ���� meta ������ ��� �޾����Ƿ� �� ���� �߸� ȣ�� �� ���̴�.
			return FAIL;

	default:
		return FAIL;
	}

HYBRID_LOG_LBN:
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //���� �� �����Ǿ� ���� ������
		goto NON_ASSIGNED_LBN;

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //�ϳ��� �����Ǿ� ���� ������
		goto MISMATCH_BETWEEN_META_TBL_ERR; //meta ������ ��� ���� ���� ���̺� ���� ����ġ

	/*** Calculate PBN1 Invalid Ratio ***/
	calc_block_invalid_ratio(src_PBN1_block_meta_buffer_array, PBN1_invalid_ratio);

	/*** Calculate PBN2 Invalid Ratio ***/
	calc_block_invalid_ratio(src_PBN2_block_meta_buffer_array, PBN2_invalid_ratio);

	flashmem->victim_block_info.is_logical = true;
	flashmem->victim_block_info.victim_block_num = LBN;

	try
	{
		LBN_invalid_ratio = (float)((PBN1_invalid_ratio + PBN2_invalid_ratio) / 2); //LBN�� ��ȿ�� ���
		if (LBN_invalid_ratio >= 0 && LBN_invalid_ratio <= 1)
			flashmem->victim_block_info.victim_block_invalid_ratio = LBN_invalid_ratio;
		else
			throw LBN_invalid_ratio;
	}
	catch (float& LBN_invalid_ratio)
	{
		fprintf(stderr, "ġ���� ���� : �߸��� ��ȿ��(%f)", LBN_invalid_ratio);
		system("pause");
		exit(1);
	}

	goto END_SUCCESS;

END_SUCCESS:
	flashmem->gc->scheduler(flashmem, mapping_method, table_type); //���� �Ϸ�� Victim Block ���� ó���� ���� GC �����ٷ� ����
	return SUCCESS;

NON_ASSIGNED_LBN:
	return COMPLETE;

MISMATCH_BETWEEN_META_TBL_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ��� ���� ���� ���̺� ���� ���� ����ġ (update_victim_block_info)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (update_victim_block_info)\n");
	system("pause");
	exit(1);

VICTIM_BLOCK_INFO_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ó������ ���� Victim Block ���� (update_victim_block_info)\n");
	system("pause");
	exit(1);

WRONG_VICTIM_BLOCK_PROC_STATE:
	fprintf(stderr, "ġ���� ���� : Wrong VICTIM_BLOCK_PROC_STATE (one_dequeue_job)\n");
	system("pause");
	exit(1);
}

int update_v_flash_info_for_reorganization(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array) //Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� ������ ���� ���� ���� ��� ���� ������ �÷��� �޸� ���� ����
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
			switch (src_block_meta_buffer_array[Poffset]->sector_state)
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

int update_v_flash_info_for_erase(class FlashMem*& flashmem, META_DATA**& src_block_meta_buffer_array) //Erase�ϰ��� �ϴ� Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� �÷��� �޸��� ������ ���� ����
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
			switch (src_block_meta_buffer_array[Poffset]->sector_state)
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

int calc_block_invalid_ratio(META_DATA**& src_block_meta_buffer_array, float& dst_block_invalid_ratio) //Ư�� ���� ��� �ϳ��� ���� META_DATA ����ü �迭�� ���� �Ǻ��� �����Ͽ� ��ȿ�� ��� �� ����
{
	//for Calculate Block Invalid Ratio
	__int8 block_per_written_sector_count = 0;
	__int8 block_per_invalid_sector_count = 0;
	__int8 block_per_empty_sector_count = 0;

	if (src_block_meta_buffer_array != NULL)
	{
		for (__int8 Poffset = 0; Poffset < BLOCK_PER_SECTOR; Poffset++) //��� ���� �� �������� ���� �ε���
		{
			switch (src_block_meta_buffer_array[Poffset]->sector_state)
			{
			case SECTOR_STATE::EMPTY:  //������� ���, �׻� ��ȿ�� �������̴�
				block_per_empty_sector_count++; //�� ������ �� ����
				break;

			case SECTOR_STATE::VALID: //������� �ʰ�, ��ȿ�� �������̸�
				block_per_written_sector_count++; //��ϵ� ������ �� ����
				break;

			case SECTOR_STATE::INVALID: //������� �ʰ�, ��ȿ���� ���� �������̸�
				block_per_written_sector_count++; //��ϵ� ������ �� ����
				block_per_invalid_sector_count++; //��ȿ ������ �� ����
				break;
			}
		}
	}
	else
		goto NULL_SRC_META_ERR;

	try
	{
		float block_invalid_ratio = (float)block_per_invalid_sector_count / (float)BLOCK_PER_SECTOR; //���� ����� ��ȿ�� ���
		if (block_invalid_ratio >= 0 && block_invalid_ratio <= 1)
			dst_block_invalid_ratio = block_invalid_ratio;
		else
			throw block_invalid_ratio;
	}
	catch (float& block_invalid_ratio)
	{
		fprintf(stderr, "ġ���� ���� : �߸��� ��ȿ��(%f)\n", block_invalid_ratio);
		fprintf(stderr, "block_per_written_sector_count : %d, block_per_invalid_sector_count : %d, block_per_empty_sector_count : %d\n", block_per_written_sector_count, block_per_invalid_sector_count, block_per_empty_sector_count);
		system("pause");
		exit(1);
	}

	return SUCCESS;

NULL_SRC_META_ERR:
	fprintf(stderr, "ġ���� ���� : ��� ��ȿ�� ��� ���� meta ������ �������� ���� (calc_block_invalid_ratio)\n");
	system("pause");
	exit(1);
}
		
/* ����
int search_empty_normal_block_for_dynamic_table(class FlashMem*& flashmem, unsigned int& dst_block_num, META_DATA*& dst_meta_buffer, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //�� �Ϲ� ���� ���(PBN)�� ���������� Ž���Ͽ� PBN�Ǵ� ���̺� �� LBN ��, �ش� PBN�� meta���� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	switch (mapping_method) //���� ��Ŀ� ���� �ش� ó�� ��ġ�� �̵�
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (table_type == TABLE_TYPE::STATIC) //��� ���� Static Table
			goto BLOCK_MAPPING_STATIC_PROC;

		else if (table_type == TABLE_TYPE::DYNAMIC) //��� ���� Dynamic Table
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
		PSN = (flashmem->block_level_mapping_table[table_index] * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
		SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

		if (meta_buffer->block_state == BLOCK_STATE::NORMAL_BLOCK_EMPTY)
			//�Ϲ� ��Ͽ� ���Ͽ�, ����ְ�, ��ȿ�� ����̸�
		{
			//LBN �� meta ���� ����
			dst_block_num = table_index;
			dst_meta_buffer = meta_buffer;

			return SUCCESS;
		}

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}
	//���� �� ����� ã�� ��������
	return COMPLETE;

DYNAMIC_COMMON_PROC: //Dynamic Table ���� ó�� ��ƾ : �� ���� ����� Spare ���� �Ǻ�, �� ��� ����Ž�� �� PBN����
	for (unsigned int block_index = 0; block_index < f_flash_info.block_size; block_index++) //���������� ����ִ� ����� ã�´� (Spare ����� ��ġ�� ���Ⱑ �Ͼ�� ���� �������̹Ƿ� ��� ��Ͽ� ���� ��ĳ��)
	{
		PSN = (block_index * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
		SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

		if (meta_buffer->block_state == BLOCK_STATE::NORMAL_BLOCK_EMPTY)
			//�Ϲ� ��Ͽ� ���Ͽ�, ����ְ�, ��ȿ�� ����̸�
		{
			//PBN �� meta ���� ����
			dst_block_num = block_index;
			dst_meta_buffer = meta_buffer;

			return SUCCESS;
		}

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}

	//���� �� ����� ã�� ��������
	return COMPLETE;

WRONG_TABLE_TYPE_ERR: //�߸��� ���̺� Ÿ��
	fprintf(stderr, "ġ���� ���� : �߸��� ���̺� Ÿ��\n");
	system("pause");
	exit(1);

WRONG_FUNC_CALL_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� �Լ� ȣ��\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (search_empty_normal_block)\n");
	system("pause");
	exit(1);
}
*/
int search_empty_offset_in_block(class FlashMem*& flashmem, unsigned int src_PBN, __int8& dst_Poffset, META_DATA*& dst_meta_buffer, enum MAPPING_METHOD mapping_method) //�Ϲ� ���� ���(PBN) ���θ� �������� ����ִ� ��ġ Ž��, Poffset ��, �ش� ��ġ�� meta���� ����
{
	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}

	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����
	__int8 low, mid, high, current_empty_index;

	switch (flashmem->search_mode)
	{
	case SEARCH_MODE::SEQ_SEARCH: //���� Ž��
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++) //���������� ����ִ� �������� ã�´�
		{
			PSN = (src_PBN * BLOCK_PER_SECTOR) + offset_index;
			SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

			if (meta_buffer->sector_state == SECTOR_STATE::EMPTY) //����ְ�, ��ȿ�� �������̸�
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

	case SEARCH_MODE::BINARY_SEARCH: //���� Ž��
		if (mapping_method != MAPPING_METHOD::HYBRID_LOG) //������ ���� ������ ��� �� ��쿡��, ���� Ž�� ��� ����
			goto WRONG_BINARY_SEARCH_MODE_ERR;

		low = 0;
		high = BLOCK_PER_SECTOR - 1;
		mid = get_rand_offset(); //Wear-leveling�� ���� �ʱ� ������ Ž�� ��ġ ����
		current_empty_index = OFFSET_MAPPING_INIT_VALUE;

		while (low <= high)
		{
			PSN = (src_PBN * BLOCK_PER_SECTOR) + mid; //Ž�� ��ġ
			SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

			if (meta_buffer->sector_state == SECTOR_STATE::EMPTY) //���������
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

			if (low <= high && current_empty_index != OFFSET_MAPPING_INIT_VALUE) //Ž�� �Ϸ� �� �� ���Ͱ� �����ϴ� ���, meta ���� ���� ���� ���� meta_buffer ����
				break;

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
		}

		if (current_empty_index != OFFSET_MAPPING_INIT_VALUE)
		{
			//Poffset �� meta ���� ����
			dst_Poffset = current_empty_index;
			dst_meta_buffer = meta_buffer;

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

int search_empty_offset_in_block(META_DATA**& src_block_meta_buffer_array, __int8& dst_Poffset, enum MAPPING_METHOD mapping_method) //�Ϲ� ���� ���(PBN)�� ��� ���� meta ������ �������� ����ִ� ��ġ Ž��, Poffset �� ����
{
	return 0;//�ϴܺ���
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
		if (is_logical == true) //src_block_num�� LBN�� ���
			goto BLOCK_LBN;
		else //src_block_num�� PBN�� ���
			goto COMMON_PBN;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (is_logical == true) //src_block_num�� LBN�� ���
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
		switch (block_meta_buffer_array[offset_index]->block_state)
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
		switch (block_meta_buffer_array[offset_index]->sector_state)
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
		switch (block_meta_buffer_array[offset_index]->block_state)
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
		switch (block_meta_buffer_array[offset_index]->sector_state)
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
			switch (block_meta_buffer_array[offset_index]->block_state)
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
			switch (block_meta_buffer_array[offset_index]->sector_state)
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
			switch (block_meta_buffer_array[offset_index]->block_state)
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
			switch (block_meta_buffer_array[offset_index]->sector_state)
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